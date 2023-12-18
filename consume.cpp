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

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include "mqtt/async_client.h"

using namespace std;

// const string SERVER_ADDRESS	{ "tcp://localhost:1883" };
// const string CLIENT_ID		{ "paho_cpp_async_consume" };
// const string TOPIC = { "command" };

// const vector<string> TOPICS { "data/#", "command" };

const int  QOS = 1;

/////////////////////////////////////////////////////////////////////////////



int main(int argc, char* argv[])
{
	char address[500];
	char clientID[500];
	auto TOPICS = mqtt::string_collection::create({"general"});
	const vector<int> QOS;

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
				TOPICS.push_back(str);
				QOS.push_back(1);
				
			default:
				fprintf(stderr,
				"unrecognized command %c\n",(char)c);
				fprintf(stderr,"%s",Usage);
				exit(1);
		}
	}
	auto cli = std::make_shared<mqtt::async_client>(address, CLIENT_ID);

	auto connOpts = mqtt::connect_options_builder()
		.clean_session(true)
		.automatic_reconnect(seconds(2), seconds(30))
		.finalize();

	try {
		// Start consumer before connecting to make sure to not miss messages

		cli.start_consuming();

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

		while (true) {
			auto msg = cli.consume_message();
			if (!msg) continue;
			cout << msg->get_topic() << ": " << msg->to_string() << endl;
		}

		// If we're here, the client was almost certainly disconnected.
		// But we check, just to make sure.

		if (cli.is_connected()) {
			cout << "\nShutting down and disconnecting from the MQTT server..." << flush;
			// cli.unsubscribe(TOPIC)->wait();
            // cli.unsubscribe(TOPIC_2)->wait();
			cli.stop_consuming();
			cli.disconnect()->wait();
			cout << "OK" << endl;
		}
		else {
			cout << "\nClient was disconnected" << endl;
		}
	}
	catch (const mqtt::exception& exc) {
		cerr << "\n  " << exc << endl;
		return 1;
	}

 	return 0;
}
