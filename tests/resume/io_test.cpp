#include "../../src/bounded_socket.hpp"
#include "../../src/display_monitor.hpp"
#include "../../src/render_gate.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <dirent.h>
using namespace wampy_resume;
int fds(){int n=0;auto d=opendir("/proc/self/fd");while(readdir(d))++n;closedir(d);return n;}
int main(){
 int pair[2];assert(socketpair(AF_UNIX,SOCK_STREAM|SOCK_NONBLOCK,0,pair)==0);OwnedFd peer(pair[0]);
 std::thread server([&](){OwnedFd other(pair[1]);assert(waitFd(other.fd,POLLIN,monotonicMs()+1000));char buf[4];assert(read(other.fd,buf,4)>0);assert(send(other.fd,"abc",3,MSG_NOSIGNAL)==3);std::this_thread::sleep_for(std::chrono::milliseconds(5));assert(send(other.fd,"def",3,MSG_NOSIGNAL)==3);});
 std::string out;assert(exchangeConnected(peer.fd,"x",1,&out,monotonicMs()+200)&&out=="abcdef");server.join();
 assert(socketpair(AF_UNIX,SOCK_STREAM|SOCK_NONBLOCK,0,pair)==0);OwnedFd client(pair[0]),silent(pair[1]);auto before=monotonicMs();assert(!exchangeConnected(client.fd,"x",1,&out,before+20));assert(monotonicMs()-before<500&&out.empty());
 int baseline=fds();for(int i=0;i<200;++i)assert(!exchange("/tmp/nonexistent-wampy-io-test","x",1,&out,10));assert(fds()==baseline);
 std::string path="/tmp/wampy-brightness-"+std::to_string(getpid());
 auto writeValue=[&](const char* value){int f=open(path.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0600);assert(f>=0);assert(write(f,value,strlen(value))>0);close(f);};
 writeValue("0\n");DisplayMonitor monitor(path.c_str());bool on=true;assert(monitor.sample(on)&&!on);writeValue("255\n");assert(monitor.sample(on)&&on);writeValue("invalid");assert(!monitor.sample(on));unlink(path.c_str());assert(!monitor.sample(on));
 RenderGate gate(false);std::atomic<bool>stop{false},passed{false};std::thread worker([&](){passed=gate.waitUntilVisible(stop);});gate=true;worker.join();assert(passed);gate=false;stop=true;gate.notify();assert(!gate.waitUntilVisible(stop));
 std::cout<<"PASS: real transport fragments/deadline/FD cleanup, inotify brightness, render-gate wake/cancel\n";
}
