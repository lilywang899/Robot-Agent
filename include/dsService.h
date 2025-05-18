/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */

#pragma once
#include <string>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <pthread.h>
#include <memory>
class Agent;

class DSService {
 public:   
  DSService(); 
  ~DSService(){};
  void Init();
  void Start();
  void Shutdown();
  static void* EntryOfThread(void* argv);
  int Send(const char* data, int len);
  void setController(Agent* p_agent)
  {
     agent = p_agent;
  }
  void loadConfig(const std::string& fileName);

  void AsyncResult( std::string& result);

 private:
  void run();
  std::string Read();
  void parse(const char* data,unsigned int len);

  // port used to get command from Driver station.
  int in_port;        /**< Input port number */
  int sockfd_in;      /**< Input socket file descriptor */
  struct sockaddr_in serveraddr_in;  /* server's addr */
  size_t buffer_size; /**< Holds the number of received bytes */
  char buffer[1024];  /**< Holds the received data buffer */

  // port used to send data to  Driver station.
  int out_port;       /**< Output port number */
  int sockfd_out;     /**< Output socket file descriptor */
  struct sockaddr_in serveraddr_out; /* server's addr */
  char address[512];  /**< Address of remote host */

  Agent* agent;
  pthread_t thread_id;
};

