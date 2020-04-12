import matplotlib.pyplot as plt
import numpy as np
import serial
import time

iter = 100

t = np.arange(0,iter/10,0.1) # time vector; create Fs samples between 0 and 1.0 sec.
data = np.zeros(iter*4)

serdev = '/dev/ttyACM0'
s = serial.Serial(serdev,115200)

print('please press the button')

for i in range(0, iter):
    for j in range(0,4):
        line=s.readline() # Read an echo string from K66F terminated with '\n'
        data[i+j*iter] = float(line)

data = data.reshape(4,iter)
print('plotting...')
fig, ax = plt.subplots(2, 1)

ax[0].plot(t,data[0,:],label = "x")
ax[0].plot(t,data[1,:],label = "y")
ax[0].plot(t,data[2,:],label = "z")
ax[0].legend(loc='upper left')

ax[0].set_xlabel('Time')
ax[0].set_ylabel('Acc Vector')

ax[1].stem(t, data[3,:]) # plotting the spectrum
ax[1].set_xlabel('Time')
ax[1].set_ylabel('Tilt')

plt.show()
s.close()
