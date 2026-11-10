// Basic bounds/required-table checks for accidental corruption before ImGui.
// This is not a validator for hostile or adversarial fonts.
#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <cstdint>
namespace wampy_clock {
inline bool plausibleSfnt(const std::vector<unsigned char>& b) {
 if(b.size()<12)return false;
 auto word=[&](size_t p){return (std::uint32_t(b[p])<<24)|(std::uint32_t(b[p+1])<<16)|(std::uint32_t(b[p+2])<<8)|b[p+3];};
 auto magic=word(0);
 if(magic!=0x00010000 && magic!=0x4f54544f && magic!=0x74727565)return false;
 size_t count=(unsigned(b[4])<<8)|b[5];
 if(count==0||count>4096||count>(b.size()-12)/16)return false;
 std::set<std::uint32_t> tags;
 for(size_t i=0;i<count;++i){
  size_t p=12+16*i;auto tag=word(p),offset=word(p+8),length=word(p+12);
  if(offset>b.size()||length>b.size()-offset||!tags.insert(tag).second)return false;
  if(tag==0x68656164 && (length<54 || word(offset+12)!=0x5f0f3cf5))return false; // head
  if(tag==0x68686561 && length<36)return false; // hhea
  if(tag==0x6d617870 && length<6)return false; // maxp
  if(tag==0x636d6170 && length<4)return false; // cmap
 }
 for(auto tag:{0x636d6170u,0x68656164u,0x68686561u,0x686d7478u,0x6d617870u})if(!tags.count(tag))return false;
 return tags.count(0x43464620u) || (tags.count(0x676c7966u)&&tags.count(0x6c6f6361u)); // CFF or glyf/loca
}
inline bool plausibleFontFile(const std::string& path) {
 std::ifstream file(path,std::ios::binary|std::ios::ate);
 if(!file)return false;auto size=file.tellg();
 if(size<12 || size>64*1024*1024)return false;
 std::vector<unsigned char> bytes(static_cast<size_t>(size));file.seekg(0);
 return bool(file.read(reinterpret_cast<char*>(bytes.data()),size)) && plausibleSfnt(bytes);
}
}
