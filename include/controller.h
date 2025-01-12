/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */
#pragma once
#include <deque>
#include <mutex>
#include <set>
#include <map>
#include <string>
#include <memory>
#include "fifo.h"
#include "message.h"
#include "spdlog/spdlog.h"

using TCallback= std::function<void (std::string& result)>;

class Controller;

class Controller
{
 public:
   Controller();
   ~Controller();

   int  Start();
   void Shutdown();
   void Message(const std::string& topic,std::shared_ptr<MESSAGE> message, TCallback callback);

   void OnMessage(const std::string& topic,std::shared_ptr<MESSAGE> message, TCallback callback);
     // Common Infrastructure of Active Object.
     static void* EntryOfThread(void* arg);
     void Run();
     void OnProxyRequest();
     pthread_t thr_id_;
     bool shutdown_;
     Fifo<std::function<void()>>send_queue_;
     int GetFd() const
     {
         return send_queue_.GetFd();
     }
};
