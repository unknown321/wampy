// Basic bounds checks for accidental corruption before invoking ImGui's font parser.
// This is not a validator for hostile or adversarial fonts.
#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
namespace wampy_clock {
inline bool plausibleSfnt(const std::vector<unsigned char>& b) {
 if(b.size()<12)return false;
 auto word=[&](size_t p){return (std::uint32_t(b[p])<<24)|(std::uint32_t(b[p+1])<<16)|(std::uint32_t(b[p+2])<<8)|b[p+3];};
 auto magic=word(0);
 if(magic!=0x00010000 && magic!=0x4f54544f && magic!=0x74727565)return false;
 size_t count=(unsigned(b[4])<<8)|b[5];
 if(count==0||count>4096||count>(b.size()-12)/16)return false;
 for(size_t i=0;i<count;++i){size_t p=12+16*i;auto offset=word(p+8),length=word(p+12);if(offset>b.size()||length>b.size()-offset)return false;}
 return true;
}
inline bool plausibleFontFile(const std::string& path) {
 std::ifstream file(path,std::ios::binary|std::ios::ate);
 if(!file)return false;auto size=file.tellg();
 if(size<12 || size>64*1024*1024)return false;
 std::vector<unsigned char> bytes(static_cast<size_t>(size));file.seekg(0);
 return bool(file.read(reinterpret_cast<char*>(bytes.data()),size)) && plausibleSfnt(bytes);
}
}
