/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */

#include "wrapper.h"
#include "mqttClient.h"
#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"  
#include "spdlog/fmt/ostr.h" 
#include <functional>
#include <chrono>
#include <thread>
#include "spdlog/spdlog.h"
#include "controller.h"
#include <fstream>
#include "agent.h"

using namespace std::placeholders;

class mqttClient* g_mqttClient_ptr;

enum {
	STATE_SUBSCRIBE,	/* subscribe to the topic */
	STATE_PUBLISH_QOS0,	/* Send the message in QoS0 */
	STATE_WAIT_ACK0   	/* Wait for the synthetic "ack" */
};

mqttClient::mqttClient()
{
   shutdown_ = false;
   clientId = "MqttClient";
}
    
void mqttClient::loadConfig(const std::string& fileName)
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
             if (key == "topic") {
                 topics.emplace_back(value);
             }
             else if ( key =="client_id") {
                 clientId = value;
             }
             else if ( key =="username") {
                 username = value;
             }
             else if ( key =="password") {
                 password = value;
             }
             else if ( key =="address") {
                 address = value;
             }
             else if ( key =="host") {
                 host = value;
             }
             else if ( key =="port") {
                 port = value;
             }
         }
     }
     for(auto topic:topics) {
         spdlog::info("topic:[{}]",topic);
     }
     spdlog::info("clientId:[{}], username:[{}], password:[{}],address:[{}],host:[{}].", 
                   clientId, username, password, address, host);
}

void mqttClient::Start() {
 if(pthread_create(&thr_id_, nullptr, EntryOfThread,this) != 0)
 {
     spdlog::critical("Failed to create thread for mqtt Client.");
 }
}

void mqttClient::Shutdown() {
    shutdown_ = true;
    if(pthread_cancel(thr_id_) != 0)
    {
    }
    void *res;
    int s=1;
    s = pthread_join(thr_id_,&res);
}

/*static*/
void* mqttClient::EntryOfThread(void* arg)
{
    mqttClient* pClient = static_cast<mqttClient*>(arg);
    pClient->run();
}


void mqttClient::run() {

  const struct lws_protocols protocols[] = {
     {
	.name			= "mqtt",
	.callback		= &callbackEx,
	.per_session_data_size	= sizeof(struct pss)
     },
     LWS_PROTOCOL_LIST_TERM
  };
  const lws_retry_bo_t retry = {
	.secs_since_valid_ping		= 20, /* if idle, PINGREQ after secs */
	.secs_since_valid_hangup	= 25, /* hangup if still idle secs */
  };

  lws_state_notify_link_t notifier = { { NULL, NULL, NULL }, &system_notify_cb, "app" };
  lws_state_notify_link_t *na[] = { &notifier, NULL };

  struct lws_context_creation_info info;
  memset(&info, 0, sizeof info); 

  info.port = CONTEXT_PORT_NO_LISTEN;
  info.protocols = protocols;
  info.register_notifier_list = na;
  info.fd_limit_per_thread = 1 + 1 + 1;
  info.retry_and_idle_policy = &retry;
  //lws_set_log_level(LLL_USER|LLL_ERR|LLL_WARN|LLL_NOTICE|LLL_CLIENT|LLL_DEBUG|LLL_PARSER, NULL);
  lws_set_log_level(LLL_USER|LLL_ERR|LLL_WARN|LLL_NOTICE|LLL_CLIENT, NULL);

  context = lws_create_context(&info);
  if (!context) { 
     lwsl_err("lws init failed\n");
  }

  int n = 0;   
  while( (n >= 0 )&&(shutdown_ == false)) {
   n = lws_service(context, 0);
  }            
  lws_context_destroy(context);
  spdlog::info("exit from thread.");
}

int mqttClient::connect(struct lws_context *pcontext ){
     lws_mqtt_client_connect_param_t client_connect_param = {
	.client_id			= clientId.c_str(),
	.keep_alive			= 60,
	.clean_start			= 1,
	.client_id_nofree		= 1,
	.username_nofree		= 1,
	.password_nofree		= 1,
	.will_param = {
		.topic			= "good/bye",
		.message		= "sign-off",
		.qos			= static_cast<lws_mqtt_qos_levels_t>(0),
		.retain			= 0,
	},
	.username			= username.c_str(),
	.password			= password.c_str(),
    };

    struct lws_client_connect_info info;

    memset(&info, 0, sizeof info);

    info.mqtt_cp = &client_connect_param;
    info.address = address.c_str();
    info.host    = host.c_str();
    info.protocol = "mqtt";
    info.context  = pcontext;
    info.method = "MQTT";
    info.alpn = "mqtt";
    info.port = atoi(port.c_str());

    spdlog::info("connection client_id:[{}],username:[{}],address:[{}],host:[{}],port:[{}]",
                 client_connect_param.client_id, client_connect_param.username,
                 info.address, info.host, info.port);
    if (!lws_client_connect_via_info(&info)) 
    {
	lwsl_err("%s: Client Connect Failed\n", __func__);
	return 1;
    }
    return 0;
}

int mqttClient::reconnect(){

}

void mqttClient::disconnect(){

}

void mqttClient::publish( std::string& p_topic, const std::shared_ptr<MESSAGE>& p_message) {
        
    MqttMessage_ msg = std::make_pair(p_topic,p_message);
    {
      std::lock_guard<std::mutex> lock(mqttMutex);
      messages.emplace_back(msg);
    }

    std::string componentName = "app"; 
    struct lws* wsi = getWsiInstance(componentName);
    lws_callback_on_writable(wsi);
}

void mqttClient::publish( std::string& topic, const std::string& payload) {
    spdlog::warn("payload only support MESSAGE.");
#if 0    
    pub_param.topic	= const_cast<char*>(topic.c_str());
    pub_param.topic_len = (uint16_t)strlen(pub_param.topic);
    pub_param.qos = QOS0; //by default.
    pub_param.payload_len = payload.length();
    pub_payload = payload;

    std::string componentName = "app"; 
    struct lws* wsi = getWsiInstance(componentName);
    spdlog::info("publish topic:{}, len:{}, qos:{}, payload:{}", pub_param.topic, pub_param.topic_len, pub_param.qos,pub_param.payload_len);
    lws_callback_on_writable(wsi);
#endif 
}

int mqttClient::unsubscribe(const std::string& topicFilter){ 

}

int mqttClient::notify_callback(lws_state_manager_t *mgr,
                                lws_state_notify_link_t *link,
                                int current,
                                int target) 
{
      context = (struct lws_context*)mgr->parent;

      if (current != LWS_SYSTATE_OPERATIONAL ||  target != LWS_SYSTATE_OPERATIONAL)
      {
           return 0;
      }

     /*
      * We delay trying to do the client connection until
      * the protocols have been initialized for each
      * vhost... this happens after we have network and
      * time so we can judge tls cert validity.
      */

      if (connect(context))
      {
        spdlog::info("failed to setup connection");

      }
      return 0;
}

void mqttClient::onClientWriteAble(struct lws *wsi, struct pss* pss)
{
    switch (pss->state) 
    {
	case STATE_SUBSCRIBE:
        {
  	  lwsl_user("%s: WRITEABLE: Subscribing\n", __func__);
          for(int idx = 0; idx < topics.size(); ++idx)
          {
              lws_mqtt_topic_elem_t elem;
              elem.name= topics[idx].c_str();
              elem.qos=QOS0;
              mqtt_topics.emplace_back(elem);
           }

           sub_param.num_topics = topics.size();
           sub_param.topic = &mqtt_topics[0];

           if (lws_mqtt_client_send_subcribe(wsi, &sub_param)) 
            {
		lwsl_notice("%s: subscribe failed\n", __func__);
	    }
            pss->state=STATE_PUBLISH_QOS0;
          }                        
	  break;

	case STATE_PUBLISH_QOS0:
              {
                  MqttMessage_ elem;
                  { std::lock_guard<std::mutex> lock(mqttMutex);
                    elem = messages.front();
                    messages.erase(messages.begin());
                  }

               pub_param.topic = const_cast<char*>(elem.first.c_str());
               pub_param.topic_len = elem.first.length();
               pub_param.qos =QOS0;
               pub_param.payload_len = elem.second->length;
               spdlog::info("publish topic [{}], len [{}]", pub_param.topic, pub_param.payload_len);
               if (lws_mqtt_client_send_publish(wsi, &pub_param, elem.second.get(), pub_param.payload_len, 1)) 
               {
                  lwsl_user("%s: failed to send publish.\n", __func__);
               }
              }

        default:
               break;
      }
}
int mqttClient::callback( struct lws *wsi,  enum lws_callback_reasons reason,  void *user,  void *in, size_t len)
{
        //spdlog::info("callback is hit, reason {}", reason);
	struct pss *pss = (struct pss *)user;

	switch (reason) {
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_err("%s: CLIENT_CONNECTION_ERROR: %s\n", __func__, in ? (char *)in : "(null)");
		break;

	case LWS_CALLBACK_MQTT_CLIENT_CLOSED:
                {
		lwsl_user("%s: CLIENT_CLOSED\n", __func__);
                std::string componentName = "app";
                removeWsiInstance(componentName);
                }
		break;

	case LWS_CALLBACK_MQTT_CLIENT_ESTABLISHED:
		lwsl_user("%s: MQTT_CLIENT_ESTABLISHED\n", __func__);
                {
                std::string componentName = "app"; 
                addWsiInstance(componentName,wsi);
		lws_callback_on_writable(wsi);
                }
		return 0;

	case LWS_CALLBACK_MQTT_SUBSCRIBED:
		lwsl_user("%s: MQTT_SUBSCRIBED\n", __func__);
		break;

	case LWS_CALLBACK_MQTT_CLIENT_WRITEABLE:
                onClientWriteAble(wsi,pss);
		break;
	case LWS_CALLBACK_MQTT_ACK:
		lwsl_user("%s: MQTT_ACK\n", __func__);
                if ( pss->state == STATE_PUBLISH_QOS0 )
                {
                    std::lock_guard<std::mutex> lock(mqttMutex);
                    if(! messages.empty())
                    {
  		        lws_callback_on_writable(wsi);
                    }
                }
         	break;
	case LWS_CALLBACK_MQTT_RESEND:
		lwsl_user("%s: MQTT_RESEND\n", __func__);
		/*
		 * We must resend the packet ID mentioned in len
		 */
		if (++pss->retries == 3) {
			break;
		}
		pss->state--;
		pss->pos = 0;
		break;

	case LWS_CALLBACK_MQTT_CLIENT_RX:

		lwsl_user("%s: MQTT_CLIENT_RX\n", __func__);
                processMessage(in, len, wsi);
/*
	    {
	        Client * const client = (Client *)user;
	        const size_t remaining = lws_remaining_packet_payload(wsi);

	        if (!remaining && lws_is_final_fragment(wsi)) {
	            if (client->HasFragments()) {
	                client->AppendMessageFragment(in, len, 0);
	                in = (void *)client->GetMessage();
	                len = client->GetMessageLength();
	            }

	            client->ProcessMessage((char *)in, len, wsi);
	            client->ResetMessage();
	        } else
	            client->AppendMessageFragment(in, len, remaining);
	    }
*/            
		return 0;
	default:
		break;
	}

	return 0;
}
        
void mqttClient::processMessage(void* in, size_t len, struct lws* wsi)
{
      lws_mqtt_publish_param_t* pub_param = (lws_mqtt_publish_param_t *)in;
      assert(pub_param);

      lwsl_hexdump_notice(pub_param->topic, pub_param->topic_len);
      lwsl_hexdump_notice(pub_param->payload, pub_param->payload_len);
      
      unsigned char phoneNumber[]={1,3,9,1,1,2,9,5,4,6,7};
      MESSAGE msg = {0};
      msg.sid=0x1;
      msg.did=0x2;
      msg.length = sizeof(phoneNumber)/sizeof(phoneNumber[0]);
      msg.type = SMM_OutGoingRequest;
      memcpy(msg.Union.smm_OutGoingRequest.PhoneNumber,phoneNumber,sizeof(phoneNumber)/sizeof(phoneNumber[0]));
      std::shared_ptr<MESSAGE> message = std::make_shared<MESSAGE>(msg);

      TCallback callback = std::bind<>(&mqttClient::AsyncResult, this, std::placeholders::_1);
      agent->Message(message,callback);
}

void mqttClient::AsyncResult( std::string& result) 
{
      spdlog::info("AsyncResult is [{}]",result);
}

void mqttClient::addWsiInstance( std::string& componentName, struct lws *wsi)
{
    const std::lock_guard<std::mutex> lock(wsiMapMutex);
    wsi_map_type_::iterator itmap = wsi_map.find(componentName);
    if(itmap == wsi_map.end())
    {
        std::pair<std::string, struct lws*> instance = std::make_pair(componentName, wsi);
        wsi_map.insert(instance);
        spdlog::trace("wsi instance for [{}] is added.", componentName);
    }
    else {
        spdlog::warn("wsi instance for [{}] exist.", componentName);
    }
}

void mqttClient::removeWsiInstance( std::string& componentName)
{
    const std::lock_guard<std::mutex> lock(wsiMapMutex);
    wsi_map_type_::iterator itmap = wsi_map.find(componentName);
    if(itmap == wsi_map.end())
    {
       spdlog::warn("wsi instaYnce for [{}] does not exist.", componentName);
    }
    else 
    {
        wsi_map.erase(itmap);
        spdlog::trace("wsi instaYnce for [{}] is removed.", componentName);
    }
}

struct lws* mqttClient:: getWsiInstance( std::string& componentName)
{
    struct lws* instance = NULL;  
    const std::lock_guard<std::mutex> lock(wsiMapMutex);
    wsi_map_type_::iterator itmap = wsi_map.find(componentName);
    if(itmap == wsi_map.end())
    {
       spdlog::warn("wsi instance for [{}] does NOT exist.", componentName);
    }
    else
    {
        instance = itmap->second;
    }
    return instance;
}
