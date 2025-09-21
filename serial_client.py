import threading
import queue
import serial 
from serial import SerialException

class SerialClient:
    def __init__(self):
        self.serial_port = None
        self.rx_queue = queue.Queue()
        self.stop_event = threading.Event()
        self.thread = None
        self.newline = "\r\n"

    def open(self, port: str, baud: int = 9600, newline: str = "\r\n"):
        self.close() # Close existing connection if any
        self.newline = newline
        self.serial_port = serial.Serial(port, baud, timeout=0.1)
        self.stop_event.clear()
        self.thread = threading.Thread(target=self._reader, daemon=True)
        self.thread.start()

    def is_open(self) -> bool:
        return self.serial_port is not None and self.serial_port.is_open

    def close(self):
        self.stop_event.set()
        if self.thread and self.thread.is_alive():
            self.thread.join(timeout = 0.5)
        self.thread = None
        if self.serial_port:
            try:
                self.serial_port.close()
            except SerialException:
                pass
        self.serial_port = None


    def write_line(self, line: str):
        if self.is_open():
            data = (line + self.newline).encode(errors='ignore')
            self.serial_port.write(data)

    def _reader(self):
        buffer = bytearray()
        while not self.stop_event.is_set():
            try:
                bytes_read = self.serial_port.read(256)
                if not bytes_read:
                    continue
                buffer.extend(bytes_read) # Append new data to buffer
                # split by newline, keep \r\n
                while True:
                    index = buffer.find(b'\n') # Find the end of line
                    if index == -1:
                        break
                    line = buffer[:index+1].decode(errors='ignore')
                    self.rx_queue.put(line)
                    del buffer[:index+1] # Remove processed line from buffer
            except Exception:
                # stop on error
                break

    def read_line(self):
        lines = []
        while True:
            try:
                line = self.rx_queue.get_nowait()
                lines.append(line)
            except queue.Empty:
                break
        return lines