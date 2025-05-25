/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */

#pragma once

#include <pthread.h>
#include "mqttClient.h"
#include <deque>
#include <mutex>
#include <set>
#include <map>
#include <string>
#include <memory>
#include "fifo.h"
#include "message.h"
#include "spdlog/spdlog.h"

class DSService;
class mqttClient;
class Controller;

using TCallback= std::function<void (std::string& result)>;

class Agent {
 public:   
  Agent(const std::string& configFile); 
  ~Agent(){};
  void loadConfig(const std::string& fileName);
  void Start();
  void Shutdown();
  void Message(std::shared_ptr<MESSAGE> message, TCallback callback);

 private:
  static void* EntryOfThread(void* argv);
  void Run();
  void parseDsMessage();
  void parseRobotMessage();
  std::shared_ptr<DSService> dsService;   
  std::shared_ptr<Controller> controller;
  pthread_t thread_id;
  bool shutdown_;
  void OnMessage(std::shared_ptr<MESSAGE> message, TCallback callback);
  void OnProxyRequest();
  Fifo<std::function<void()>>send_queue_;
  int GetFd() const
  {
      return send_queue_.GetFd();
  }
  std::string configFile;
  unsigned int counter;
  bool DS_enabled=false;
  int DS_joint_enabled=100;
  //unsigned char DS_joint_control[100]="&0,-71,180,0,0,0,160";
  //std::vector<std::string> DS_joint_control = {"&", "0", "-71","180","0","0","0","160"};

  //unsigned char DS_message[100];
};

