from memory import Memory
from fullorg import FullOrganism
from parasite import Parasite


import numpy as np
from queue import Queue

from arrays import read_organism

from time import time

import common
from random import randint

import matplotlib.pyplot as plt
import matplotlib.patches as patches
import matplotlib.lines as lines

from time import sleep

import pandas as pd

class Game:
	def __init__(self):
		self.memory = Memory(common.width, common.height)
		self.queue = Queue()
		self.organism_type = FullOrganism
		self.iteration = 0

	def update_queue(self):
		self.queue.organisms.sort()
		new_organisms = []
		for o in self.queue.organisms:
			if o.reproduction_cycle() > common.max_iters_without_child or o.errors() > common.max_number_of_errors:
				x, y = o.position()
				w, h = o.size()
				self.memory.free(x, y, w, h)
				print('freed memory')
			else:
				new_organisms.append(o)
		self.queue.organisms = new_organisms

	def radiation(self):
		x = randint(0, common.width-1)
		y = randint(0, common.height-1)
		if Memory.memory_cell_types[self.memory.celltypes[x][y]] == 'allocated':
			z = randint(0, len(self.organism_type.instruction_set)-1)
			try:
				self.memory.data[x][y] = list(common.instructions.keys())[z]
			except Exception as e:
				print(e)

	def display(self):
		plt.imshow([[common.colors[i] for i in j] for j in self.memory.data.transpose()])
		plt.axis('off')
		plt.show()

	def run(self):
		self.iteration = 0

		x, y = 20, 20
		body = read_organism('initial.gen')
		self.memory.write_organism(body, (x, y))
		organism = self.organism_type(0, x, y, len(body), len(body[0]), self)
		body = read_organism('parasite.gen')
		self.memory.write_organism(body, (20, 5))
		organism = Parasite(1, 20, 10, len(body), len(body[0]), self)

		times = []
		start = time()

		for i in range(5000000):
			self.iteration += 1
			qlen = len(self.queue)
			for organism in self.queue:
				organism.cycle()
				sleep(0.001)
			self.update_queue()
			for i in range(len(self.queue)//5):
				self.radiation()
			if len(self.queue) != qlen or (self.iteration % 20 == 0):
				end = time()
				print(end-start)
				self.display()
			if len(self.queue) == 0: #empty queue
				break

		end = time()

Game().run()