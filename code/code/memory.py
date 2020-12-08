import numpy as np
from time import time
import common
from random import randint

class Memory:
	def __init__(self, width=common.width, height=common.height):
		self.width = width
		self.height = height
		self.isAllocated = np.zeros((width, height), dtype=np.bool)
		self.data = np.zeros((width, height), dtype=str)
		self.data.fill('.')

		#to speed up update
		self.last_update_time = time()
		self.last_print_time = time()
		self.string_representation = ''
		self.last_terminal_size = (0, 0)
		self.last_corner_location = (0, 0)

	def radiation(self):
		x = randint(0, common.memory_size[0]-1)
		y = randint(0, common.memory_size[1]-1)
		z = randint(0, len(common.instructions)-1)
		self.data[x][y] = list(common.instructions.keys())[z]

	def to_string_lines(self, terminal_size: tuple, corner_location: tuple):
		if (self.last_update_time < self.last_print_time \
			and terminal_size==self.last_terminal_size \
				and corner_location==self.last_corner_location):
			return self.string_representation

		self.last_terminal_size = terminal_size
		self.last_corner_location = corner_location
		chunk = self.data[
			self.last_corner_location[0]\
				: self.last_corner_location[0]+self.last_terminal_size[0],\
			self.last_corner_location[1]\
				: self.last_corner_location[1]+self.last_terminal_size[1]]
		self.string_representation = '\n'.join([''.join(i) for i in chunk])
		return self.string_representation


	def can_allocate_memory(self, chunk_size, address):
		x, y = address
		w, h = chunk_size
		if not (-1 < x < self.width) or\
			not (-1 < y < self.height):
			return False
		return np.sum(self.isAllocated[x:x+w, y:y+h]) == 0


	def write_chunk(self, chunk, position):
		x, y = position
		w, h = chunk.shape
		if not self.can_allocate_memory(chunk.shape, position):
			#raise MemoryError("can't allocate here")
			return False
		self.isAllocated[x:x+w, y:y+h] = 1
		self.data[x:x+w, y:y+h] = chunk
		self.last_update_time = time()
		return True

	def allocate(self, size, position):
		x, y = position
		w, h = size
		if not self.can_allocate_memory(size, position):
			raise MemoryError("can't allocate here")
		self.isAllocated[x:x+w, y:y+h] = 1
		self.last_update_time = time()
		return True


	def erase(self, position, dimensions):
		x, y = position
		w, h = dimensions

		self.isAllocated[x:x+w, y:y+h] = 0
		self.data[x:x+w, y:y+h] = '.'

	def free(self, position, dimensions):
		x, y = position
		w, h = dimensions

		self.isAllocated[x:x+w, y:y+h] = 0

	def shape(self):
		return (self.width, self.height)


	def get_chunk(self, address, size):
		x, y = address
		w, h = size
		return self.data[x:x+w, y:y+h]


	def read(self, address):
		x, y = address
		if not (-1 < x < self.width) or \
			not(-1 < y < self.height):
			return False
		return self.data[x][y]


	def write(self, address, what):
		for i in common.instructions:
			if all(common.instructions[i][0] == what):
				self.data[address[0]][address[1]] = i
				break


memory = Memory()