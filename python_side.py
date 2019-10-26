import numpy as np 
import socket
import sys
import os
import gc
from abc import ABC, abstractmethod
import struct
#import pytest
import time

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
		int_len = struct.pack('I',len(data))
		self.__cli.sendall(int_len)
		self.__cli.sendall(data.tobytes())

	def get(self) -> np.ndarray:
		size_str = self.__cli.recv(4);
		size = struct.unpack('I', size_str)[0]
		data = self.__cli.recv(size);
		return np.frombuffer(data, dtype=np.uint8)

	def __del__(self):
		self.__cli.close()
		self.__sock.close()

class swig_resend(gate):
	def __init__(self):
		super().__init__()

#start_test=2**10
#end_test=2**16
#test_data = np.linspace(start=start_test, stop=end_test, num=end_test-start_test, dtype=np.uint32)
#@pytest.mark.parametrize("size",test_data)
def test_case(size):
	data = np.linspace(start=0, stop=size, num=size+1, dtype=np.uint8)
	data = np.array([data])
	data = np.repeat(data, size, axis=0)
	data = np.reshape(data, [1,-1])
	data = data[0]
	rsender = ndarray_resender()
	rsender.host()
	rsender.send(data)
	newdata = rsender.get()
	del rsender
	gc.collect()
	assert (data == newdata).all()

for i in range(0, int(sys.argv[1])):
	start = time.time()
	test_case(i)
	end = time.time()
	print("sent and recieved "+str((i+1)*i)+" bytes in "+str(end-start))