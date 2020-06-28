import paho.mqtt.client as paho
import time
import matplotlib.pyplot as plt
import numpy as np
import serial
mqttc = paho.Client()

# XBee setting
serdev = '/dev/ttyUSB0'
s = serial.Serial(serdev, 9600)

s.write("+++".encode())
char = s.read(2)
s.write("ATMY <0x140>\r\n".encode())
char = s.read(3)
s.write("ATDL <0x240>\r\n".encode())
char = s.read(3)
s.write("ATID <0x87>\r\n".encode())
char = s.read(3)
s.write("ATWR\r\n".encode())
char = s.read(3)
s.write("ATMY\r\n".encode())
char = s.read(4)
s.write("ATDL\r\n".encode())
char = s.read(4)
s.write("ATCN\r\n".encode())
char = s.read(3)
print("start")

data = []
line = s.readline()
end = "end\r\n".encode()

while line != end:
    print(line)
    line=s.readline()
    data.append(line)
print(line)
# Settings for connection
host = "localhost"
topic= "Mbed"
port = 1883

# Settings for connection
def on_connect(self, mosq, obj, rc):
    print("Connected rc: " + str(rc))

def on_message(mosq, obj, msg):
    print("[Received] Topic: " + msg.topic + ", Message: " + str(msg.payload) + "\n")

def on_subscribe(mosq, obj, mid, granted_qos):
    print("Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
    print("Unsubscribed OK")

# Set callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

# Connect
mqttc.connect(host, port=1883, keepalive=60)

f=open('motion.txt','w')
for ele in data:
    f.write(ele.decode('UTF-8','strict'))
f.close()

for i in range(len(data)):
    msg = data[i]
    mqttc.publish(topic, msg)
    print('output',msg)
    time.sleep(1)
