class_name ZeroMQReceiver
extends Node

@export var zmq_in_address:String = "tcp://localhost:5555"
@export var zmq_out_address:String = "tcp://localhost:5556"
@export var zmq_in_socket_type:ZMQ.SocketType = ZMQ.SocketType.REP
@export var zmq_out_socket_type:ZMQ.SocketType = ZMQ.SocketType.REQ
@onready var zmq_receiver:ZMQReceiver = ZMQReceiver.new_from(zmq_in_address, zmq_in_socket_type)

# Called when the node enters the scene tree for the first time.
func _ready():
	print("ready!!")
	# osc.init(osc_in_port, osc_out_port, host_ip) # inPort, outPort, outIP

	add_child(zmq_receiver)

	# Message input Handler 
	# osc.onMessage("/test", func(msg:OSCMessage):
	zmq_receiver.onMessage(func(args: Array):
		print("[ZMQ] Args: ", args)
	)

	# Message output
	# osc.send("/address", [123])

func _exit_tree():
	zmq_receiver.stop()
	remove_child(zmq_receiver)


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass
