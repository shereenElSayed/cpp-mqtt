#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <chrono>
#include <cstring>
#include "mqtt/async_client.h"

const std::string DFLT_SERVER_ADDRESS	{ "wss://localhost:8080" };
const std::string DFLT_CLIENT_ID		{ "cpp-mqtt" };
const std::string DFLT_USERNAME	    	{ "user" };
const std::string DFLT_PASSWORD 		{ "password" };

const std::string KEY_STORE				{ "tls.key" };
const std::string TRUST_STORE			{ "tls.crt" };

const std::string LWT_TOPIC				{ "events/disconnect" };
const std::string LWT_PAYLOAD			{ "Last will and testament." };

const std::string TOPIC("my-topic");

const int  QOS = 2;
const auto TIMEOUT = std::chrono::seconds(1);
const int  N_RETRY_ATTEMPTS = 15;



/////////////////////////////////////////////////////////////////////////////

/**
 * A callback class for use with the main MQTT client.
 */
class mqtt_client_callback : public virtual mqtt::callback
{
private:
	// The MQTT client object to use in callback functions
	mqtt::async_client& cli_;

public:
	// constructor
	mqtt_client_callback(mqtt::async_client& cli, mqtt::connect_options& connOpts):cli_(cli){}
	
	// Callback when connection is lost  (No reconnect function implemented)
	void connection_lost(const std::string& cause) override {
		std::cout << "\nConnection lost" << std::endl;
		if (!cause.empty())
			std::cout << "\tcause: " << cause << std::endl;
		exit(1);
	}

	// Callback when meassage sent with QOS > 0
	void delivery_complete(mqtt::delivery_token_ptr tok) override {
		std::cout << "Delivery ACK for mesage id: "
			<< (tok ? tok->get_message_id() : -1) << " (only for QOS > 0)" << std::endl;
	}

	// Callback when a message arrives.
	void message_arrived(mqtt::const_message_ptr msg) override {
		std::cout << "Message arrived" << std::endl;
		std::cout << "\ttopic: '" << msg->get_topic() << "'" << std::endl;
		std::cout << "\tpayload: '" << msg->to_string() << "'\n" << std::endl;
	}

	// Callback when the client connects
	// Publish a meassage 
	// Subscribe on a topic 
	void connected(const std::string& cause) override {
		std::cout << "\nConnection success" << std::endl;
		
		// Send a welcome message
		std::cout << "\nSending 🖐  message with QOS=" << QOS << "..." << std::endl;
		auto msg = mqtt::make_message("hello", "Hello secure C++ world!", QOS, false);
		mqtt::delivery_token_ptr tok = cli_.publish(msg);
		// Check if sent
		if (tok->get_return_code() == mqtt::SUCCESS){
			std::cout << "...OK, sent message with id: " << tok->get_message_id() << std::endl;
		} else {
			std::cout << "...NOK" << std::endl;
		}
		
		// Subscribe to topic
		std::cout << "\nSubscribing to topic '" << TOPIC << "'"
			<< " for client '" << DFLT_CLIENT_ID
			<< "' using QoS=" << QOS << std::endl;
		cli_.subscribe(TOPIC, QOS);
	}
};

/////////////////////////////////////////////////////////////////////////////

using namespace std;

int main(int argc, char* argv[])
{
	string	username  = (argc > 3) ? string(argv[3]) : DFLT_USERNAME,
			password  = (argc > 4) ? string(argv[4]) : DFLT_PASSWORD,
			address  = (argc > 1) ? string(argv[1]) : DFLT_SERVER_ADDRESS,
			clientID = (argc > 2) ? string(argv[2]) : DFLT_CLIENT_ID;

	// Check if files are in place
	{
		ifstream tstore(TRUST_STORE);
		if (!tstore) {
			cerr << "The trust store file does not exist: " << TRUST_STORE << endl;
			cerr << "  Get a copy from \"paho.mqtt.c/test/ssl/test-root-ca.crt\"" << endl;;
			return 1;
		}

		ifstream kstore(KEY_STORE);
		if (!kstore) {
			cerr << "The key store file does not exist: " << KEY_STORE << endl;
			cerr << "  Get a copy from \"paho.mqtt.c/test/ssl/client.pem\"" << endl;
			return 1;
		}
    }

	// Create the client object
	cout << "Initializing for server '" << address << "'..." << endl;
	cout << "\nPress Q<Enter> or Ctrl+C to quit\n" << endl;
	mqtt::async_client client(address, clientID);

	// Build the connect options, including SSL and a LWT message.
	auto sslopts = mqtt::ssl_options_builder()
					   .trust_store(TRUST_STORE)
					   .enable_server_cert_auth(false) // will fail for self signed
					   .error_handler([](const std::string& msg) {
						   std::cerr << "SSL Error: " << msg << std::endl;
					   })
					   .finalize();

    auto willmsg = mqtt::message(LWT_TOPIC, LWT_PAYLOAD, QOS, true);

	auto connopts = mqtt::connect_options_builder()
					    .user_name(username)
					    .password(password)
					    .will(std::move(willmsg))
						.ssl(std::move(sslopts))
						//.mqtt_version(MQTTVERSION_5)
						.connect_timeout(TIMEOUT)
						.finalize();

	// Create a callback object with the client and connection options
	mqtt_client_callback cb(client,connopts);

	// Set the callback object for the client
	client.set_callback(cb);						
	cout << "...OK" << endl;

	try {
		// Connect using SSL/TLS
		cout << "\nConnecting..." << endl;
		mqtt::token_ptr conntok = client.connect(connopts);
		cout << "Waiting for the connection..." << endl;
		conntok->wait_for(TIMEOUT);
		if (!client.is_connected()){
			cout << "...timeout..." << endl;
			return -1;
		}
		
		// Just block till user tells us to quit.
		while (std::tolower(std::cin.get()) != 'q');

		// Disconnect
		cout << "\nDisconnecting..." << endl;
		client.disconnect()->wait();
		cout << "...OK" << endl;
	}
	catch (const mqtt::exception& exc) {
		cerr << exc.what() << endl;
		return 1;
	}

 	return 0;
}