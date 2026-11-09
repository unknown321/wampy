// GPL-3.0-or-later. Read-only backlight observer, used on the UI owner thread.
#pragma once
#include <sys/inotify.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cerrno>
namespace wampy_resume {
class DisplayMonitor {
 const char* path_;int fd_=-1,watch_=-1;
public:
 explicit DisplayMonitor(const char* path):path_(path){fd_=inotify_init1(IN_NONBLOCK|IN_CLOEXEC);if(fd_>=0)watch_=inotify_add_watch(fd_,path_,IN_CLOSE_WRITE|IN_DELETE_SELF|IN_MOVE_SELF);}
 ~DisplayMonitor(){if(fd_>=0)close(fd_);} DisplayMonitor(const DisplayMonitor&)=delete;
 int fd()const{return watch_>=0?fd_:-1;}
 bool sample(bool& on){
  if(watch_<0)return false;alignas(inotify_event)char events[4096];ssize_t n;
  while((n=read(fd_,events,sizeof(events)))>0){for(ssize_t pos=0;pos<n;){auto e=reinterpret_cast<const inotify_event*>(events+pos);if(e->mask&(IN_IGNORED|IN_DELETE_SELF|IN_MOVE_SELF|IN_Q_OVERFLOW)){watch_=-1;return false;}pos+=sizeof(inotify_event)+e->len;}}
  if(n<0&&errno!=EAGAIN&&errno!=EINTR)return false;
  int f=open(path_,O_RDONLY|O_CLOEXEC);if(f<0)return false;char value[32]{};n=read(f,value,sizeof(value)-1);close(f);if(n<=0)return false;
  char* end=nullptr;errno=0;long brightness=strtol(value,&end,10);if(errno||end==value||brightness<0||(*end!='\0'&&*end!='\n'))return false;on=brightness>0;return true;
 }
};
}
