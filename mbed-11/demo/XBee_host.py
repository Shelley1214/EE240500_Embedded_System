import serial
import time

# XBee setting
serdev = '/dev/ttyUSB0'
s = serial.Serial(serdev, 9600)

s.write("+++".encode())
s.write("ATMY <0x140>\r\n".encode())
s.write("ATDL <0x240>\r\n".encode())
s.write("ATID <0x1>\r\n".encode())
s.write("ATWR\r\n".encode())
s.write("ATMY\r\n".encode())
s.write("ATDL\r\n".encode())
s.write("ATCN\r\n".encode())

print("start sending RPC")

while True:
    s.write("/getAcc/run\r".encode())
    time.sleep(1)    
s.close()
