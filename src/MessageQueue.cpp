/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */

#include "MessageQueue.h"
#include <condition_variable>
#include <queue>

class MessageQueue::Impl {
public:
    Impl() : mutex{}, queue{}, condition_variable{} {}

    void push(Message &&message) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            queue.push(message.move());
        }
        condition_variable.notify_one();
    }

    std::unique_ptr<Message> pop() {
        std::unique_lock<std::mutex> lock(mutex);
        condition_variable.wait(lock, [this] {
            return !queue.empty();
        });
        auto message = queue.front()->move();
        queue.pop();
        return message;
    }

private:
    std::mutex mutex;
    std::queue<std::unique_ptr<Message>> queue;
    std::condition_variable condition_variable;
};

MessageQueue::MessageQueue() : impl(new Impl) {}

MessageQueue::~MessageQueue() = default;

void MessageQueue::push(Message &&message) {
    impl->push(std::move(message));
}

std::unique_ptr<Message> MessageQueue::get() {
    return impl->pop();
}



