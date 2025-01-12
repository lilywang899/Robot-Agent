/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */
#include <functional>
#include  <iostream>
#include <sys/poll.h>
#include "controller.h"

Controller::Controller() 
{
    shutdown_ = false;
}

int Controller::Start()
{
	if(pthread_create(&thr_id_, nullptr, EntryOfThread,this) != 0)
	{
            return -1;
	}
	return 0;
}

void Controller::Shutdown()
{
	shutdown_ = true;
	if(pthread_cancel(thr_id_) != 0)
	{
	}
	void *res;
	int s=1;
	s = pthread_join(thr_id_,&res);
}

Controller::~Controller()
{
    std::cout << "Destroyed controller" << std::endl;
    std::cout.flush();
}
/*static*/
void* Controller::EntryOfThread(void* arg)
{
    Controller* pMgr = static_cast<Controller*>(arg);
    pMgr->Run();
}

void Controller::Run()
{
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

void Controller::OnMessage(const std::string& p_topic, std::shared_ptr<MESSAGE> message, TCallback callback) 
{
    std::cout << "Controller::OnMessage hit"<<std::endl;
    int ret = 0;
    std::string result ="OK";
     
    spdlog::info("Controller::OnMessage sid [{}],did[{}],type[{}],content[{}]", 
                 message->sid,message->did,message->type,message->Union.smm_OutGoingRequest.PhoneNumber[1]);
    callback(result);
   
}

void Controller::Message(const std::string& topic,std::shared_ptr<MESSAGE> message, TCallback callback)
{
	std::function<void()>* admin = new std::function<void()> (std::bind(&Controller::OnMessage, this, topic, message, callback));
	send_queue_.push(admin);
}

void Controller::OnProxyRequest()
{
	std::function<void()>* f = send_queue_.Pop();
	while(f)
	{
		(*f)();
		delete f;
		f = send_queue_.Pop();
	}
}
   
