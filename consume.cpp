// async_consume.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// This application is an MQTT consumer/subscriber using the C++
// asynchronous client interface, employing the  to receive messages
// and status updates.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker.
//  - Subscribing to a topic
//  - Receiving messages through the synchronous queuing API
//

/*******************************************************************************
 * Copyright (c) 2013-2020 Frank Pagliughi <fpagliughi@mindspring.com>
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Frank Pagliughi - initial implementation and documentation
 *******************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
// #include <cstring>
#include <cctype>
// #include <thread>
#include <chrono>
#include "cspot_mqtt.h"
#include <woofc.h>
#include "mqtt/async_client.h"
#define _BSD_SOURCE

#include <sys/time.h>

using namespace std;

// const string SERVER_ADDRESS	{ "tcp://localhost:1883" };
// const string CLIENT_ID		{ "paho_cpp_async_consume" };
// const string TOPIC = { "command" };

// const vector<string> TOPICS { "data/#", "command" };

const int  QOS = 1;

/////////////////////////////////////////////////////////////////////////////



int main(int argc, char* argv[])
{
	#define ARGS "h:c:t:"
	char address[500];
	char clientID[500];
	
	std::vector<std::string> topics;
	std::vector<int> QOS;
	int c;

	while((c = getopt(argc,argv,ARGS)) != EOF) {
		switch(c) {
			case 'h':
				strncpy(address,optarg,sizeof(address));
				break;
			case 'c':
				strncpy(clientID,optarg,sizeof(clientID));
				break;
			case 't':
				char topic_name[1024];
				strncpy(topic_name,optarg,sizeof(topic_name));
				std::string str(topic_name);
				topics.push_back(str);
				QOS.push_back(1);
				break;
		}
	}

	WooFInit();
	int err;
	for(auto topic : topics){
		err = WooFCreate(topic.c_str(), sizeof(EL), 1025); /**/
		if (err < 0)
		{
			fprintf(stderr, "WooFCreate for %s\n",topic.c_str());
			fflush(stderr);
			exit(1);
		}
	}


	auto TOPICS = mqtt::string_collection::create(topics);
	auto cli = std::make_shared<mqtt::async_client>(address, clientID);

	auto connOpts = mqtt::connect_options_builder()
		.clean_session(true)
		.automatic_reconnect(std::chrono::seconds(2), std::chrono::seconds(30))
		.finalize();

	try {
		// Start consumer before connecting to make sure to not miss messages

		cli->start_consuming();

		// Connect to the server
		cout << "Connecting to the MQTT server at " << address << "..." << flush;
		auto rsp = cli->connect(connOpts)->get_connect_response();
		cout << "OK\n" << endl;
		if (!rsp.is_session_present()){
            cli->subscribe(TOPICS, QOS);
            printf("subscribing\n");
        }

		// Consume messages
		// This just exits if the client is disconnected.
		// (See some other examples for auto or manual reconnect)

		// cout << "Waiting for messages on topic: '" << TOPIC << "'" << endl;
		struct timeval  tv;
		gettimeofday(&tv,NULL);
		double start=((double)((double)tv.tv_sec + (double)tv.tv_usec/1000000.0));
		printf("Start time: %f ms", start*1000);
		fflush(stdout);
		double time;


		while (true) {
			auto msg = cli->consume_message();
			if (!msg) continue;
			
			//<CAP_TOKEN>:......<ID_TOKEN>:....<MESSAGE>:value
			cout << msg->get_topic() << ": " << msg->to_string() << endl;
			std::string msg_str = msg->to_string();
			// int cap_token_index = msg_str.rfind("<CAP_TOKEN");
			int id_token_index = msg_str.find("<ID_TOKEN>:");
			int message_index = msg_str.find("<MESSAGE>:");
			//printf("id_token_index: %c  message_index : %c\n", msg_str[ id_token_index], msg_str[ message_index]); 
			std::string cap_token = msg_str.substr(11,id_token_index-11);
			// printf("cap_token: %s\n",cap_token.c_str());
			std::string id_token = msg_str.substr(id_token_index+11, message_index-id_token_index-11);
			// printf("id_token: %s\n",id_token.c_str());
			int value = atoi(msg_str.substr(message_index+10).c_str());
			
			gettimeofday(&tv,NULL);
			time =((double)((double)tv.tv_sec + (double)tv.tv_usec/1000000.0));
			EL element;
			element.start_time = time*1000;
			element.value = value;
			err = WooFPutWithToken(cap_token.c_str(), id_token.c_str(), msg->get_topic().c_str(), (msg->get_topic()+"_handler").c_str(),(void *)&element);

			if(WooFInvalid(err)) {
				fprintf(stderr,"Failed to put\n");
				break;
			}

		}

		// If we're here, the client was almost certainly disconnected.
		// But we check, just to make sure.

		
		cout << "\nClient was disconnected" << endl;
		cli->disconnect();
		cout << "OK" << endl;
	}
	catch (const mqtt::exception& exc) {
		cerr << "\n  " << exc << endl;
		return 1;
	}

 	return 0;
}
