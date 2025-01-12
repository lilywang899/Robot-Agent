/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */
#pragma once

#include <pthread.h>
#include <queue>
#include "fdevent.h"

template<typename T>
class Fifo
{
	public:
	Fifo()
    {
		pthread_mutex_init(&lock_, NULL);
	}
	
    ~Fifo() 
    {
		pthread_mutex_unlock(&lock_);
		pthread_mutex_destroy(&lock_);
	}
	
    int GetFd() const
    {
		return fd_event_.fd();
	}
	
    void push(T* item)
	{
		pthread_mutex_lock(&lock_);
		queue_.push(item);
		pthread_mutex_unlock(&lock_);
		fd_event_.notify();		
	}
	
	T* Pop()
	{
		T* item = 0;
		fd_event_.wait();
		pthread_mutex_lock(&lock_);
		if(!queue_.empty())
		{
			item = queue_.front();
			queue_.pop();
		}
		pthread_mutex_unlock(&lock_);
		return item;
	}
	
    private:
	    pthread_mutex_t lock_;
		std::queue<T*> queue_;
		FdEvent fd_event_;
};

