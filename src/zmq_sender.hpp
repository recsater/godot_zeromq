#pragma once

#include "zmq.hpp"

#include <godot_cpp/classes/node.hpp>

namespace godot {

class ZMQSender : public Node
{
    GDCLASS(ZMQSender, Node);

protected:
    static void _bind_methods();

public:
    
        zmq::context_t context;
        zmq::socket_t socket;
        String _out_zmq_addr = "tcp://localhost:5555";
    
        ZMQSender();
        ~ZMQSender();
    
        static ZMQSender* new_from(String outAddr, int socketType);
        void init(String outAddr, int socketType);
        void _ready() override;
        void _process(double delta) override;
    
        void sendBuffer(PackedByteArray buffer);
        void send(String address, Array arguments);
        void stop();
};

}