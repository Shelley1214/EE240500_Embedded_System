import serial
import time
import numpy as np
import matplotlib.pyplot as plt

# XBee setting
serdev = '/dev/ttyUSB0'
s = serial.Serial(serdev, 9600)
Num = []
_times = 20

s.write("+++".encode())
char = s.read(2)
s.write("ATMY <0x140>\r\n".encode())
char = s.read(3)
s.write("ATDL <0x240>\r\n".encode())
char = s.read(3)
s.write("ATID <0x1>\r\n".encode())
char = s.read(3)
s.write("ATWR\r\n".encode())
char = s.read(3)
s.write("ATMY\r\n".encode())
char = s.read(4)
s.write("ATDL\r\n".encode())
char = s.read(4)
s.write("ATCN\r\n".encode())
char = s.read(3)
print("start sending RPC")

for i in range(0,_times):
    s.write("/getNum/run\r".encode())
    tmp = int(s.readline())
    print(tmp)
    Num.append(tmp)
    time.sleep(1)
s.close()

x = np.array(range(1,len(Num)+1))
plt.plot(x,Num)
plt.show()