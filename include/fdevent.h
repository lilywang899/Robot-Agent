/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */
#pragma once
 
#include <sys/eventfd.h>

class FdEvent
{
    public:
        FdEvent();
        ~FdEvent();
        int fd() const
        {
            return fd_;
        }
        bool wait();
        bool notify();
    private:
        int fd_;
};
