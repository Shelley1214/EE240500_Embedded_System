import serial
import time

# XBee setting
serdev = '/dev/ttyUSB0'
s = serial.Serial(serdev, 9600)

prtdev = '/dev/ttyACM0'
t = serial.Serial(prtdev, 9600)

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

while True:
    s.write("/getAcc/run\r".encode())
    line=t.readline() # Read an echo string from K66F terminated with '\n' (pc.putc())
    print(line)
    time.sleep(1)    
s.close()
