import time
# pySerial
# https://pyserial.readthedocs.io
import serial

s = serial.Serial(port = 'COM7',
                  baudrate = 230400,
                  bytesize = serial.EIGHTBITS,
                  parity = serial.PARITY_NONE,
                  stopbits = serial.STOPBITS_ONE,
                  rtscts = True,
                  dsrdtr = True,
                  timeout = 1)
if s.is_open == False:
    s.open()

print('calibrating...')

request = bytes([0x41, 0x54, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00])
s.write(request)
response = s.read(7)

while True:
    request = bytes([0x41, 0x54, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00])
    s.write(request)
    response = s.read(7)
    if len(response) != 7:
        continue
    if response[2] != 4:
        print(response[2])
    if response[2] == 0:
        break
    time.sleep(0.5)

print('finished calibrating...')

maxX = response[3] + (response[4] << 8) + (response[5] << 16) + (response[6] << 24)
midX = int(maxX / 2)

print('max x = ' + str(maxX))
print('moving to the middle...')

request = bytes([0x41, 0x54, 0x03, 0x02, midX & 0xFF, (midX >> 8) & 0xFF, (midX >> 16) & 0xFF, (midX >> 24) & 0xFF])
s.write(request)
response = s.read(7)

while True:
    request = bytes([0x41, 0x54, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00])
    s.write(request)
    response = s.read(7)
    if len(response) != 7:
        continue
    if response[2] != 4:
        print(response[2])
    if response[2] == 0:
        break
    time.sleep(0.5)

print('done!')
