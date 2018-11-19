#include "Queue.h"
#include <thread>
#include <iostream>

void Producer(ThreadSafeQueue<int>& q, std::mutex& m) {
    auto timeout = std::chrono::milliseconds(300);
    for (int i = 0; i < 20; i++) {
        auto status = q.Push(i, timeout);
    }
}

void Consumer(ThreadSafeQueue<int>& q, std::mutex& m) {
    auto timeout = std::chrono::milliseconds(300);
    for (int i = 0; i < 30; i++) {
        auto res = q.Pop(timeout);
    }
}

int main() {
    auto timeout = std::chrono::seconds(1);
    std::mutex m;
    ThreadSafeQueue<int> q(10);
    std::vector<std::thread> producers, consumers;
    size_t consumers_count = 5, producers_count = 3;
    for (int i = 0; i < producers_count; i++) {
        producers.emplace_back(Producer, std::ref(q), std::ref(m));
    }

    for (int i = 0; i < consumers_count; i++) {
        consumers.emplace_back(Consumer, std::ref(q), std::ref(m));
    }


    for (auto& producer : producers) {
        if (producer.joinable()) {
            producer.join();
        }
    }
    for (auto& consumer : consumers) {
        if (consumer.joinable()) {
            consumer.join();
        }
    }
    return 0;
}