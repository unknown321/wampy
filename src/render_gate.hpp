// GPL-3.0-or-later. Shared visibility gate; UI owns transitions, workers wait.
#pragma once
#include <atomic>
#include <condition_variable>
#include <mutex>
namespace wampy_resume {
class RenderGate {
    std::atomic<bool> visible_;
    std::mutex mutex_;
    std::condition_variable changed_;
public:
    explicit RenderGate(bool visible=false):visible_(visible){}
    bool load() const { return visible_.load(); }
    operator bool() const { return load(); }
    RenderGate& operator=(bool value) { if(visible_.exchange(value)!=value) notify(); return *this; }
    void notify() { std::lock_guard<std::mutex> lock(mutex_); changed_.notify_all(); }
    bool waitUntilVisible(const std::atomic<bool>& stop) {
        std::unique_lock<std::mutex> lock(mutex_);
        changed_.wait(lock,[&](){return visible_.load() || stop.load();}); return !stop.load();
    }
};
}
