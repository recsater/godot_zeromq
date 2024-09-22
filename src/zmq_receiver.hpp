#pragma once

#include "zmq.hpp"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/classes/mutex.hpp>

namespace godot {

class ZMQReceiver : public Node
{
    GDCLASS(ZMQReceiver, Node);

protected:
    static void _bind_methods();

public:
    
        Callable messageHandler;
        Thread* thread;
        Mutex* mutex;
        zmq::context_t context;
        zmq::socket_t socket;
        String _in_zmq_addr = "tcp://*:5555";

        bool _isRunning = true;
    
        ZMQReceiver();
        ~ZMQReceiver();
    
        static ZMQReceiver* new_from(String inAddr, int socketType);
        void init(String inAddr, int socketType);
        void _ready() override;
        void _process(double delta) override;
        void _thread_func();
        void _process_packet(PackedByteArray packet);
    
        void stop();
        void onMessage(Callable callback);
};
    
}