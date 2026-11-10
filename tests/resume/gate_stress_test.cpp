#include "../../src/render_gate.hpp"
#include <thread>
#include <cassert>
#include <iostream>
int main(){for(int i=0;i<1000;++i){wampy_resume::RenderGate gate(false);std::atomic<bool> stop{false};std::atomic<int> passed{0};std::thread a([&](){if(gate.waitUntilVisible(stop))++passed;});std::thread b([&](){if(gate.waitUntilVisible(stop))++passed;});assert(passed==0);gate=true;a.join();b.join();assert(passed==2);gate=false;std::thread cancel([&](){assert(!gate.waitUntilVisible(stop));});stop=true;gate.notify();cancel.join();}std::cout<<"PASS: RenderGate 1000 dual-worker wake/pause/cancel rounds\n";}
