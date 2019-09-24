//
// Created by link9 on 02.07.19.
//

#include <iostream>
#include <capnzero/CapnZero.h>
#include <zmq.h>
#include <thread>

#include "capnzerowrapper.h"

bool running = true;

void recv_msg(void* socket) {
    while(running) {
        std::cout << "received: " << receiveSerializedMessage(socket, capnzero::Protocol::UDP) << std::endl;
    }
}

int main(int argc, char** argv) {

    void* ctx = zmq_ctx_new();

    // sub
    void* sub_socket = zmq_socket(ctx, ZMQ_DISH);
    int timeout(500);
    zmq_setsockopt(sub_socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    zmq_join(sub_socket, "MCGroup");
    zmq_bind(sub_socket, "udp://224.0.0.1:5555");
    std::thread* t = new std::thread(&recv_msg, sub_socket);
    t->join();

    // pub
    void* socket = zmq_socket(ctx, ZMQ_RADIO);
    zmq_connect(socket, "udp://224.0.0.1:5555");
    sendMessage(socket, capnzero::Protocol::UDP, "MCGroup", "Hello World man!");
    std::cout << "sent \"Hello World man!\"" << std::endl;


    return 0;
}