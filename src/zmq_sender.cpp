#include "zmq_sender.hpp"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void ZMQSender::_bind_methods() {
    ClassDB::bind_static_method("ZMQSender", D_METHOD("new_from", "outAddr", "socketType"), &ZMQSender::new_from);
    ClassDB::bind_method(D_METHOD("init", "outAddr", "socketType"), &ZMQSender::init);
    ClassDB::bind_method(D_METHOD("stop"), &ZMQSender::stop);
    ClassDB::bind_method(D_METHOD("sendBuffer", "buffer"), &ZMQSender::sendBuffer);
    ClassDB::bind_method(D_METHOD("send", "address", "arguments"), &ZMQSender::send);
}

ZMQSender::ZMQSender()
{
    // UtilityFunctions::print("ZMQSender::constructor");
}

ZMQSender::~ZMQSender()
{
    // UtilityFunctions::print("ZMQSender::destructor");
}

ZMQSender* ZMQSender::new_from(String outAddr, int socketType) {
    // UtilityFunctions::print("ZMQSender::new_from");

    ZMQSender* zmq_sender = memnew(ZMQSender);
    zmq_sender->init(outAddr, socketType);

    return zmq_sender;
}

void ZMQSender::init(String outAddr, int socketType) {
    // UtilityFunctions::print("ZMQSender::init");
    // UtilityFunctions::print("ZMQSender::init outAddr: " + outAddr + " socketType: " + String::num_int64(socketType));
    _out_zmq_addr = outAddr;

    context = zmq::context_t(1);
    socket = zmq::socket_t(context, socketType);
    socket.bind(outAddr.utf8().get_data());
}

void ZMQSender::_ready() {
    // UtilityFunctions::print("ZMQSender::_ready()");
}

void ZMQSender::_process(double delta) {
    // UtilityFunctions::print("ZMQSender::_process()");
}

void ZMQSender::sendBuffer(PackedByteArray buffer) {
    UtilityFunctions::print("ZMQSender::sendBuffer, but not implemented");

    // zmq::message_t message(buffer.size());
    // memcpy(message.data(), buffer.read().ptr(), buffer.size());

    // socket.send(message);
}

void ZMQSender::send(String address, Array arguments) {
    UtilityFunctions::print("ZMQSender::send, but not implemented");

    // String message = address + " " + arguments.join(" ");
    // zmq::message_t zmq_message(message.utf8().length());
    // memcpy(zmq_message.data(), message.utf8().get_data(), message.utf8().length());

    // socket.send(zmq_message);
}

void ZMQSender::stop() {
    // UtilityFunctions::print("ZMQSender::stop");

    socket.close();
    context.close();
}