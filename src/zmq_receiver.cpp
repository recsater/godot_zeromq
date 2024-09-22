#include "zmq_receiver.hpp"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZMQReceiver::_bind_methods() {
    ClassDB::bind_static_method("ZMQReceiver", D_METHOD("new_from", "inAddr", "socketType"), &ZMQReceiver::new_from);
    ClassDB::bind_method(D_METHOD("init", "inAddr", "socketType"), &ZMQReceiver::init);
    ClassDB::bind_method(D_METHOD("stop"), &ZMQReceiver::stop);
    ClassDB::bind_method(D_METHOD("onMessage", "address", "callback"), &ZMQReceiver::onMessage);
}

ZMQReceiver::ZMQReceiver()
{
    // UtilityFunctions::print("ZMQReceiver::constructor");
}

ZMQReceiver::~ZMQReceiver()
{
    // UtilityFunctions::print("ZMQReceiver::destructor");
}

ZMQReceiver* ZMQReceiver::new_from(String inAddr, int socketType) {
    // UtilityFunctions::print("ZMQReceiver::new_from");

    ZMQReceiver* zmq_receiver = memnew(ZMQReceiver);
    zmq_receiver->init(inAddr, socketType);

    return zmq_receiver;
}

void ZMQReceiver::init(String inAddr, int socketType) {
    // UtilityFunctions::print("ZMQReceiver::init");
    // UtilityFunctions::print("ZMQReceiver::init inAddr: " + inAddr + " socketType: " + String::num_int64(socketType));
    _in_zmq_addr = inAddr;

    context = zmq::context_t(1);
    socket = zmq::socket_t(context, static_cast<int>(socketType));
    socket.connect(inAddr.utf8().get_data());

    thread = memnew(Thread);
    mutex = memnew(Mutex);

    thread->start(Callable(this, "_thread_func"));
}

void ZMQReceiver::_ready() {
    // UtilityFunctions::print("ZMQReceiver::_ready()");
}

void ZMQReceiver::_process(double delta) {
    // UtilityFunctions::print("ZMQReceiver::_process()");
}

void ZMQReceiver::_thread_func() {
    // UtilityFunctions::print("ZMQReceiver::_thread_func()");

    while (_isRunning) {
        zmq::message_t message;
        auto result = socket.recv(message, zmq::recv_flags::none);

        if (!result) {
            continue;
        }

        PackedByteArray packet;
        packet.resize(message.size());
        memcpy(packet.ptrw(), message.data(), message.size());

        _process_packet(packet);
    }
}

void ZMQReceiver::_process_packet(PackedByteArray packet) {
    // UtilityFunctions::print("ZMQReceiver::_process_packet()");

    UtilityFunctions::print("ZMQReceiver::_process_packet(), but not implemented");

    mutex->lock();
    Array args;
    // TODO: implement
    // args.push_back(packet);
    if (messageHandler.is_valid()) {
        messageHandler.call(args);
    }
    mutex->unlock();
}

void ZMQReceiver::stop() {
    // UtilityFunctions::print("ZMQReceiver::stop()");

    _isRunning = false;

    thread->wait_to_finish();

    socket.close();
    context.close();
}

void ZMQReceiver::onMessage(Callable callback) {
    // UtilityFunctions::print("ZMQReceiver::onMessage()");

    mutex->lock();
    messageHandler = callback;
    mutex->unlock();
}