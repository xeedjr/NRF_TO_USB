import threading
import time
from nrf24l01 import nrf24l01
import paho.mqtt.client as mqtt
import paho.mqtt.subscribe as subscribe
from struct import *

client = mqtt.Client()

def spin_loop(nrf):
    while True:
        nrf.process_in()

def event(pipe, data):
    print('Event pipe ' + str(pipe) + ' : ' + str(data))
    if pipe == 0: 
        ## from radiator
        cmd, setedT, currT= unpack("<Bff", data)
        client.publish("/radiators/radiator1/state", "{:2.1f}".format(currT))
        client.publish("/radiators/radiator1/settemp", "{:2.1f}".format(setedT))

nrf = nrf24l01('COM10', event)

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    #client.subscribe("$SYS/#")
    client.subscribe("/radiators/radiator1/set")
    client.subscribe("/radiators/radiator1/settemp/set")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    if (msg.topic == "/radiators/radiator1/settemp/set"):
        data = pack("<Bf", 1, float(msg.payload))
        nrf.send_pipe_data(0, data);


def mqtt_loop(client):
    # Blocking call that processes network traffic, dispatches callbacks and
    # handles reconnecting.
    # Other loop*() functions are available that give a threaded interface and a
    # manual interface.
    while True:
        client.loop_forever()


client.on_connect = on_connect
client.on_message = on_message
client.connect("192.168.56.50", 1883, 60)

x = threading.Thread(target=mqtt_loop, args=(client,))
x.start()
x = threading.Thread(target=spin_loop, args=(nrf,))
x.start()

#nrf.test()


while True:
    time.sleep(10)
    #nrf.send_pipe_data(0, [1, 21]);