/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */

#include <libwebsockets.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <assert.h>

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"  
#include "spdlog/fmt/ostr.h" 
#include <memory>
#include "agent.h"
#include <chrono>
#include "console.h"
#include "wrapper.h"

using namespace spdlog;
//#include "spdlog"
int init_logging();

void loadConfig(const std::string& fileName, std::string& mode);
int main(int argc, const char **argv)
{
    spdlog::warn("Easy padding in numbers like {:08d}", 12);
    spdlog::info("Welcome to spdlog version {}.{}.{}  !", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR,SPDLOG_VER_PATCH);
    std::string mode;
    std::string configFile = "/home/lily/Robot-Agent/config/config.txt";
    //std::string configFile = "/home/l753wang/Robot-Agent/config/config.txt";

    init_logging();

    loadConfig(configFile,mode);
    if (mode == "robot") 
    {
       spdlog::info("run as robot.");
       client_create();
       g_mqttClient_ptr->loadConfig(configFile);
       g_mqttClient_ptr->Start();

       std::shared_ptr<Console> console = std::make_shared<Console>();
       console->setClient(g_mqttClient_ptr);
       console->Start();
       while(1) {
           std::this_thread::sleep_for(std::chrono::milliseconds(1000));
       }
    }
    else 
    {
       spdlog::info("run as robot agent.");
       std::shared_ptr<Agent> agent = std::make_shared<Agent>(configFile);
       agent->Start();
       while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
       }
    }
}

void loadConfig(const std::string& fileName, std::string& mode)
{
     std::string line;
     std::size_t pos;
     std::string result;

     std::ifstream ifs(fileName);

     while (std::getline(ifs, line)) {
         line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
         pos = line.find("=");
         if (pos != std::string::npos) {
             std::string key = line.substr(0, pos);
             std::string value = line.substr(pos + 1);
             if (key == "mode") {
                 mode = value;
             } 
         }
     }
}

