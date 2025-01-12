/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */

#include <stdlib.h>
#include "wrapper.h"
#include "mqttClient.h"
	
extern mqttClient* g_mqttClient_ptr;

struct wrapper {
	void *client;
};

struct wrapper *wrapper;

void client_create()
{
	wrapper  = (struct wrapper *)malloc(sizeof(*wrapper));
	mqttClient* instance = new mqttClient();
        g_mqttClient_ptr = instance;
        wrapper->client = instance;
}

void client_destroy()
{
	if (wrapper == NULL)
		return;
	mqttClient* instance = static_cast< mqttClient *>(wrapper->client);
	delete instance;
        g_mqttClient_ptr = NULL;
	free(wrapper);
}

int callbackEx(struct lws *wsi, 
                enum lws_callback_reasons reason,  
                void *user, 
                void *in, 
                size_t len) {
	
	if (wrapper == NULL)
		return 0;
	mqttClient* instance = static_cast< mqttClient *>(wrapper->client);
	return instance->callback(wsi,reason, user,in,len);
}
int system_notify_cb(lws_state_manager_t *mgr,
                     lws_state_notify_link_t *link,
                     int current,
                     int target)
{
	if (wrapper == NULL)
		return 0;
	mqttClient* instance = static_cast< mqttClient *>(wrapper->client);
	return instance->notify_callback(mgr,link, current,target);
}
