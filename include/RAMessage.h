/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */
#pragma once
#include <memory>

class Message {
public:
    Message() = default;

    Message(const Message &) = delete;

    virtual std::unique_ptr<Message> move() = 0;

    virtual ~Message() = default;

protected:
    Message(Message &&) = default;

    Message &operator=(Message &&) = default;
};

/**
 * An inter-thread message.
 * @tparam T type of payload
 */
template<typename T>
class RAMessage : public Message {
public:

    /**
    * Construct the message with payload arguments.
    * @tparam Args the argument type
    * @param args  the arguments required to construct the payload class
    */
    template<typename ... Args>
    explicit RAMessage(Args &&... args):
            Message(),
            payload(new T(std::forward<Args>(args) ...)) {

    }

    ~RAMessage() override = default;

    RAMessage(const RAMessage &) = delete;

    RAMessage &operator=(const RAMessage &) = delete;

    /** "Virtual move constructor" */
    std::unique_ptr<Message> move() override {
        return std::unique_ptr<Message>(new RAMessage<T>(std::move(*this)));
    }

    /** Get the payload data */
    T &getPayload() const {
        return *payload;
    }

protected:
    RAMessage(RAMessage &&) noexcept = default;

    RAMessage &operator=(RAMessage &&) noexcept = default;

private:
    /**
    * The message payload.
    */
    std::unique_ptr<T> payload;
};
