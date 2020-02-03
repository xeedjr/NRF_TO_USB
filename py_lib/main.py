import serial

def bytesTostr(data):
    res = ''
    for x in data:
        res = res + '{:02X}'.format(x)
    return res

def strTobytes(hex_string):
    return bytes.fromhex(hex_string);

def sendt(data):
    ser.write(str.encode('\n' + bytesTostr(data)));

def test ():
    ser.write(str.encode('\n' + bytesTostr([1, 1])));
    ser.read_until();
    slen = ser.read(size=2);
    lens = strTobytes(str(slen, 'ascii'))
    data = ser.read(int.from_bytes(lens, "big") * 2);
    print (data)
    
def send_pipe_data (pipe, data):
    req = [0, 2]
    req.append(pipe);
    req += data;
    req[0] = len(req)-1;
    ser.write(str.encode('\n' + bytesTostr(req)));
    ser.read_until();
    slen = ser.read(size=2);
    lens = strTobytes(str(slen, 'ascii'))
    data = ser.read(int.from_bytes(lens, "big") * 2);
    print (data)


def loop():
    while True :
        data = ser.read()

ser = serial.Serial('COM10')  # open serial port
print(ser.name)         # check which port was really used

test()
send_pipe_data(0, [1, 18]);
#read_pipe_data(0, [1, 18]);

loop()