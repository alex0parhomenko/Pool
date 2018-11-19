#pragma once
#include <chrono>
#include <mutex>
#include <optional>
#include <queue>
#include <condition_variable>
#include <iostream>

template<class T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue(const size_t max_queue_size)
            :MaxQueueSize(max_queue_size) {}

    ThreadSafeQueue() = delete;
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue(ThreadSafeQueue&&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

    template<class Duration>
    bool Push(const T& item, Duration wait_ms);

    template<class Duration>
    bool Push(T&& item, Duration wait_ms);

    template<class Duration>
    std::optional<T> Pop(Duration wait_ms);

    T Front() {
        std::lock_guard lock(Mutex);
        return Queue.front();
    }

    size_t GetSize() const {
        std::lock_guard lock(Mutex);
        return Queue.size();
    }
private:
    template<class Duration>
    static std::chrono::system_clock::time_point GetTaskFinishTimePoint(Duration ms);

    mutable std::timed_mutex Mutex;
    std::condition_variable_any CVPush, CVPop;
    std::queue<T> Queue;
    const size_t MaxQueueSize;
};

template<class T>
template<class Duration>
bool ThreadSafeQueue<T>::Push(const T& item, Duration wait_ms) {
    const auto finish = GetTaskFinishTimePoint(wait_ms);

    std::unique_lock lock(Mutex, finish);
    if (!lock.owns_lock()) {
        return false;
    }
    auto operation_status = true;
    if (Queue.size() == MaxQueueSize) {
        operation_status = CVPop.wait_until(lock, finish,
                [this] {return Queue.size() < MaxQueueSize;});
    }
    if (operation_status) {
        Queue.push(item);
        lock.unlock();
        CVPush.notify_one();
    }
    return operation_status;
}

template<class T>
template<class Duration>
bool ThreadSafeQueue<T>::Push(T&& item, Duration wait_ms) {
    const auto finish = GetTaskFinishTimePoint(wait_ms);

    std::unique_lock lock(Mutex, finish);
    if (!lock.owns_lock()) {
        return false;
    }
    auto operation_status = true;
    if (Queue.size() == MaxQueueSize) {
        operation_status = CVPop.wait_until(lock, finish,
                [this] {return Queue.size() < MaxQueueSize;});
    }
    if (operation_status) {
        Queue.push(item);
        lock.unlock();
        CVPush.notify_one();
    }
    return operation_status;
}

template<class T>
template<class Duration>
std::optional<T> ThreadSafeQueue<T>::Pop(Duration wait_ms) {
    const auto finish = GetTaskFinishTimePoint(wait_ms);

    std::optional<T> result;
    std::unique_lock lock(Mutex, finish);
    if (!lock.owns_lock()) {
        return result;
    }
    auto operation_status = true;
    if (Queue.empty()) {
        operation_status = CVPush.wait_until(lock, finish, [this] {return !Queue.empty();});
    }

    if (operation_status) {
        result = Queue.front();
        Queue.pop();
        lock.unlock();
        CVPop.notify_one();
    }
    return result;
}

template<class T>
template<class Duration>
std::chrono::system_clock::time_point ThreadSafeQueue<T>::GetTaskFinishTimePoint(Duration ms) {
    return std::chrono::system_clock::now() + ms;
}
