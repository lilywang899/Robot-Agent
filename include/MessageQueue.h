/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */
#pragma once
#include "RAMessage.h"

/**
 * A safe message queue to be used for inter-thread communication.
 */
class MessageQueue {
public:
    MessageQueue();

    ~MessageQueue();

    void push(Message &&message);

    std::unique_ptr<Message> get();

private:
    class Impl;

    std::unique_ptr<Impl> impl;

};
