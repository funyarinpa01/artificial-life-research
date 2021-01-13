from memory import Memory
from memory import log as mlog
from commands import FullOrganism
from organism import log as orglog
import numpy as np
from queue import Queue
from arrays import read_organism
from time import time
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import matplotlib.lines as lines
import common
from random import randint


class Game:
	def __init__(self):
		self.memory = Memory(common.width, common.height)
		self.queue = Queue()
		self.organism_class = FullOrganism

	def update_queue(self):
		self.queue.organisms.sort()
		new_organisms = []
		for o in self.queue.organisms:
			if o.dead():
				x, y = o.position()
				w, h = o.size()
				self.memory.free(x, y, w, h)
			else:
				new_organisms.append(o)
		self.queue.organisms = new_organisms

	def radiation(self):
		x = randint(0, common.memory_size[0]-1)
		y = randint(0, common.memory_size[1]-1)
		if Memory.memory_cell_types[self.memory.celltypes[x][y]] == 'allocated':
			z = randint(0, len(self.organism_class.instruction_set)-1)
			self.memory.data[x][y] = list(self.organism_class.instruction_set.keys())[z]
			print(f"MR {x} {y} {z}", end='|', file=mlog)

	def display(self):
		plt.imshow([[common.colors[i] for i in j] for j in self.memory.data.transpose()])
		for i, o in enumerate(self.queue):
			x, y = o.position()
			w, h = o.size()
			plt.plot([x, x+w, x+w, x, x], [y, y, y+h, y+h, y])
			plt.text(x+w//2, y+h//2, str(i), fontsize=10, bbox=dict(facecolor='white', alpha=0.5))
		plt.axis('off')
		plt.show()

	def run(self):
		ITER = 0
		x, y = 100, 100
		body = read_organism('initial.gen')
		self.memory.write_organism(body, (x, y))
		organism = self.organism_class(0, x, y, len(body), len(body[0]), self)


		times = []
		start = time()

		for i in range(5000000):
			ITER += 1
			qlen = len(self.queue)
			for organism in self.queue:
				organism.cycle()
			self.update_queue()

			for i in range(len(self.queue)//5):
				self.radiation()

			if len(self.queue) != qlen or (ITER % 50000 == 0):
				print(ITER)
				self.display()

			if len(self.queue) == 0:
				print('empty queue')
				break

			print('\n', end='', file=orglog)
			print('\n', end='', file=mlog)

		end = time()
		print('time: ', end-start)

Game().run()