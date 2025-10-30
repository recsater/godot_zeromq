#include "zmq_receiver.hpp"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZMQReceiver::_bind_methods() {
    ClassDB::bind_static_method("ZMQReceiver", D_METHOD("new_from", "address", "socketType", "connectionMode", "socketFilter"), &ZMQReceiver::new_from);
    ClassDB::bind_static_method("ZMQReceiver", D_METHOD("socket_type_to_string", "socketType"), &ZMQReceiver::socket_type_to_string);
    ClassDB::bind_static_method("ZMQReceiver", D_METHOD("connection_mode_to_string", "connectionMode"), &ZMQReceiver::connection_mode_to_string);
    ClassDB::bind_method(D_METHOD("init", "address", "socketType", "connectionMode", "socketFilter"), &ZMQReceiver::init);
    ClassDB::bind_method(D_METHOD("stop"), &ZMQReceiver::stop);
    ClassDB::bind_method(D_METHOD("onMessageString", "callback"), &ZMQReceiver::onMessageString);
    ClassDB::bind_method(D_METHOD("onMessageBytes", "callback"), &ZMQReceiver::onMessageBytes);
    ClassDB::bind_method(D_METHOD("sendString", "message"), &ZMQReceiver::sendString);
    ClassDB::bind_method(D_METHOD("sendBytes", "message"), &ZMQReceiver::sendBytes);
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

String ZMQReceiver::connection_mode_to_string(int connectionMode) {
    // UtilityFunctions::print("ZMQReceiver::connection_mode_to_string");

    // enum ConnectionMode {
    //     BIND = 1,
    //     CONNECT = 2
    // }

    switch (connectionMode) {
        case 1: // BIND
            return "BIND";
        case 2: // CONNECT
            return "CONNECT";
        default:
            UtilityFunctions::push_error("ZMQReceiver::connection_mode_to_string() unknown connection mode: " + String::num_int64(connectionMode));
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

ZMQReceiver* ZMQReceiver::new_from(String address, int socketType, int connectionMode, String socketFilter) {
    // UtilityFunctions::print("ZMQReceiver::new_from");

    ZMQReceiver* zmq_receiver = memnew(ZMQReceiver);
    zmq_receiver->init(address, socketType, connectionMode, socketFilter);

    return zmq_receiver;
}

void ZMQReceiver::init(String address, int socketType, int connectionMode, String socketFilter) {
    // UtilityFunctions::print("ZMQReceiver::init");
    // UtilityFunctions::print("ZMQReceiver::init address: " + address + " socketType: " + String::num_int64(socketType));

    addr = address;
    socket_type = socketType;
    connection_mode = connectionMode;
    socket_filter = socketFilter;

    context = zmq::context_t(1);
    socket = zmq::socket_t(context, socketType);

    std::string addr = address.utf8().get_data();

    if (connectionMode == 1 /* BIND */ ) {
        socket.bind(addr);
    } else if (connectionMode == 2 /* CONNECT */) {
        socket.connect(addr);
    } else {
        UtilityFunctions::push_error("ZMQReceiver::init unknown connection mode: " + String::num_int64(connectionMode));
        assert(false);
    }

    if (socketType == 2 /* SUB */) {
        socket.setsockopt(ZMQ_SUBSCRIBE, socketFilter.utf8().get_data(), socketFilter.length());
    }

    // UtilityFunctions::print("ZMQReceiver::init socket connected to: " + address);

    thread = memnew(Thread);
    mutex = memnew(Mutex);

    isThreadRunning = true;
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
    if (socket_type == 2 /* SUB */) {
        UtilityFunctions::print("ZMQReceiver started (addr: " + addr
            + ", type: " + socket_type_to_string(socket_type)
            + ", mode: " + connection_mode_to_string(connection_mode)
            + ", filter: '" + socket_filter + "')");
    } else {
        UtilityFunctions::print("ZMQReceiver started (addr: " + addr
            + ", type: " + socket_type_to_string(socket_type)
            + ", mode: " + connection_mode_to_string(connection_mode) + ")");
    }

    while (isThreadRunning) {
        Array message_parts;
        bool received_any_part = false;

        while (true) {
            zmq::message_t message_part;
            auto result = socket.recv(message_part, zmq::recv_flags::none);

            if (!result) {
                break;
            }

            received_any_part = true;

            if (receive_with_bytes) {
                PackedByteArray bytes;
                bytes.resize(message_part.size());
                memcpy(bytes.ptrw(), message_part.data(), message_part.size());
                message_parts.push_back(bytes);
            } else {
                std::string _str = message_part.to_string();
                String str = _str.c_str();
                message_parts.push_back(str);
            }

            if (!message_part.more()) {
                break;
            }
        }

        if (!received_any_part) {
            continue;
        }

        mutex->lock();
        if (receive_with_bytes) {
            if (bytesMessageHandler.is_valid()) {
                bytesMessageHandler.call(message_parts);
            }
        } else {
            if (stringMessageHandler.is_valid()) {
                stringMessageHandler.call(message_parts);
            }
        }
        mutex->unlock();
    }
}

void ZMQReceiver::stop() {
    // UtilityFunctions::print("ZMQReceiver::stop()");

    isThreadRunning = false;

    thread->wait_to_finish();

    socket.close();
    context.close();
}

void ZMQReceiver::onMessageString(Callable callback) {
    // UtilityFunctions::print("ZMQReceiver::onMessageString()");

    mutex->lock();
    receive_with_bytes = false;
    stringMessageHandler = callback;
    mutex->unlock();
}

void ZMQReceiver::onMessageBytes(Callable callback) {
    // UtilityFunctions::print("ZMQReceiver::onMessageBytes()");

    mutex->lock();
    receive_with_bytes = true;
    bytesMessageHandler = callback;
    mutex->unlock();
}

void ZMQReceiver::sendString(String message) {
    PackedByteArray bytes = message.to_utf8_buffer();
    sendBytes(bytes);
}

void ZMQReceiver::sendBytes(PackedByteArray message) {
    // zmq::message_t msg(message.size());
    // memcpy(msg.data(), message.ptr(), message.size());
    zmq::message_t msg(message.ptr(), message.size());
    socket.send(msg, zmq::send_flags::none);
}
