import numpy as np
import serial
import time
waitTime = 0.1

# generate the waveform table
signalLength = 50*3
signalTable = np.array([  
  261, 261, 392, 392, 440, 440, 392,
  349, 349, 330, 330, 294, 294, 261,
  392, 392, 349, 349, 330, 330, 294,
  392, 392, 349, 349, 330, 330, 294,
  261, 261, 392, 392, 440, 440, 392,
  349, 349, 330, 330, 294, 294, 261,0,0,0,0,0,0,0,0,

  392, 330, 330, 349, 294, 294, 261, 294, 330, 349, 392, 392, 392,
  392, 330, 330, 349, 294, 294, 261, 330, 392, 392, 330,
  294, 294, 294, 294, 294, 330, 349,
  330, 330, 330, 330, 330, 349, 392,
  392, 330, 330, 349, 294, 294, 261, 330, 392, 392, 261,0,

  392, 392, 330, 261, 392, 392, 330, 261,
  294, 330, 349, 349, 330, 349, 392, 392,
  392, 330, 392, 330, 294, 330, 261,
  349, 294, 294, 294, 330, 261, 261, 261,
  294, 330, 349, 294, 261, 247, 261,0,0,0,0,0,0,0,0,0,0,0,0])


# output formatter
formatter = lambda x: "%.3f" % x

# send the waveform table to K66F
serdev = '/dev/ttyACM0'
s = serial.Serial(serdev)
print("Sending signal ...")
print("It may take about %d seconds ..." % (int(signalLength * waitTime)))

for data in signalTable:
  s.write(bytes(formatter(data/1000), 'UTF-8'))
  time.sleep(waitTime)

s.close()
print("Signal sended")
