#include "zeromq.hpp"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/packet_peer_udp.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <memory>

using namespace godot;

void ZMQ::_bind_methods() {
    // ClassDB::bind_method(D_METHOD("_init", "inPort", "outPort", "outIP"), &ZMQ::_init);
    // ClassDB::bind_static_method("ZMQ", D_METHOD("create", "inPort", "outPort", "outIP"), &ZMQ::create);
    ClassDB::bind_static_method("ZMQ", D_METHOD("new_from", "inPort", "outPort", "outIP"), &ZMQ::new_from);
    ClassDB::bind_method(D_METHOD("init", "inPort", "outPort", "outIP"), &ZMQ::init);
    // ClassDB::bind_method(D_METHOD("set_max_pending_connections", "p_max_pending_connections"), &ZMQ::set_max_pending_connections);
    ClassDB::bind_method(D_METHOD("sendBuffer", "buffer"), &ZMQ::sendBuffer);
    ClassDB::bind_method(D_METHOD("send", "address", "arguments"), &ZMQ::send);
    ClassDB::bind_method(D_METHOD("stop"), &ZMQ::stop);
    ClassDB::bind_method(D_METHOD("onMessage", "address", "callback"), &ZMQ::onMessage);
}

ZMQ::ZMQ()
{
    // UtilityFunctions::print("ZMQ::constructor");
}

ZMQ::~ZMQ()
{
}

ZMQ* ZMQ::new_from(int inPort, int outPort, String outIP) {
    // UtilityFunctions::print("ZMQ::new_from");

    ZMQ* osc = memnew(ZMQ);
    osc->init(inPort, outPort, outIP);

    return osc;
}

void ZMQ::init(int inPort, int outPort, String outIP) {
    // UtilityFunctions::print("ZMQ::init");
    // UtilityFunctions::print("ZMQ::init inPort: " + String::num_int64(inPort) + " outPort: " + String::num_int64(outPort) + " outIP: " + outIP);
    _inPort = inPort;
    _outPort = outPort;
    _outIP = outIP;

    // server = memnew(UDPServer);
    // server->listen(inPort);
    UtilityFunctions::print("ZMQ UDP server listening on port: " + String::num_int64(inPort));
}

// void ZMQ::set_max_pending_connections(int p_max_pending_connections) {
//     // UtilityFunctions::print("ZMQ::set_max_pending_connections");

//     server->set_max_pending_connections(p_max_pending_connections);
// }

void ZMQ::_ready() {
    // UtilityFunctions::print("ZMQ::_ready()");
}

void ZMQ::_process(double delta) {
    // server->poll(); // Important!
    // if (server->is_connection_available()) {
    //     Ref<PacketPeerUDP> peer = server->take_connection();

    //     while(peer->get_available_packet_count() > 0) {
    //         PackedByteArray packet = peer->get_packet();
    //         _process_packet(packet);
    //     }
    // }
}

void ZMQ::_process_packet(PackedByteArray packet) {
    UtilityFunctions::print("[debug] ZMQ packet received, but unimplemented");
}

void ZMQ::sendBuffer(PackedByteArray buffer) {
    // UtilityFunctions::print("ZMQ::sendBuffer");

    Ref<PacketPeerUDP> udp = memnew(PacketPeerUDP);
    udp->connect_to_host(_outIP, _outPort);
    udp->put_packet(buffer);
    // UtilityFunctions::print("ZMQ UDP packet sent to " + _outIP + ":" + String::num_int64(_outPort));
}

void ZMQ::send(String address, Array arguments) {
    UtilityFunctions::print("ZMQ::send, but unimplemented");
}

void ZMQ::stop() {
    // server->stop();
    UtilityFunctions::print("ZMQ receiver stopped");
}

void ZMQ::onMessage(String address, Callable callback) {
    // UtilityFunctions::print("ZMQ::onMessage");

    if (!messageHandlers.has(address)) {
        messageHandlers[address] = Array();
    }
    ((Array)messageHandlers[address]).push_back(callback);

    // UtilityFunctions::print("ZMQ::onMessage: callback (hash " + String::num(callback.hash()) + ") added to address: " + address);
}