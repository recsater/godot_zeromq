#pragma once

#include "zmq.hpp"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/classes/mutex.hpp>

#include <memory>

namespace godot {

class ZMQReceiver : public Node
{
    GDCLASS(ZMQReceiver, Node);

protected:
    static void _bind_methods();
    static String socket_type_to_string(int socketType);

public:
        Thread* thread;
        Mutex* mutex;
        Callable messageHandler;
        zmq::context_t context;
        zmq::socket_t socket;
        String _in_zmq_addr;
        int _in_socket_type;
        int _out_socket_type;

        bool _isRunning = true;
    
        ZMQReceiver();
        ~ZMQReceiver();
    
        static ZMQReceiver* new_from(String inAddr, int socketType);
        void init(String inAddr, int socketType);
        void _ready() override;
        void _process(double delta) override;
        void _thread_func();
        void _process_packet(PackedByteArray bytes, String str);
    
        void stop();
        void sendBack(Array arguments);
        void onMessage(Callable callback);
};
    
}