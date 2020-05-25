import paho.mqtt.client as paho
import matplotlib.pyplot as plt
import numpy as np
import time

# Settings for connection
mqttc = paho.Client()
host = "192.168.43.210"
topic = "Mbed"
_times = 40
data = []

# Callbacks
def on_connect(self, mosq, obj, rc):
      print("Connected rc: " + str(rc))

def on_message(mosq, obj, msg):
      global data
      tmp = msg.payload.decode(encoding='utf-8')
      tmp = tmp.split()
      num = [ float(tmp[0]), float(tmp[1]), float(tmp[2])]
      print(num)
      data.append(num)

def on_subscribe(mosq, obj, mid, granted_qos):
      print("Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
      print("Unsubscribed OK")

# Set callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

# Connect and subscribe
print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=60)
mqttc.subscribe(topic, 0)

# Loop forever, receiving messages
mqttc.loop_start()

formmer = 0
x = []
for i in range (0,_times):
      x.append(len(data)-formmer)
      formmer = len(data)
      time.sleep(0.5)

mqttc.loop_stop()
x.append(len(data)-formmer)

tmp = []
for i in range (1,_times+1):
      for j in range(1, x[i]+1):
            tmp.append((i-1)*0.5 + 0.5/x[i]*j)

tmp = np.array(tmp)
data = np.array(data)
plt.plot(tmp,data[:,0],label = "x")
plt.plot(tmp,data[:,1],label = "y")
plt.plot(tmp,data[:,2],label = "z")
plt.show()