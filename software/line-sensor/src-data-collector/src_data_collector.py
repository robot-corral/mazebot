import serial

s = serial.Serial(port = 'COM7',
                  baudrate = 230400,
                  bytesize = serial.EIGHTBITS,
                  parity = serial.PARITY_NONE,
                  stopbits = serial.STOPBITS_ONE,
                  rtscts = True,
                  dsrdtr = True)
if s.is_open == False:
    s.open()
request = bytes([0x41, 0x54, 0x02, 0x01, 0xFF, 0x00, 0x00, 0x00])
s.write(request)
response = s.read(7)
