#include "../../src/bounded_socket.hpp"
#include <cassert>
#include <thread>
#include <iostream>
#include <atomic>
#include <dirent.h>
using namespace wampy_resume;
int fdCount(){int count=0;auto d=opendir("/proc/self/fd");assert(d);while(readdir(d))++count;closedir(d);return count;}
int main(){
 int f[2];std::string reply;std::atomic<int> lateWrite{0};
 assert(socketpair(AF_UNIX,SOCK_STREAM|SOCK_NONBLOCK,0,f)==0);
 std::thread delayed([&](){OwnedFd peer(f[1]);char request;assert(waitFd(peer.fd,POLLIN,monotonicMs()+500));assert(read(peer.fd,&request,1)==1);std::this_thread::sleep_for(std::chrono::milliseconds(60));lateWrite=send(peer.fd,"late",4,MSG_NOSIGNAL);});
 {OwnedFd client(f[0]);assert(!exchangeConnected(client.fd,"a",1,&reply,monotonicMs()+20)&&reply.empty());}
 delayed.join();assert(lateWrite<0);
 assert(socketpair(AF_UNIX,SOCK_STREAM|SOCK_NONBLOCK,0,f)==0);
 std::thread fresh([&](){OwnedFd peer(f[1]);char request;assert(waitFd(peer.fd,POLLIN,monotonicMs()+500));assert(read(peer.fd,&request,1)==1);assert(send(peer.fd,"fresh",5,MSG_NOSIGNAL)==5);});
 {OwnedFd client(f[0]);assert(exchangeConnected(client.fd,"b",1,&reply,monotonicMs()+500)&&reply=="fresh");}fresh.join();
 assert(socketpair(AF_UNIX,SOCK_STREAM|SOCK_NONBLOCK,0,f)==0);
 std::thread oversized([&](){OwnedFd peer(f[1]);char request;assert(waitFd(peer.fd,POLLIN,monotonicMs()+500));assert(read(peer.fd,&request,1)==1);std::string payload(70000,'x');size_t sent=0;auto deadline=monotonicMs()+500;while(sent<payload.size()&&waitFd(peer.fd,POLLOUT,deadline)){auto n=send(peer.fd,payload.data()+sent,payload.size()-sent,MSG_NOSIGNAL);if(n<=0)break;sent+=size_t(n);}});
 {OwnedFd client(f[0]);assert(!exchangeConnected(client.fd,"x",1,&reply,monotonicMs()+500)&&reply.empty());}oversized.join();
 int before=fdCount();for(int i=0;i<200;++i){assert(socketpair(AF_UNIX,SOCK_STREAM|SOCK_NONBLOCK,0,f)==0);OwnedFd a(f[0]),b(f[1]);assert(!exchangeConnected(a.fd,"x",1,&reply,monotonicMs()+1));}assert(fdCount()==before);
 std::cout<<"PASS: real socketpair deadlines/late peer close/fresh RPC/oversized reply and allocated FD cleanup; pathname connect not covered\n";
}
