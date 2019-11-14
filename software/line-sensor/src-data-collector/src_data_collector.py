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
b = s.read(1)
b = s.read(1)
