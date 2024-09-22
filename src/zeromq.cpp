#include "zeromq.hpp"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/packet_peer_udp.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <memory>

using namespace godot;

void ZeroMQ::_bind_methods() {
    // ClassDB::bind_method(D_METHOD("_init", "inPort", "outPort", "outIP"), &ZeroMQ::_init);
    // ClassDB::bind_static_method("ZeroMQ", D_METHOD("create", "inPort", "outPort", "outIP"), &ZeroMQ::create);
    ClassDB::bind_static_method("ZeroMQ", D_METHOD("new_from", "inPort", "outPort", "outIP"), &ZeroMQ::new_from);
    ClassDB::bind_method(D_METHOD("init", "inPort", "outPort", "outIP"), &ZeroMQ::init);
    ClassDB::bind_method(D_METHOD("set_max_pending_connections", "p_max_pending_connections"), &ZeroMQ::set_max_pending_connections);
    ClassDB::bind_method(D_METHOD("sendBuffer", "buffer"), &ZeroMQ::sendBuffer);
    ClassDB::bind_method(D_METHOD("send", "address", "arguments"), &ZeroMQ::send);
    ClassDB::bind_method(D_METHOD("stop"), &ZeroMQ::stop);
    ClassDB::bind_method(D_METHOD("onMessage", "address", "callback"), &ZeroMQ::onMessage);
}

ZeroMQ::ZeroMQ()
{
    // UtilityFunctions::print("ZeroMQ::constructor");
}

ZeroMQ::~ZeroMQ()
{
}

ZeroMQ* ZeroMQ::new_from(int inPort, int outPort, String outIP) {
    // UtilityFunctions::print("ZeroMQ::new_from");

    ZeroMQ* osc = memnew(ZeroMQ);
    osc->init(inPort, outPort, outIP);

    return osc;
}

void ZeroMQ::init(int inPort, int outPort, String outIP) {
    // UtilityFunctions::print("ZeroMQ::init");
    // UtilityFunctions::print("ZeroMQ::init inPort: " + String::num_int64(inPort) + " outPort: " + String::num_int64(outPort) + " outIP: " + outIP);
    _inPort = inPort;
    _outPort = outPort;
    _outIP = outIP;

    server = memnew(UDPServer);
    server->listen(inPort);
    UtilityFunctions::print("ZeroMQ UDP server listening on port: " + String::num_int64(inPort));
}

void ZeroMQ::set_max_pending_connections(int p_max_pending_connections) {
    // UtilityFunctions::print("ZeroMQ::set_max_pending_connections");

    server->set_max_pending_connections(p_max_pending_connections);
}

void ZeroMQ::_ready() {
    // UtilityFunctions::print("ZeroMQ::_ready()");
}

void ZeroMQ::_process(double delta) {
    server->poll(); // Important!
    if (server->is_connection_available()) {
        Ref<PacketPeerUDP> peer = server->take_connection();

        while(peer->get_available_packet_count() > 0) {
            PackedByteArray packet = peer->get_packet();
            _process_packet(packet);
        }
    }
}

void ZeroMQ::_process_packet(PackedByteArray packet) {
    // UtilityFunctions::print("[debug] ZeroMQ packet received: " + packet.get_string_from_utf8());

    std::vector<std::shared_ptr<ZeroMQMessage>> all_messages;

    std::shared_ptr<ZeroMQMessage> msg = std::make_shared<ZeroMQMessage>();
    TypedArray<PackedByteArray> rest_messages = msg->_parseMessage(packet);
    all_messages.push_back(msg);

    for (int i = 0; i < rest_messages.size(); i++) {
        std::shared_ptr<ZeroMQMessage> msg = std::make_shared<ZeroMQMessage>();
        msg->_parseMessage(rest_messages[i]);
        all_messages.push_back(msg);
    }

    for (int i = 0; i < all_messages.size(); i++) {
        std::shared_ptr<ZeroMQMessage> msg = all_messages[i];

        // UtilityFunctions::print("ZeroMQ message received: " + msg->address());
        // String typeTags = "";
        // for (int j = 0; j < msg->_myTypetag.size(); j++) {
        //     typeTags += String::chr(msg->_myTypetag[j]);
        // }
        // UtilityFunctions::print("ZeroMQ message type tags: " + typeTags);
        // UtilityFunctions::print("ZeroMQ message content: " + msg->toString());
        
        if (!msg->isValid()) {
            // UtilityFunctions::print("[debug] ZeroMQ message is invalid");
            continue;
        }
        if (messageHandlers.has("*")) {
            Array empty = Array();
            Array arr = messageHandlers.get("*", empty);

            for (int i = 0; i < arr.size(); i++) {
                // UtilityFunctions::print("Calling handler in *");
                Callable handler = arr[i];
                // handler.call(msg.get());
                // handler.call("call", msg.get());
                // handler.call();
                handler.call(msg->address(), msg->getValues());
            }
        }
        if (messageHandlers.has(msg->address())) {
            Array arr = messageHandlers[msg->address()];

            for (int i = 0; i < arr.size(); i++) {
                // UtilityFunctions::print("Calling handler (hash " + String::num(handler.hash()) + ") in address: " + msg->address());
                Callable handler = arr[i];
                // handler.call(msg.get());
                // handler.call("call", msg.get());
                // handler.call();
                handler.call(msg->address(), msg->getValues());
            }
        }
    }
}

void ZeroMQ::sendBuffer(PackedByteArray buffer) {
    // UtilityFunctions::print("ZeroMQ::sendBuffer");

    Ref<PacketPeerUDP> udp = memnew(PacketPeerUDP);
    udp->connect_to_host(_outIP, _outPort);
    udp->put_packet(buffer);
    // UtilityFunctions::print("ZeroMQ UDP packet sent to " + _outIP + ":" + String::num_int64(_outPort));
}

void ZeroMQ::send(String address, Array arguments) {
    // UtilityFunctions::print("ZeroMQ::send");

    ZeroMQMessage msg;
    msg.init(address);
    for (int i = 0; i < arguments.size(); i++) {
        msg.add(arguments[i]);
    }
    sendBuffer(msg.toPackedByteArray());
}

void ZeroMQ::stop() {
    server->stop();
    UtilityFunctions::print("ZeroMQ UDP server stopped");
}

void ZeroMQ::onMessage(String address, Callable callback) {
    // UtilityFunctions::print("ZeroMQ::onMessage");

    if (!messageHandlers.has(address)) {
        messageHandlers[address] = Array();
    }
    ((Array)messageHandlers[address]).push_back(callback);

    // UtilityFunctions::print("ZeroMQ::onMessage: callback (hash " + String::num(callback.hash()) + ") added to address: " + address);
}