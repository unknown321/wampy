#include "../../src/bounded_socket.hpp"
#include <cassert>
#include <thread>
#include <iostream>
using namespace wampy_resume;
int main(){
 std::string path="/tmp/wampy-late-"+std::to_string(getpid());OwnedFd listener(socket(AF_UNIX,SOCK_STREAM|SOCK_CLOEXEC,0));assert(listener.fd>=0);sockaddr_un a{};a.sun_family=AF_UNIX;strcpy(a.sun_path,path.c_str());assert(bind(listener.fd,reinterpret_cast<sockaddr*>(&a),sizeof(a))==0);assert(listen(listener.fd,2)==0);
 std::thread server([&](){for(int i=0;i<2;++i){OwnedFd peer(accept(listener.fd,nullptr,nullptr));assert(peer.fd>=0);char request;assert(read(peer.fd,&request,1)==1);if(i==0)std::this_thread::sleep_for(std::chrono::milliseconds(60));const char* msg=i?"fresh":"late";send(peer.fd,msg,strlen(msg),MSG_NOSIGNAL);}});
 std::string reply;assert(!exchange(path.c_str(),"a",1,&reply,20)&&reply.empty());assert(exchange(path.c_str(),"b",1,&reply,300)&&reply=="fresh");server.join();unlink(path.c_str());
 int f[2];assert(socketpair(AF_UNIX,SOCK_STREAM|SOCK_NONBLOCK,0,f)==0);OwnedFd client(f[0]);std::thread oversized([&](){OwnedFd peer(f[1]);char request;assert(waitFd(peer.fd,POLLIN,monotonicMs()+500));assert(read(peer.fd,&request,1)==1);std::string payload(70000,'x');size_t sent=0;auto deadline=monotonicMs()+500;while(sent<payload.size()&&waitFd(peer.fd,POLLOUT,deadline)){auto n=send(peer.fd,payload.data()+sent,payload.size()-sent,MSG_NOSIGNAL);if(n<=0)break;sent+=size_t(n);}});
 assert(!exchangeConnected(client.fd,"x",1,&reply,monotonicMs()+500)&&reply.empty());oversized.join();
 std::cout<<"PASS: timed-out request closed, late reply cannot enter next RPC, oversized response rejected\n";
}
