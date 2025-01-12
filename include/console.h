/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */

#pragma once
#include <pthread.h>
#include <string>
#include <memory>

class mqttClient;

class Console {
 public:   
  Console(); 
  ~Console(){};
  void readInput();
  void execute();
  void Shutdown();
  void Start();
  void setClient(mqttClient* p_client) {
      client = p_client;
  }

 private:
  static void* EntryOfThread(void* argv);
  void Run();
  mqttClient* client;   
  pthread_t thread_id;
  std::string topic;
  bool shutdown_;
  std::string command;
  std::string parameters;
  unsigned int counter;
};

