#ifndef WAMPY_INOTIFIABLE_H
#define WAMPY_INOTIFIABLE_H

class INotifiable {
  public:
    virtual void Notify() = 0;
    bool active{};
};

#endif // WAMPY_INOTIFIABLE_H
