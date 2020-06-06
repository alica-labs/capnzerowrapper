//
// Created by link9 on 02.07.19.
//

#include <iostream>
#include <capnzero/CapnZero.h>
#include <zmq.h>
#include <thread>
#include <chrono>
#include <vector>
#include <string>

#include "capnzerowrapper.h"

bool running = true;

// split string by string delimiter (https://stackoverflow.com/a/46931770/12799662)
std::vector<std::string> split (std::string s, std::string delimiter) {
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	std::string token;
	std::vector<std::string> res;

	while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
		token = s.substr (pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back (token);
	}

	res.push_back (s.substr (pos_start));
	return res;
}

void recv_msg(void* socket) {
    while(running) {
    	const char* msg = receiveSerializedMessage(socket, capnzero::Protocol::UDP);
        std::cout << "received: " << msg << std::endl;
        std::vector<std::string> v = split(msg, "::");
        freeStr(msg[0]);
    }
}


int main(int argc, char** argv) {

    void* ctx = zmq_ctx_new();

    // sub
    void* sub_socket = zmq_socket(ctx, ZMQ_DISH);
    int timeout(500);
    zmq_setsockopt(sub_socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    zmq_join(sub_socket, "aeinfo");
    zmq_bind(sub_socket, "udp://224.0.0.2:5555");
    std::thread* t = new std::thread(&recv_msg, sub_socket);
    t->join();

    // pub
//    void* socket = zmq_socket(ctx, ZMQ_RADIO);
//    zmq_connect(socket, "udp://224.0.0.1:5555");
//    sendMessage(socket, capnzero::Protocol::UDP, "MCGroup", "Hello World man!");
//    std::cout << "sent \"Hello World man!\"" << std::endl;

	while(running) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

    return 0;
}