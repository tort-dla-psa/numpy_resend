import numpy as np 
import socket
import sys
import os
import gc
from abc import ABC, abstractmethod

class gate(ABC):
    def __init__(self):
        pass

    @abstractmethod
    def send(self, data:np.ndarray):
        yield None
    
    @abstractmethod
    def get(self) -> np.ndarray:
        yield None

class ndarray_resender(gate):
    def __init__(self):
        super().__init__()

    __sock_path = '/tmp/sock'

    def host(self):
        os.unlink(self.__sock_path)
        self.__sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.__sock.bind(self.__sock_path)
        self.__sock.listen(1)
        self.__cli, self.__cli_port = self.__sock.accept()

    def send(self, data: np.ndarray):
        lenstr = str(len(data)).zfill(4);
        self.__cli.sendall(lenstr.encode('utf8'))
        print(data.tobytes())
        self.__cli.sendall(data.tobytes())

    def get(self) -> np.ndarray:
        size = self.__cli.recv(4);
        data = self.__cli.recv(size);
        return np.frombuffer(data, dtype=np.uint8)

class swig_resend(gate):
    def __init__(self):
        super().__init__()

def test_socket(data: np.ndarray):
    print("starting")
    rsender = ndarray_resender()
    print("hosting")
    rsender.host()
    print("sending")
    rsender.send(data)
    print("getting")
    data = rsender.get()
    print("ending")
    del rsender
    gc.collect()

size = 2**8
data = np.linspace(start=0, stop=size, num=size, dtype=np.uint8)
data = np.array([data])
data = np.repeat(data, size, axis=0)
data = np.reshape(data, [-1,1])
test_socket(data)