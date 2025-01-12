/*
 * The Driver Station Library (LibDS)
 * Copyright (c) Lily Wang and other contributors.
 * Open Source Software; you can modify and/or share it under the terms of
 * the MIT license file in the root directory of this project.
 */

#pragma once
#include <vector>
#include <string>
#include <chrono>
#include <libwebsockets.h>
#include <mutex>
#include <map>
#include <memory>
#include <functional>
#include "message.h"
#include <utility>
#include <mutex>
#include "libwebsockets/lws-mqtt.h"


class Agent;
extern class mqttClient* g_mqttClient_ptr;


struct pss {
       int		state;
       size_t	pos;
       int   	retries;

};
 
class mqttClient 
{
	/** An arbitrary, but relatively long timeout */
	static const std::chrono::seconds DFLT_TIMEOUT;

	/** The default quality of service */
	static const int DFLT_QOS;  // =1;

	/** The longest time to wait for an operation to complete.  */
	std::chrono::milliseconds timeout_;

	//void message_arrived(const_message_ptr msg); 
	
        //void delivery_complete(delivery_token_ptr tok); 

	/** Non-copyable */
	//client() =delete;

public:
	/**
	 * Create a client that can be used to communicate with an MQTT server.
	 * This allows the caller to specify a user-defined persistence object,
	 * or use no persistence.
	 * @param serverURI the address of the server to connect to, specified
	 *  				as a URI.
	 * @param clientId a client identifier that is unique on the server
	 *  			   being connected to
	 * @param persistence The user persistence structure. If this is null,
	 *  				  then no persistence is used.
	 */
	mqttClient();
	//client(const std::string& serverURI, const std::string& clientId);

	/**
	 * Virtual destructor
	 */
	~mqttClient() {}; 
	
        /**
	 * Connects to an MQTT server using the default options.
	 */
        int connect(struct lws_context *context );
        
        void loadConfig(const std::string& fileName);
	
        /**
	 * Reconnects the client using options from the previous connect.
	 * The client must have previously called connect() for this to work.
	 */
 	int reconnect();
	
        /**
	 * Disconnects from the server.
	 */
        void disconnect();

	/**
	 * Disconnects from the server.
	 * @param timeoutMS the amount of time in milliseconds to allow for
	 *  			  existing work to finish before disconnecting. A value
	 *  			  of zero or less means the client will not quiesce.
	 */

	/**
	 * Gets the client ID used by this client.
	 * @return The client ID used by this client.
	 */
        std::string get_client_id() const 
         {
             return clientId; 
         }


	/**
	 * Gets the address of the server used by this client.
	 * @return The address of the server used by this client, as a URI.
	 */
         std::string get_server_uri() const {
             return serverUri; 
         }

	/**
	 * Return the maximum time to wait for an action to complete.
	 * @return int
	 */

         std::vector<std::string> get_topic() ;

	/**
	 * Return the maximum time to wait for an action to complete.
	 * @return int
	 */

        void  set_clientId(const std::string& p_clientId) {
            clientId = p_clientId;
        }

	/**
	 * Determines if this client is currently connected to the server.
	 * @return @em true if the client is currently connected, @em false if
	 *  	   not.
	 */
	 bool is_connected() const {
             return isConnected; 
         }

	/**
	 * Publishes a message to a topic on the server and return once it is
	 * delivered.
	 * @param top The topic to publish
	 * @param payload The data to publish
	 */
	void publish( std::string& topic, const std::string& payload) ;


	/**
	 * Publishes a message to a topic on the server.
	 * This version will not timeout since that could leave the library with
	 * a reference to memory that could disappear while the library is still
	 * using it.
	 * @param msg The message
	 */
        void publish( std::string& topic, const std::shared_ptr<MESSAGE>& message);

        void processMessage(void* in, size_t len, struct lws* wsi );

	/**
	 * Requests the server unsubscribe the client from a topic.
	 * @param topicFilter A single topic to unsubscribe.
	 * @param props The MQTT v5 properties.
	 * @return The "unsubscribe" response from the server.
	 */
	int unsubscribe(const std::string& topicFilter);

        int callback(struct lws *wsi, 
                     enum lws_callback_reasons reason,  
                     void *user, 
                     void *in, 
                     size_t len);
        int notify_callback(lws_state_manager_t *mgr,
                            lws_state_notify_link_t *link,
                            int current,
                            int target);
        void Start();

        void Shutdown();

        void run();
   
        static void* EntryOfThread(void* arg);

        void addWsiInstance( std::string& componentName, struct lws *wsi);
        void removeWsiInstance( std::string& componentName);
        void setController(Agent* p_agent)
        {
            agent = p_agent;
        }

        struct lws* getWsiInstance( std::string& componentName);

        void AsyncResult( std::string& result);
        void onClientWriteAble(struct lws *wsi, struct pss* pss);

        bool shutdown_;
        pthread_t thr_id_;

        bool isConnected;
        std::vector<lws_mqtt_topic_elem_t> mqtt_topics;
        std::vector<std::string> topics;
        std::string clientId;
        std::string username;
        std::string password;
        std::string serverUri;
        std::string address;
        std::string port;
        std::string host;
   
	struct lws_context *context;
      
        lws_mqtt_subscribe_param_t sub_param;
        lws_mqtt_publish_param_t   pub_param;

        using MqttMessage_ = std::pair<std::string, std::shared_ptr<MESSAGE> >;
        std::vector<MqttMessage_> messages;
        /*
        If you want to keep a list of live wsi, 
        you need to use lifecycle callbacks on the protocol in the service 
        thread to manage the list, with your own locking. 
        Typically you use an ESTABLISHED callback to add ws wsi to your list and 
        a CLOSED callback to remove them.*/
        using wsi_map_type_ = std::map<std::string,struct lws*>;
        wsi_map_type_ wsi_map;
        std::mutex  wsiMapMutex;
        Agent* agent;
        std::mutex mqttMutex;

};