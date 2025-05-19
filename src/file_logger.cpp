/*
* The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang, Alex Spataru and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include "nlohmann/json.hpp"
#include <fstream>

//#include "tinyxml2.h"
#include "yaml-cpp/yaml.h"

using namespace nlohmann;
//using namespace tinyxml2;
using namespace YAML;


// void basic_logfile_example()
// {
//     try
//     {
//         // auto logger = spdlog::basic_logger_mt("basic_logger", "logs/basic-log.txt");
//         // logger->info("This is a basic logger test");
//         // logger->warn("warning");
//         // logger->error("error");
//         auto logger = spdlog::basic_logger_mt("test_logger", "logs/test.txt");
//         spdlog::set_default_logger(logger);
//         spdlog::flush_on(spdlog::level::info);
//
//         spdlog::get("test_logger")->info("LoggingTest::ctor");
//     }
//     catch (const spdlog::spdlog_ex &ex)
//     {
//         printf("Log init failed: \"%s\"\n", ex.what());
//     }
// }

void rotating_logfiles(const std::string &logPath)
{
 // Create a file rotating logger with 5 MB size max and 3 rotated files
 auto max_size = 1024*1024;
 auto max_files = 3;
 auto logger = spdlog::rotating_logger_mt("test_logger", logPath, max_size, max_files);
 spdlog::set_default_logger(logger);
 spdlog::flush_on(spdlog::level::info);

 spdlog::get("test_logger")->info("LoggingTest::ctor");
}

int init_logging()
{
 try
 {
  //load config file in config folder to get the log file path specified (all log files go to log folder)
  Node configObj = LoadFile("/home/lily/Robot-Agent/config/ds_config.yaml");
  const char * logLevel = configObj["logger_config"]["log_level"].as<std::string>().c_str();
  const std::string logPath = configObj["logger_config"]["log_file_path"].as<std::string>();
  spdlog::info("yaml configObj.logger_config.log_level : {}", logLevel);
  rotating_logfiles(logPath);
 }
 catch (const YAML::BadFile& e)
 {
#if 0
  XMLDocument configObj;
  XMLError result = configObj.LoadFile( "/home/l753wang/DriverStation-Lite/config/ds_config.xml" );
  if (result != XML_SUCCESS) {
   printf("could not find .xml or .json");
   return -1;
  }
  XMLElement * RootElement = configObj.RootElement();
  const char* logLevel = RootElement->FirstChildElement( "logger_config" )->FirstChildElement( "log_level" )->GetText();
  const char* logPath = RootElement->FirstChildElement( "logger_config" )->FirstChildElement( "log_file_path" )->GetText();
  spdlog::info("xml configObj.logger_config.log_level : {}", logLevel);
  rotating_logfiles(logPath);
#endif
 }

 return 0;
}



