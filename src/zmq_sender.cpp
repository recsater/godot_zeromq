#include "zmq_sender.hpp"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZMQSender::_bind_methods() {
    ClassDB::bind_static_method("ZMQSender", D_METHOD("new_from", "address", "socketType", "connectionMode", "socketFilter", "auto_receive_on_sender"), &ZMQSender::new_from);
    ClassDB::bind_static_method("ZMQSender", D_METHOD("socket_type_to_string", "socketType"), &ZMQSender::socket_type_to_string);
    ClassDB::bind_static_method("ZMQSender", D_METHOD("connection_mode_to_string", "connectionMode"), &ZMQSender::connection_mode_to_string);
    ClassDB::bind_method(D_METHOD("init", "address", "socketType", "connectionMode", "socketFilter", "auto_receive_on_sender"), &ZMQSender::init);
    ClassDB::bind_method(D_METHOD("stop"), &ZMQSender::stop);
    ClassDB::bind_method(D_METHOD("onMessageString", "callback"), &ZMQSender::onMessageString);
    ClassDB::bind_method(D_METHOD("onMessageBytes", "callback"), &ZMQSender::onMessageBytes);
    ClassDB::bind_method(D_METHOD("sendString", "message"), &ZMQSender::sendString);
    ClassDB::bind_method(D_METHOD("sendBytes", "message"), &ZMQSender::sendBytes);
    ClassDB::bind_method(D_METHOD("beginReceiveRequest"), &ZMQSender::beginReceiveRequest);
    ClassDB::bind_method(D_METHOD("_thread_func"), &ZMQSender::_thread_func);
}

String ZMQSender::socket_type_to_string(int socketType) {
    // UtilityFunctions::print("ZMQSender::socket_type_to_string");

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
            UtilityFunctions::push_error("ZMQSender::socket_type_to_string() unknown socket type: " + String::num_int64(socketType));
            assert(false);
            return "UNKNOWN";
    }
}

String ZMQSender::connection_mode_to_string(int connectionMode) {
    // UtilityFunctions::print("ZMQSender::connection_mode_to_string");

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
            UtilityFunctions::push_error("ZMQSender::connection_mode_to_string() unknown connection mode: " + String::num_int64(connectionMode));
            assert(false);
            return "UNKNOWN";
    }
}

ZMQSender::ZMQSender()
{
    // UtilityFunctions::print("ZMQSender::constructor");
}

ZMQSender::~ZMQSender()
{
    // UtilityFunctions::print("ZMQSender::destructor");
}

ZMQSender* ZMQSender::new_from(String address, int socketType, int connectionMode, String socketFilter, bool autoStartReceiveThreadAfterSend) {
    // UtilityFunctions::print("ZMQSender::new_from");

    ZMQSender* zmq_sender = memnew(ZMQSender);
    zmq_sender->init(address, socketType, connectionMode, socketFilter, autoStartReceiveThreadAfterSend);

    return zmq_sender;
}

void ZMQSender::init(String address, int socketType, int connectionMode, String socketFilter, bool autoStartReceiveThreadAfterSend) {
    // UtilityFunctions::print("ZMQSender::init");
    // UtilityFunctions::print("ZMQSender::init address: " + address + " socketType: " + String::num_int64(socketType));

    addr = address;
    socket_type = socketType;
    connection_mode = connectionMode;
    socket_filter = socketFilter;
    auto_start_receive_thread_after_send = autoStartReceiveThreadAfterSend;

    context = zmq::context_t(1);
    socket = zmq::socket_t(context, socketType);

    std::string addr = address.utf8().get_data();

    if (connectionMode == 1 /* BIND */ ) {
        socket.bind(addr);
    } else if (connectionMode == 2 /* CONNECT */) {
        socket.connect(addr);
    } else {
        UtilityFunctions::push_error("ZMQSender::init unknown connection mode: " + String::num_int64(connectionMode));
        assert(false);
    }

    if (socketType == 2 /* SUB */) {
        socket.setsockopt(ZMQ_SUBSCRIBE, socketFilter.utf8().get_data(), socketFilter.length());
    }

    if (socketType == 2 /* SUB */) {
        UtilityFunctions::print("ZMQSender initialized (addr: " + address
            + ", type: " + socket_type_to_string(socket_type)
            + ", mode: " + connection_mode_to_string(connection_mode)
            + ", filter: '" + socket_filter + "')");
    }else {
        UtilityFunctions::print("ZMQSender initialized (addr: " + address
            + ", type: " + socket_type_to_string(socket_type)
            + ", mode: " + connection_mode_to_string(connection_mode) + ")");
    }

    // UtilityFunctions::print("ZMQSender::init socket connected to: " + address);

    thread = memnew(Thread);
    mutex = memnew(Mutex);
}

void ZMQSender::_ready() {
    // UtilityFunctions::print("ZMQSender::_ready()");
}

void ZMQSender::_process(double delta) {
    // UtilityFunctions::print("ZMQSender::_process()");
}

void ZMQSender::_thread_func() {
    // UtilityFunctions::print("ZMQSender::_thread_func()");

    while (isThreadRunning) {
        if (need_to_start_receiving) {
            need_to_start_receiving = false;
            
            Array message_parts;
            
            while(true) {
                zmq::message_t message_part;
                auto result = socket.recv(message_part, zmq::recv_flags::none);

                if (!result) {
                    if (message_parts.is_empty()) {
                        goto next_message_loop_sender;
                    }
                    UtilityFunctions::push_warning("Incomplete multipart message received on sender.");
                    break;
                }

                if (receive_with_bytes) {
                    if (bytesMessageHandler.is_valid()) {
                        PackedByteArray bytes;
                        bytes.resize(message_part.size());
                        memcpy(bytes.ptrw(), message_part.data(), message_part.size());
                        message_parts.push_back(bytes);
                    }
                } else {
                    if (stringMessageHandler.is_valid()) {
                        std::string _str = message_part.to_string();
                        String str = _str.c_str();
                        message_parts.push_back(str);
                    }
                }

                if (!message_part.more()) {
                    break;
                }
            }

            if (!message_parts.is_empty()) {
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
        next_message_loop_sender:;
        }
    }
}

void ZMQSender::stop() {
    // UtilityFunctions::print("ZMQSender::stop()");

    if( isThreadRunning ) {
        isThreadRunning = false;
        thread->wait_to_finish();
    }

    socket.close();
    context.close();
}

void ZMQSender::onMessageString(Callable callback) {
    // UtilityFunctions::print("ZMQSender::onMessageString()");

    mutex->lock();
    receive_with_bytes = false;
    stringMessageHandler = callback;
    mutex->unlock();
}

void ZMQSender::onMessageBytes(Callable callback) {
    // UtilityFunctions::print("ZMQSender::onMessageBytes()");

    mutex->lock();
    receive_with_bytes = true;
    bytesMessageHandler = callback;
    mutex->unlock();
}

void ZMQSender::sendString(String message) {
    PackedByteArray bytes = message.to_utf8_buffer();
    sendBytes(bytes);
}

void ZMQSender::sendBytes(PackedByteArray message) {
    // zmq::message_t msg(message.size());
    // memcpy(msg.data(), message.ptr(), message.size());
    zmq::message_t msg(message.ptr(), message.size());
    socket.send(msg, zmq::send_flags::none);

    if (auto_start_receive_thread_after_send) {
        beginReceiveRequest();
    }
}

void ZMQSender::beginReceiveThread() {
    if (isThreadRunning) {
        thread->wait_to_finish();
    }

    if (thread) {
        memfree(thread); // Free previous thread instance
    }

    thread = memnew(Thread);
    isThreadRunning = true;
    auto callable = Callable(this, "_thread_func");
    thread->start(callable);
}

void ZMQSender::beginReceiveRequest() {
    need_to_start_receiving = true;

    if(!isThreadRunning) {
        beginReceiveThread();
    }
}
