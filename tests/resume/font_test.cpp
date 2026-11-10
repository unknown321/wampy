#include "../../src/digital_clock/font_file.hpp"
#include <cassert>
#include <iostream>
using namespace wampy_clock;
int main(){
 assert(!plausibleFontFile("/nonexistent-clock-font"));
 std::vector<unsigned char>b(28);assert(!plausibleSfnt(b));b[1]=1;b[5]=1;assert(plausibleSfnt(b));
 b[20]=0xff;assert(!plausibleSfnt(b));b[20]=0;b[27]=29;assert(!plausibleSfnt(b));
 for(size_t n=0;n<28;++n){b.resize(n);assert(!plausibleSfnt(b));b.resize(28);}
 std::cout<<"PASS: missing/truncated/invalid/out-of-bounds font resources rejected before atlas load\n";
}
