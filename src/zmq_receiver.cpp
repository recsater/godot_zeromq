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

    // choose bind or connect
    // With Bind, you allow peers to connect to you, thus you don’t know how many peers there will be in the future and you cannot create the queues in advance. Instead, queues are created as individual peers connect to the bound socket.
    // With Connect, ZeroMQ knows that there’s going to be at least a single peer and thus it can create a single queue immediately. This applies to all socket types except ROUTER, where queues are only created after the peer we connect to has acknowledge our connection.
    
    //     ZMQ bind versus connect
    // In brief, you should use bind for:

    // stable things; use connect for volatile things
    // when there is on; use connect when the number is unknown
    // when listening; use connect when broadcasting
    // long-lived process should bind; short-lived should connect
    // bind for incoming; connect for outgoing
    // bound sockets start muted; connected sockets start ready [except router sockets]
    
    if (socketType == 2 /* SUB */
        || socketType == 4 /* REP */
        || socketType == 6 /* ROUTER */
        || socketType == 8 /* PUSH */
        || socketType == 10 /* XSUB */) {
        socket.connect(addr);
    } else {
        socket.bind(addr);
    }

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

        // UtilityFunctions::print("ZMQReceiver::_thread_func() received message");
        std::string _str = message.to_string();
        String str = _str.c_str();
        // UtilityFunctions::print("ZMQReceiver::_thread_func() received message: " + str);

        PackedByteArray packet;
        packet.resize(message.size());
        memcpy(packet.ptrw(), message.data(), message.size());

        _process_packet(packet, str);
    }
}

void ZMQReceiver::_process_packet(PackedByteArray bytes, String str) {
    // UtilityFunctions::print("ZMQReceiver::_process_packet()");

    mutex->lock();
    if (messageHandler.is_valid()) {
        messageHandler.call(bytes, str);
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