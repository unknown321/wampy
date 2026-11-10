// GPL-3.0-or-later. Deadline applies to the entire request, not each read.
#pragma once
#include <chrono>
#include <string>
#include <cerrno>
#include <cstring>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
namespace wampy_resume {
inline long long monotonicMs(){return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();}
struct OwnedFd { int fd; explicit OwnedFd(int f):fd(f){} ~OwnedFd(){if(fd>=0)close(fd);} OwnedFd(const OwnedFd&)=delete; OwnedFd& operator=(const OwnedFd&)=delete; };
inline bool waitFd(int fd,short events,long long deadline){
    for(;;){auto left=deadline-monotonicMs();if(left<=0)return false;pollfd p{fd,events,0};int n=poll(&p,1,static_cast<int>(left));if(n<0&&errno==EINTR)continue;if(monotonicMs()>=deadline)return false;return n>0&&!(p.revents&POLLNVAL)&&(p.revents&(events|POLLERR|POLLHUP));}
}
inline bool exchangeConnected(int fd,const char* data,size_t len,std::string* out,long long deadline){
    out->clear();size_t sent=0;
    while(sent<len){if(!waitFd(fd,POLLOUT,deadline))return false;auto n=send(fd,data+sent,len-sent,MSG_NOSIGNAL);if(n<0&&(errno==EINTR||errno==EAGAIN))continue;if(n<=0)return false;sent+=static_cast<size_t>(n);}
    for(;;){if(!waitFd(fd,POLLIN,deadline)){out->clear();return false;}char buf[4096];auto n=recv(fd,buf,sizeof(buf),0);if(monotonicMs()>=deadline){out->clear();return false;}if(n<0&&(errno==EINTR||errno==EAGAIN))continue;if(n<0||out->size()+(n>0?n:0)>65536){out->clear();return false;}if(!n)return !out->empty();out->append(buf,static_cast<size_t>(n));}
}
inline bool exchange(const char* path,const char* data,size_t len,std::string* out,int ms){
    out->clear();if(!path||strlen(path)>=sizeof(sockaddr_un::sun_path)||ms<=0)return false;
    OwnedFd fd(socket(AF_UNIX,SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC,0));if(fd.fd<0)return false;
    auto deadline=monotonicMs()+ms;sockaddr_un a{};a.sun_family=AF_UNIX;strcpy(a.sun_path,path);
    if(connect(fd.fd,reinterpret_cast<sockaddr*>(&a),sizeof(a))<0){if(errno!=EINPROGRESS||!waitFd(fd.fd,POLLOUT,deadline))return false;int error=0;socklen_t size=sizeof(error);if(getsockopt(fd.fd,SOL_SOCKET,SO_ERROR,&error,&size)||error)return false;}
    return exchangeConnected(fd.fd,data,len,out,deadline);
}
}
