#pragma once

#include <capnzero/CapnZero.h>
#include <capnzero-base-msgs/string.capnp.h>
#include "../include/alica_capnz_msg/AlicaEngineInfo.capnp.h"

extern "C" {
static void cleanUpMsgData(void* data, void* hint) {
    delete reinterpret_cast<kj::Array<capnp::word>*>(hint);
}

int sendMessage(void *socket, capnzero::Protocol protocol, const char *c_topic, const char *c_message) {
    std::string message(c_message);
    std::string topic(c_topic);

    assert(topic.length() < capnzero::MAX_TOPIC_LENGTH && "The given topic is too long!");

    // init builder
    ::capnp::MallocMessageBuilder msgBuilder;
    capnzero::String::Builder beaconMsgBuilder = msgBuilder.initRoot<capnzero::String>();

    // set content
    beaconMsgBuilder.setString(message);

    // setup zmq msg
    zmq_msg_t msg;
    int sumBytesSend = 0;

    kj::Array<capnp::word> wordArray = capnp::messageToFlatArray(msgBuilder);
    kj::Array<capnp::word> *wordArrayPtr = new kj::Array<capnp::word>(kj::mv(wordArray)); // will be deleted by zero-mq
    capnzero::check(zmq_msg_init_data(&msg, wordArrayPtr->begin(), wordArrayPtr->size() * sizeof(capnp::word),
                                      &cleanUpMsgData,
                                      wordArrayPtr), "zmq_msg_init_data");

    // set group
    if (protocol == capnzero::Protocol::UDP) {
//        std::cout << "Publisher: Sending on Group '" << topic << "'" << std::endl;
        capnzero::check(zmq_msg_set_group(&msg, topic.c_str()), "zmq_msg_set_group");
    } else {
        // for NON-UDP via multi part messages
        zmq_msg_t topicMsg;
        capnzero::check(zmq_msg_init_data(&topicMsg, &topic, topic.size() * sizeof(topic), NULL, NULL), "zmq_msg_init_data for topic");
        sumBytesSend = capnzero::checkSend(zmq_msg_send(&topicMsg, socket, ZMQ_SNDMORE), topicMsg, "Publisher-topic");
        if (sumBytesSend == 0) {
            // sending topic did not work, so stop here
            return sumBytesSend;
        }
    }

    sumBytesSend += capnzero::checkSend(zmq_msg_send(&msg, socket, 0), msg, "Publisher-content");
    return sumBytesSend;
}

const char* receiveSerializedMessage(void *socket, capnzero::Protocol protocol) {
    if (protocol != capnzero::Protocol::UDP) {
        zmq_msg_t topic;
        capnzero::check(zmq_msg_init(&topic), "zmq_msg_init");
        //zmq_msg_recv(&topic, socket, 0);
        if (0 == capnzero::checkReceive(zmq_msg_recv(&topic, socket, ZMQ_SNDMORE), topic, "Subscriber::receive-topic")) {
            // error or timeout on recv
            return "";
        }
    }

    zmq_msg_t msg;
    capnzero::check(zmq_msg_init(&msg), "zmq_msg_init");
    if (0 == capnzero::checkReceive(zmq_msg_recv(&msg, socket, 0), msg, "Subscriber::receive")) {
        // error or timeout on recv
        char const *p = "";
        return p;
    }

    // Received message must contain an integral number of words.
    if (zmq_msg_size(&msg) % capnzero::Subscriber::WORD_SIZE != 0) {
        std::cerr << "Non-Integral number of words!" << std::endl;
        capnzero::check(zmq_msg_close(&msg), "zmq_msg_close");
        return "";
    }

    // Check whether message is memory aligned
    //assert(reinterpret_cast<uintptr_t>(zmq_msg_data(&msg)) % capnzero::Subscriber::wordSize == 0);


    int msgSize = zmq_msg_size(&msg);
    auto wordArray = kj::ArrayPtr<capnp::word const>(reinterpret_cast<capnp::word const *>(zmq_msg_data(&msg)),
                                                     msgSize);
    ::capnp::FlatArrayMessageReader msgReader = ::capnp::FlatArrayMessageReader(wordArray);

    capnzero::check(zmq_msg_close(&msg), "zmq_msg_close");

    std::string message = msgReader.getRoot<alica_capnz_msgs::AlicaEngineInfo>().toString().flatten().cStr();
    //std::cout << "C_DEBUG AlicaEngineInfo: " << message << std::endl;
    return strdup(message.c_str());
}

void freeStr(char* str) {
    if(str != nullptr) {
        free(str);
    }
}
}
