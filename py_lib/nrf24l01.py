import queue
import serial
from cobs import cobs

class nrf24l01:
    def __init__(self, port, event):
        self.answ_q = queue.Queue();
        self.event_pipe = event
        self.ser = serial.Serial(port)  # open serial port
        print(self.ser.name)         # check which port was really used

    def sendt(self, data):
        encoded =  bytearray([0]) + cobs.encode(bytearray(data)) + bytearray([0])
        self.ser.write(encoded);

    def event(self, packet):
        self.event_pipe(packet[1], packet[2:])

    recv_buffer = bytearray()

    def process_in(self):
        # wait for start
        while True:
            ch = self.ser.read(size=1)
            if (ch == b'\x00'):
                if (len(self.recv_buffer) > 0):
                    # process frame
                    packet = cobs.decode(self.recv_buffer)
                    self.recv_buffer.clear();

                    if (packet[0] == 1):
                        self.answ_q.put(packet);
                    else:
                        self.event(packet)
            else:
                self.recv_buffer.append(int.from_bytes(ch, 'little'))

    def test (self):
        self.sendt([1])
        packet = self.answ_q.get()
        return packet[2]
        
    def send_pipe_data (self, pipe, data):
        req = [2]
        req.append(pipe);
        req += data;
        self.sendt(req)
        packet = self.answ_q.get(block=True, timeout=None)
        print (packet)
        return packet[2]
        
