import libmav
# import unicode

message_set = libmav.MessageSet('/home/marcin/ws/slowmode-app/services/slowmode-app/libs/mavlink/message_definitions/v1.0/common.xml')

message = message_set.create('COMMAND_LONG')
message['target_system'] = 1
message["target_component"] = 1
# message["command"] = 24     # MAV_CMD_REQUEST_MESSAGE
message["command"] = 512     # MAV_CMD_REQUEST_MESSAGE
# message["param1"] = 260      # CAMERA_SETTINGS
# message["param1"] = 242     # HOME_POSITION
# message["param1"] = 259     # CAMERA_INFORMATION
message["param1"] = 0
message["param7"] = 2

# conn_physical = libmav.UDPServer(14550)
conn_physical = libmav.TCPClient('10.41.1.1', 5790)
# conn_physical = libmav.Serial('/dev/ttyUSB0', 9600, True)
conn_runtime = libmav.NetworkRuntime(message_set, conn_physical)
connection = conn_runtime.await_connection(2000)
print("Connecting established")

# Check if connection is still alive
if not connection.alive():
    print('Connection lost, waiting for reconnect...')
    connection = conn_runtime.await_connection(2000)

print("Sending message", message)
# Send a message
connection.send(message)

# Receive a message, timeout 1s
received_message = connection.receive("HEARTBEAT", 1000)
# received_message = connection.receive("COMMAND_ACK", 1000)
print(received_message)

# received_message = connection.receive("COMMAND_ACK", 1000)
# received_message = connection.receive("HOME_POSITION", 1000)
# print(str(received_message))
# received_message = connection.receive("COMMAND_ACK", 1000)

# print(received_message["zoomLevel"])
# zoomLevel = float(received_message["zoomLevel"])

# message['param1'] = 259

# connection.send(message)
# received_message = connection.receive("CAMERA_INFORMATION", 1000)
# print(received_message["focal_length"])
# focal_length = float(received_message["focal_length"])

# print("Current focal lenght:", focal_length*zoomLevel)