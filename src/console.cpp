/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */

#include "console.h"
#include "mqttClient.h"
#include "spdlog/spdlog.h"
#include "iostream";
#include <stdio.h>
#include "message.h"

Console::  Console() {
    shutdown_ = false;
    command ="voltage";
    parameters=" 12.03";
    counter = 0;
}

void Console:: readInput() {

    std::cout << "Enter command : " << std::endl;
    //flush();
    char str[25];
    fgets(str, 25, stdin);
    command=std::string(str);
    std::cout << "Enter parameters: " << std::endl;
    //flush();
    fgets(str, 25, stdin);
    parameters=std::string(str);
    spdlog::info("command:[{}], param:[{}].", command, parameters);
}
void Console::Start() {
    if(pthread_create(&thread_id, nullptr, EntryOfThread,this) != 0) {
        spdlog::critical("failed start Console thread.");
    }
}
void Console::execute() {
   std::string topic = "test/topic0";
   std::string data= command + parameters;
   // build message and send to agent.
   MESSAGE msg = {0};
   msg.sid=COM_CONTROLLER;
   msg.did=COM_AGENT;
   msg.length = data.length();
   msg.type = SMM_OutGoingRequest;
   memcpy(msg.Union.smm_OutGoingRequest.PhoneNumber,data.c_str(), data.length());
      
   std::shared_ptr<MESSAGE> message = std::make_shared<MESSAGE>(msg);
   client->publish(topic, message);
}

void Console::Shutdown() {
  shutdown_ = true;
}

/*static*/
void* Console::EntryOfThread(void* argv) {
    Console* console = static_cast<Console*>(argv);
    console->Run();
}

void Console::Run() {
  while(!shutdown_) {
      //readInput();
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
// enable below when mqtt server and another client is ready.
//      execute();
      
      counter = (counter++)%65535;
      if (counter == 0) {
//          spdlog::info("send mqtt message [{}].", counter);
      }
  }
}


