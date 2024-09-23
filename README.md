# godot_zeromq

ZeroMQ addon for Godot 4.2.2 - 4.3

## Usage

```gdscript
@onready var zmq_receiver = ZMQReceiver::new_from("tcp://localhost:5555", ZMQ.SocketType.PULL, ZMQ.ConnectionMode.BIND, "")
@onready var zmq_sender = ZMQSender::new_from("tcp://localhost:5555", ZMQ.SocketType.PUSH, ZMQ.ConnectionMode.CONNECT, "", false)

func _ready():
    add_child(zmq_receiver)
    add_child(zmq_sender)

    zmq_receiver.onMessageString(func(str: String):
        print("[ZMQ Receiver] Received: ", str)
    )

    while true:
        await get_tree().create_timer(1.0).timeout
        print("[ZMQ Sender] Sending: ", "Hello World")

func _exit_tree():
    zmq_receiver.stop()
    zmq_sender.stop()

    remove_child(zmq_receiver)
    remove_child(zmq_sender)
```

### ZMQReceiver

- `new_from(address: String, socket_type: int, connection_mode: int, socket_filter: String) -> ZMQReceiver`
- `onMessageString(callback: Callable[[String], void])`
- `onMessageBytes(callback: Callable[[PackedByteArray], void])`
- `sendString(message: String) -> void`
- `sendBytes(message: PackedByteArray) -> void`
- `stop() -> void`

NOTE: socket_filter is only used when socket_type is SUB

### ZMQSender

- `new_from(address: String, socket_type: int, connection_mode: int, socket_filter: String, auto_receive_on_sender: bool) -> ZMQSender`
- `onMessageString(callback: Callable[[String], void])`
- `onMessageBytes(callback: Callable[[PackedByteArray], void])`
- `sendString(message: String) -> void`
- `sendBytes(message: PackedByteArray) -> void`
- `beginReceiveRequest() -> void` (only enabled to use when auto_receive_on_sender is false)
- `stop() -> void`

NOTE: socket_filter is only used when socket_type is SUB

## Installation

TODO: Add installation instructions

## Example Project

see [`project/`](project) directory

## Build and Run

(This process is needed only if you build this plugin by your own)

```bash
$ git submodule update --init --recursive --recommend-shallow --depth 1
$ scons
$ scons target=template_release
$ godot project/project.godot # (only first time)
$ godot --path project/ # run demo
```