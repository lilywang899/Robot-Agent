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

int bitToIndex(unsigned char value) {
	int index = 0;
    if (value==0) {
        return index;
    }
	while (value > 1) {
		value >>= 1;
		index++;
	}
	return index + 1; // 1-based index
}
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
    g_mqttClient_ptr->setController(this); //defined in mqttClient.h

    //create driver station service and bundle to controller.
    dsService = std::make_shared<DSService>();
    dsService->setController(this); //defined in dsService.h
    dsService->Init();
    loadConfig(configFile);
    controller->Start();
    g_mqttClient_ptr->Start();
    dsService->Start();

    if(pthread_create(&thread_id, nullptr, EntryOfThread,this) != 0) {
        spdlog::critical("failed start agent thread.");
    }
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

void Agent::Run() {
    std::vector<pollfd> poll_items;

    struct pollfd item;
    item.fd = GetFd();
    item.events = POLLIN;
    item.revents = 0;
	
    poll_items.push_back(item);

	while(!shutdown_)
	{
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

void Agent::parseRobotMessage() {

}

std::shared_ptr<MESSAGE> Agent::parseDsMessage(std::shared_ptr<MESSAGE> message, TCallback callback) {
    bool enabled = bitToIndex (message->Union.smm_OutGoingRequest.PhoneNumber[3]);
    int joint_index = bitToIndex ( message->Union.smm_OutGoingRequest.PhoneNumber[15]);
    char buf_string[256]={0}; // Make sure this is big enough
    MESSAGE msg = { 0 };
    msg.sid     = COM_DS;
    msg.did     = COM_AGENT;
    msg.type    = SMM_OutGoingRequest;

    if (DS_enabled == false && enabled == true) {
        spdlog::info ("enable received");
        DS_enabled  = true;
        strcpy(buf_string,"!START");

    } else if (DS_enabled == true && enabled == false) {
        spdlog::info ("disable received");
        DS_enabled  = false;
        strcpy(buf_string,"!DISABLE");

    } else if (DS_enabled == true &&
    (message->Union.smm_OutGoingRequest.PhoneNumber[15] != 0 || DS_joint_enabled != 100)) {
        spdlog::info ("joint enable received");
        if (message->Union.smm_OutGoingRequest.PhoneNumber[15] != 0)
            if (joint_index == 9) {
                strcpy(buf_string,"#GETJPOS");
                msg.length  = strlen (buf_string);
                memcpy (msg.Union.content, buf_string, msg.length);
                auto p_message = std::make_shared<MESSAGE> (msg);
                return p_message;
            }
            else {
                DS_joint_enabled = joint_index;
            }
        spdlog::info ("joint {}  enabled", DS_joint_enabled);
        dummy_joint_angle = (message->Union.smm_OutGoingRequest.PhoneNumber[17]<<8) | message->Union.smm_OutGoingRequest.PhoneNumber[18];
        spdlog::info ("dummy_joint_angle = {}", dummy_joint_angle);
        if (dummy_joint_angle != 65535  &&
        hat_control_sent ==
        false) { // default sent from hat is -1 or 65535 in unsigned int, in
            // between each press, release counts as default so no need
            // to check repeat since there's always a default in between each press, even if same button is pressed
            hat_control_sent  = true;
            dummy_joint_angle = dummy_joint_angle / 5;
            dummy_joint_control[DS_joint_enabled] += dummy_joint_angle;
            spdlog::info ("dummy_joint_control[{}] == {}", DS_joint_enabled,
            dummy_joint_angle);

            snprintf (buf_string, sizeof (buf_string), "&%d,%d,%d,%d,%d,%d,%d",
            dummy_joint_control[0], dummy_joint_control[1],
            dummy_joint_control[2], dummy_joint_control[3], dummy_joint_control[4],
            dummy_joint_control[5], dummy_joint_control[6]);

        } else if (dummy_joint_angle == 65535)
            hat_control_sent = false;
    }
    msg.length  = strlen (buf_string);
    memcpy (msg.Union.content, buf_string, msg.length);
    auto p_message = std::make_shared<MESSAGE> (msg);

    return p_message;
}
void Agent::OnMessage (std::shared_ptr<MESSAGE> message, TCallback callback) {
    std::string result ="OK";
    callback (result);
    switch (message->sid) {
    case COM_DS: {
        std::string topic = "dummy/rx";

        std::shared_ptr<MESSAGE> p_message = Agent::parseDsMessage (message,callback);

        if (p_message->length != 0)
            g_mqttClient_ptr->publish (topic, p_message);

    } break;
    case COM_CONTROLLER: {
        char data[8] = { 0 };
        data[0]      = 0;
        data[1]      = 1;
        data[2]      = 1;
        data[3]      = 1;
        data[4]      = 1;
        data[5]      = 1;
        data[6]      = 10;
        data[7]      = 10;
        dsService->Send (data, 8);
    } break;
    case COM_MQTT_CLIENT: {
        dsService->Send((const char*)message->Union.content, message->length);
    }
    default: break;
    }
   
    counter = (counter ++)%65535;
    if (counter == 0) {
//          spdlog::info("recv mqtt message [{}].", counter);
    }
    
//    spdlog::info("Agent::OnMessage send message to driver station."); 
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
