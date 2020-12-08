from memory import memory
from organism import Organism, organisms
import numpy as np
from time import sleep
from queue import queue
from arrays import read_organism
from time import time
from sys import getsizeof
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import matplotlib.lines as lines
import common

ITER = 0

def display(fieldfilename, organismsfilename):
	organisms = pd.read_csv(organismsfilename)
	field = read_organism(fieldfilename)
	# Create figure and axes
	plt.imshow([[common.colors[i] for i in j] for j in field.transpose()])
	for _, Id, parent, x, y, w, h, e in organisms.values:
		plt.plot([x, x+w, x+w, x, x], [y, y, y+h, y+h, y], linewidth=0.5)
	for _, Id, parent, x, y, w, h, e in organisms.values:
		_, idp, pp, xp, yp, wp, hp, ep = organisms.values[parent]
		plt.plot([x+w/2, xp+wp/2], [y+h/2, yp+hp/2], linewidth=0.5)
	plt.axis('off')
	plt.savefig('display/image%d.png'%ITER, dpi=400)
	plt.clf()


def log():
	global ITER
	global memory
	global queue
	organisms = queue.organisms
	data = {'ID': [o.id for o in organisms],
			'Parent': [o.parent for o in organisms],
			'X': [o.start[0] for o in organisms],
			'Y': [o.start[1] for o in organisms],
			'W': [o.size[0] for o in organisms],
			'H': [o.size[1] for o in organisms],
			'Error': [o.errors for o in organisms]}
	df = pd.DataFrame(data)
	df.to_csv('display/organisms%d.csv'% ITER)
	text = '\n'.join([''.join(list(i)) for i in memory.data])
	with open("display/field%d.txt" % ITER, 'w') as f:
		f.write(text)
	display("display/field%d.txt" % ITER, 'display/organisms%d.csv'% ITER)


address = np.array([46, 34])
body = read_organism('initial.gen')
memory.write_chunk(body, address)
organism = Organism(address, body.shape, 0)


times = []
start = time()
for i in range(200000):
	ITER += 1
	s = time()
	qlen = len(queue)
	for organism in queue:
		#print(organism)
		organism.cycle()
		#print(organism.ip_offset(), end=' ')
	queue.update()
	if i % 5000 == 0:
		log()
		print(i, queue)
		for _ in range(20):
			memory.radiation()

end = time()
print(end-start)