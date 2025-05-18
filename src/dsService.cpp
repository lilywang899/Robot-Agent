/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */

#include "dsService.h"
#include "spdlog/spdlog.h"
#include "message.h"
#include "agent.h"
#include <fstream>
#include <algorithm>

using namespace std::placeholders;
static const int BUFSIZE=1024;

DSService::DSService()
{
   /* Fill basic data */
   in_port = 1180;
   out_port= 1150;

   /* Fill socket info structure */
   sockfd_in   = 0;
   sockfd_out  = 0;
   buffer_size = 0;

   /* Fill strings with 0 */
   memset(address, 0, sizeof(address));
   memset(buffer, 0, sizeof(buffer));
}
void DSService::loadConfig(const std::string& fileName) 
{
     std::string line;
     std::size_t pos;
     std::string result;

     std::ifstream ifs(fileName);

     while (std::getline(ifs, line)) {
         line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
         pos = line.find("=");
         if (pos != std::string::npos) {
             std::string key = line.substr(0, pos);
             std::string value = line.substr(pos + 1);
             if (key == "in_port") {
                 in_port = atoi(value.c_str()); //atoi returns str as integer, .c_str converts into character array
             }
             else if ( key =="out_port") {
                 out_port = atoi(value.c_str());
             }
         }
     }
}
 

void DSService::Init()
{
    // Creating outgoing socket file descriptor 
    if ( (sockfd_out = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
    {
        spdlog::critical("outgoing socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
   
    memset(&serveraddr_out, 0, sizeof(serveraddr_out)); 
       
    // Filling server information 
    serveraddr_out.sin_family = AF_INET; 
    serveraddr_out.sin_port = htons(out_port); 
    serveraddr_out.sin_addr.s_addr = INADDR_ANY; 

  /* 
   * socket: create the parent socket for server.
   */
  if( (sockfd_in = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
        spdlog::critical("incoming socket creation failed"); 
        exit(EXIT_FAILURE); 
  } 

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  int optval = 1;
  setsockopt(sockfd_in, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

  bzero((char *) &serveraddr_in, sizeof(serveraddr_in));
  serveraddr_in.sin_family = AF_INET;
  serveraddr_in.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr_in.sin_port = htons((unsigned short)in_port);
}

void DSService::Start()
{
  /* 
   * bind: associate the parent socket with a port 
   */
  int result =  bind(sockfd_in, (struct sockaddr *) &serveraddr_in, sizeof(serveraddr_in));
  if( result < 0 ) 
  {
      spdlog::critical("failed to bind socketfd [{}], port [{}], errrno [{}].",sockfd_in, in_port, std::strerror(errno));
  }

   /* Initialize the socket in another thread */
  if(pthread_create(&thread_id, nullptr, &EntryOfThread,this) != 0){
      spdlog::critical("Failed to create thread for dsService.");
  }
}
void DSService::Shutdown()
{
  close(sockfd_out); 
  close(sockfd_in); 
}
std::string DSService::Read()
{
}

/*static*/
void* DSService::EntryOfThread(void* argv){

  DSService* service = static_cast<DSService*>(argv);
  service->run();
}

void DSService::run()
{
  struct sockaddr_in clientaddr;     /* client addr */
  int clientlen = sizeof(clientaddr);
  while (1) {
    bzero(buffer, BUFSIZE);
    buffer_size = recvfrom(sockfd_in, buffer, BUFSIZE, 0, (struct sockaddr *) &clientaddr ,(socklen_t*)&clientlen);
    if (buffer_size < 0){
      spdlog::critical("ERROR in recvfrom");
    }
    else 
    {
        parse(buffer,buffer_size);            
        spdlog::info("recv data [{:d} {:d} {:d} {:d} {:d} {:d} {:d} {:d} {:d} ].", 
                      buffer[0],buffer[1],buffer[2],
                      buffer[3],buffer[4],buffer[5],
                      buffer[6],buffer[7],buffer[8]);
     }

  }
}
  
void DSService::parse(const char* data, unsigned int len) 
{
    unsigned controlCode= data[3];
    unsigned requestCode= data[4];
    unsigned stationCode= data[5];
    if ( controlCode || requestCode || stationCode )
    {
        // build message and send to agent.
        MESSAGE msg = {0};
        msg.sid=COM_DS;
        msg.did=COM_AGENT;
        msg.length = len;
        msg.type = SMM_OutGoingRequest;
        memcpy(msg.Union.smm_OutGoingRequest.PhoneNumber,data, len);
      
        std::shared_ptr<MESSAGE> message = std::make_shared<MESSAGE>(msg);
        TCallback callback = std::bind<>(&DSService::AsyncResult, this, std::placeholders::_1);
        agent->Message( message,callback);
        spdlog::info("recv data [{:d} {:d} {:d} {:d} {:d} {:d} {:d} {:d} {:d} ].", 
                      buffer[0],buffer[1],buffer[2],
                      buffer[3],buffer[4],buffer[5],
                      buffer[6],buffer[7],buffer[8]);
    }
}
int DSService::Send(const char* data, int len)
{
    int n = sendto(sockfd_out, data, len, MSG_CONFIRM, (const struct sockaddr *) &serveraddr_out, sizeof(serveraddr_out)); 
    if( n< 0) 
    {
        spdlog::critical("failed to send data.");
    }
    return n;
}
void DSService::AsyncResult( std::string& result) 
{
      spdlog::info("AsyncResult is [{}]",result);
}


