#include "../../src/wstring.h"
#include <cassert>
#include <random>
#include <iostream>
int main(){
 std::string valid="A\xC2\xA2\xE4\xB8\xAD\xF0\x9F\x98\x80";size_t i=0;
 for(auto point:{0x41u,0xA2u,0x4E2Du,0x1F600u})assert(utfToPoint(valid,i)==point);
 assert(i==valid.size());
 for(auto bad:{"\xFF","\x80","\xC0\x80","\xE0\x80\x80","\xED\xA0\x80","\xF4\x90\x80\x80","\xE4\xB8"}){
  std::string s(bad);size_t n=0;assert(utfToPoint(s,n)==0xFFFD && n==1);
 }
 std::mt19937 rng(321);for(int n=0;n<100000;++n){std::string s(rng()%64,' ');for(char& c:s)c=char(rng());size_t pos=0;unsigned steps=0;while(pos<s.size()){size_t prev=pos;auto p=utfToPoint(s,pos);assert(pos>prev&&pos<=s.size()&&p<=0x10FFFF&&!(p>=0xD800&&p<=0xDFFF));assert(++steps<=s.size());}}
 std::cout<<"PASS: production UTF-8 decoder handles valid/invalid/truncated input and 100000 fuzz strings without stalls\n";
}
