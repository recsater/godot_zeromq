#include "zmq_receiver.hpp"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZMQReceiver::_bind_methods() {
    ClassDB::bind_static_method("ZMQReceiver", D_METHOD("new_from", "inAddr", "socketType"), &ZMQReceiver::new_from);
    ClassDB::bind_method(D_METHOD("init", "inAddr", "socketType"), &ZMQReceiver::init);
    ClassDB::bind_method(D_METHOD("stop"), &ZMQReceiver::stop);
    ClassDB::bind_method(D_METHOD("onMessage", "callback"), &ZMQReceiver::onMessage);
    ClassDB::bind_method(D_METHOD("_thread_func"), &ZMQReceiver::_thread_func);
}

String ZMQReceiver::socket_type_to_string(int socketType) {
    // UtilityFunctions::print("ZMQReceiver::socket_type_to_string");

    // enum SocketType {
    //     PUB = 1,
    //     SUB = 2,
    //     REQ = 3,
    //     REP = 4,
    //     DEALER = 5,
    //     ROUTER = 6,
    //     PULL = 7,
    //     PUSH = 8,
    //     XPUB = 9,
    //     XSUB = 10,
    //     STREAM = 11
    // }

    switch (socketType) {
        case 1: // PUB
            return "PUB";
        case 2: // SUB
            return "SUB";
        case 3: // REQ
            return "REQ";
        case 4: // REP
            return "REP";
        case 5: // DEALER
            return "DEALER";
        case 6: // ROUTER
            return "ROUTER";
        case 7: // PULL
            return "PULL";
        case 8: // PUSH
            return "PUSH";
        case 9: // XPUB
            return "XPUB";
        case 10: // XSUB
            return "XSUB";
        case 11: // STREAM
            return "STREAM";
        default:
            UtilityFunctions::push_error("ZMQReceiver::socket_type_to_string() unknown socket type: " + String::num_int64(socketType));
            assert(false);
            return "UNKNOWN";
    }
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
    _in_socket_type = socketType;

    context = zmq::context_t(1);
    socket = zmq::socket_t(context, socketType);

    std::string addr = inAddr.utf8().get_data();
    socket.bind(addr);

    // UtilityFunctions::print("ZMQReceiver::init socket connected to: " + inAddr);

    thread = memnew(Thread);
    mutex = memnew(Mutex);

    auto callable = Callable(this, "_thread_func");
    thread->start(callable);
}

void ZMQReceiver::_ready() {
    // UtilityFunctions::print("ZMQReceiver::_ready()");
}

void ZMQReceiver::_process(double delta) {
    // UtilityFunctions::print("ZMQReceiver::_process()");
}

void ZMQReceiver::_thread_func() {
    // UtilityFunctions::print("ZMQReceiver::_thread_func()");

    UtilityFunctions::print("ZMQReceiver started on address: " + _in_zmq_addr + " with socket type: " + socket_type_to_string(_in_socket_type));

    while (_isRunning) {
        zmq::message_t message;
        auto result = socket.recv(message, zmq::recv_flags::none);

        if (!result) {
            continue;
        }

        UtilityFunctions::print("ZMQReceiver::_thread_func() received message");

        // PackedByteArray packet;
        // packet.resize(message.size());
        // memcpy(packet.ptrw(), message.data(), message.size());

        // _process_packet(packet);
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