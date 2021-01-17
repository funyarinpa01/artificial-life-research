import numpy as np
import common

log = open('memorylog.txt', 'w')

class Memory:
	memory_cell_types = {
		0: "free",
		1: "allocated",
		2: "active",
		'free': 0,
		'allocated': 1,
		'active': 2,
	}

	def __init__(self, w, h):
		self.width = w
		self.height = h
		self.celltypes = np.zeros((w, h), dtype=np.bool)
		self.data = np.zeros((w, h), dtype=str)
		self.data.fill('.')

	def can_fit(self, x, y, w, h):
		return (-1 < x < self.width) and (-1 < y < self.height) and \
			(-1 < x+w < self.width) and (-1 < y+h < self.height)

	def get_width(self):
		return self.width

	def get_height(self):
		return self.height

	def activate(self, x, y, w, h):
		if not self.can_fit(x, y, w, h):
			raise MemoryError(f"can't fit {(w, h)} at {(x, y)}")
		if np.sum(self.celltypes[x:x+w, y:y+h] == 2) == w*h:
			raise MemoryError(f"can't activate: memory chunk (size: {(w, h)}, address: {(x, y)} already active")
		self.celltypes[x:x+w, y:y+h] = 2
		print(f"MA {x} {y} {w} {h}", end='|', file=log)

	def allocate(self, x, y, w, h):
		if not self.can_fit(x, y, w, h):
			print("can't fit")
			raise MemoryError(f"can't fit {(w, h)} at {(x, y)}")
		if not (np.sum(self.celltypes[x:x+w, y:y+h] == 0) == w*h):
			print("nnot free")
			raise MemoryError(f"can't allocate: memory chunk (size: {(x, y)}, address: {(w, h)}) is not free")
		self.celltypes[x:x+w, y:y+h] = 1
		print(f"ML {x} {y} {w} {h}", end='|', file=log)

	def free(self, x, y, w, h):
		if not self.can_fit(x, y, w, h):
			raise MemoryError(f"can't fit {(w, h)} at {(x, y)}")
		self.celltypes[x:x+w, y:y+h] = 0
		print(f"MF {x} {y} {w} {h}", end='|', file=log)

	def shape(self):
		return (self.width, self.height)

	def get_command(self, x, y):
		if not (-1 < x < self.width) or \
			not(-1 < y < self.height):
			raise ValueError(f'{(x, y)} unreacheble')
		return self.data[x][y]

	def set_command(self, x, y, symbol):
		if not (-1 < x < self.width) or \
			not(-1 < y < self.height):
			raise ValueError(f'{(x, y)} unreacheble')
		self.data[x][y] = symbol
		print(f"MW {x} {y} {symbol}", end='|', file=log)

	def write_organism(self, body: np.array, address: tuple):
		x, y = address
		w, h = body.shape
		if not self.can_fit(x, y, w, h):
			return False
		for i in range(body.shape[0]):
			for j in range(body.shape[1]):
				self.set_command(x+i, y+j, body[i][j])
		self.activate(x, y, w, h)

	def is_free(self, x, y, w, h):
		if not self.can_fit(x, y, w, h):
			return False
		return np.sum(self.celltypes[x:x+w, y:y+h] == 0) == w*h

	def is_active(self, x, y, w, h):
		if not self.can_fit(x, y, w, h):
			return False
		return np.sum(self.celltypes[x:x+w, y:y+h] == 2) == w*h
