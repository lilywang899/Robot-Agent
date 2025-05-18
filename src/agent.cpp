/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */

#include <libwebsockets.h>
#include <string.h>
#include <signal.h>
#if defined(WIN32)
#define HAVE_STRUCT_TIMESPEC
#if defined(pid_t)
#undef pid_t
#endif
#endif
#include <pthread.h>
#include <assert.h>

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"  
#include "spdlog/fmt/ostr.h" 
#include <memory>
#include "wrapper.h"
#include <chrono>
#include <thread>
#include <mqttClient.h>
#include <controller.h>
#include "dsService.h"
#include "agent.h"
#include <chrono>

using namespace spdlog;

Agent::Agent (const std::string& configFile) :configFile(configFile)
{
    shutdown_ = false;
    counter = 0;
}

void Agent::loadConfig(const std::string& fileName) {

    g_mqttClient_ptr->loadConfig(fileName);
    dsService->loadConfig(fileName);
}

void Agent::Start() {

    //create mqtt client and bundle to controller.
    client_create();
    controller = std::make_shared<Controller>();
    g_mqttClient_ptr->setController(this);

    //create driver station service and bundle to controller.
    dsService = std::make_shared<DSService>();
    dsService->setController(this);
    dsService->Init();
    loadConfig(configFile);
    controller->Start();
    g_mqttClient_ptr->Start();
    dsService->Start();

    if(pthread_create(&thread_id, nullptr, EntryOfThread,this) != 0) {
        spdlog::critical("failed start agent thread.");
    }
<<<<<<< HEAD
=======

	MESSAGE msg = {0};
	msg.sid=COM_DS;
	msg.did=COM_AGENT;
	msg.length = 6;
	msg.type = SMM_OutGoingRequest;
	memcpy(msg.Union.content,"!START",6);
	auto p_message = std::make_shared<MESSAGE>(msg);
	std::string topic = "dummy/rx";
	g_mqttClient_ptr->publish(topic, p_message);

>>>>>>> 15a76ea (able to receive Driver Station updates)
}

void Agent::Shutdown() {
    dsService->Shutdown();
    controller->Shutdown();
    g_mqttClient_ptr->Shutdown();
    client_destroy();
    shutdown_ = true;
    if(pthread_cancel(thread_id) != 0)
    {
    }
    void *res;
    int s=1;
    s = pthread_join(thread_id,&res);
}

void* Agent::EntryOfThread(void* argv) {
    Agent* agent = static_cast<Agent*>(argv);
    agent->Run();
}
#if 0
void Agent::Run() {
    std::vector<pollfd> poll_items;

    struct pollfd item;
    item.fd = GetFd();
    item.events = POLLIN;
    item.revents = 0;
	
    poll_items.push_back(item);

	while(!shutdown_)
	{
		int rc= poll(&poll_items[0], poll_items.size(), 0);
		if(rc <= 0)
		{
		}
		else
		{
			std::vector<pollfd>::const_iterator i;
			for(i = poll_items.begin(); i != poll_items.end(); ++i)
			{
				if((*i).revents != 0)
				{
					OnProxyRequest();
				}
			}
		}
		pthread_testcancel();
	}
}
#endif 
void Agent::Run() {
    while(1) {    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    char data[8] ={0};
    data[0] = 0;
    data[1] = 1;
    data[2] = 1;
    data[3] = 1;
    data[4] = 1;
    data[5] = 1;
    data[6] = 10;
    data[7] = 10;
    dsService->Send(data,8);   
    }
}

void Agent::parseDsMessage() {

}

void Agent::parseRobotMessage() {

}
void Agent::OnMessage(std::shared_ptr<MESSAGE> message, TCallback callback) 
{
    std::string result ="OK";
     
    spdlog::info("Agent::OnMessage sid [{}],did[{}],type[{}],content[{}]", 
                 message->sid,message->did,message->type,message->Union.smm_OutGoingRequest.PhoneNumber[1]);
    callback(result);   
    switch (message->sid) 
    {
        case COM_DS:
          {
             std::string topic = "test/topic0";
             message->sid = COM_AGENT; 
             message->did = COM_CONTROLLER; 
// Enable it when the mqtt server and another client is ready.
//             g_mqttClient_ptr->publish(topic, message);
          }
          break;
       case COM_CONTROLLER:
          {
            char data[8] ={0};
            data[0] = 0;
            data[1] = 1;
            data[2] = 1;
            data[3] = 1;
            data[4] = 1;
            data[5] = 1;
            data[6] = 10;
            data[7] = 10;
            dsService->Send(data,8);   
          }
          break;
        default:
            break;
    }
   
    counter = (counter ++)%65535;
    if (counter == 0) {
//          spdlog::info("recv mqtt message [{}].", counter);
    }
    
    spdlog::info("Agent::OnMessage send message to driver station."); 
}

void Agent::Message(std::shared_ptr<MESSAGE> message, TCallback callback)
{
    std::function<void()>* admin = new std::function<void()> (std::bind(&Agent::OnMessage, this, message, callback));
    send_queue_.push(admin);
}

void Agent::OnProxyRequest()
{
	std::function<void()>* f = send_queue_.Pop();
	while(f)
	{
		(*f)();
		delete f;
		f = send_queue_.Pop();
	}
}
