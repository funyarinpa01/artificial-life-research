import pandas as pd
import numpy as np

import matplotlib.pyplot as plt
from math import cos, sin, pi, radians, log

class OrganismData:
	def __init__(self, oid, born, parent, x, y, width, height, body, life, exit_codes):
		self.id = oid
		self.parent = parent
		self.born = born
		self.startx = x
		self.starty = y
		self.width = width
		self.height = height
		self.body = np.array(list(body), dtype=str).reshape(width, height)
		self.life = life
		self.exit_codes = exit_codes
		self.children = []

	def add_child(self, c):
		self.children.append(c)


def get_points(radius, n):
	xs, ys = [], []
	for i in range(0, 360-360//(n+1), 360//(n+1)):
		xs.append(radius*cos(radians(i)))
		ys.append(radius*sin(radians(i)))
	return xs, ys


def organism_tree(organisms):
	'''
	make a tree of organisms
	'''
	coords = {0: (0, 0)}
	plt.plot([0], [0])
	current_generation = [organisms[0]]
	previous_generation = []
	total = 0
	r = 1
	while total < len(organisms):
		previous_generation = current_generation.copy()
		current_generation = []
		for o in previous_generation:
			current_generation += o.children
		print([_.id for _ in previous_generation], [p.id for p in current_generation])
		x, y = get_points(r, len(current_generation))
		for (o, i, j) in zip(current_generation, x, y):
			coords[o.id] = (i, j)
		#print(coords)
		plt.scatter(x, y)
		for op in previous_generation:
			xp, yp = coords[op.id]
			for oc in op.children:
				xc, yc = coords[oc.id]
				plt.plot([xp, xc], [yp, yc], s=0.5)
		#plt.scatter([coords[o.id][0] for o in previous_generation], [coords[o.id][1] for o in previous_generation])
		r += r
		total += len(current_generation)

def get_spiral(radiuses, step=10):
	xs, ys = [], []
	degree = 0
	while radiuses:
		r = radiuses.pop(0)
		xs.append(r*cos(radians(degree)))
		ys.append(r*sin(radians(degree)))
		degree += step
	return xs, ys

from random import random

def spiral(org):
	organisms = list([org[i] for i in org])
	organisms.sort(key=lambda x: x.born)
	xs, ys = get_spiral(list(range(len(organisms))))#[o.born for o in organisms])
	colors = [(random(), random(), random()) for _ in organisms]
	coords = dict()
	for o, x, y in zip(organisms, xs, ys):
		coords[o.id] = (x, y)
	for (i, o) in enumerate(organisms):
		xp, yp = coords[o.id]
		for c in o.children:
			xc, yc = coords[c.id]
			plt.plot([xp, xc], [yp, yc], color=colors[i], linewidth=2)
	plt.scatter(xs, ys, color=colors, s=10)
	plt.axis('off')
	plt.show()

def get_lineage(organisms, organism):
	lineage = [organism]
	h = 0
	w = 0
	while lineage[-1].id != 0:
		h = max(h, lineage[-1].height)
		w = max(w, lineage[-1].width)
		lineage.append(organisms[lineage[-1].parent])
	return lineage, w, h


def origin_story(organisms, organism):
	lineage, w, h = get_lineage(organisms, organism)

	margin = 5
	matrix = np.zeros(((width+margin)*len(lineage) + margin, (height+2*margin)*4), dtype=str)
	matrix.fill(' ')

	boxes = []
	y_start = margin
	x_start = w+margin
	for o in lineage:
		matrix[y_start:y_start+o.width, x_start:x_start+o.height] = o.body
		plt.text(0, y_start, str(o.id), bbox=dict(facecolor='white', alpha=0.5), size=(1/log(1+len(lineage)))*10)
		boxes.append((y_start, margin, y_start+o.width, margin+o.height))
		y_start += w+margin

	#zero_level_differences
	xa1, ya1, xa2, ya2 = boxes[-1]
	y_start = margin
	x_start += w+2*margin
	for i in range(len(lineage)-1):
		xb1, yb1, xb2, yb2 = boxes[i]
		wb = max(xa2-xa1, xb2-xb1)
		hb = max(ya2-ya1, yb2-yb1)
		compared = np.ones((wb, hb))
		for x in range(min(xa2-xa1, xb2-xb1)):
			for y in range(min(ya2-ya1, yb2-yb1)):
				if matrix[xa1+x, ya1+y] == matrix[xb1+x, yb1+y]:
					compared[x, y] = 0
		matrix[y_start:y_start+wb, x_start:x_start+hb] = compared
		y_start += w+margin


	#previous_next_differences
	y_start = margin
	x_start += w+2*margin
	for i in range(len(lineage)-1):
		xa1, ya1, xa2, ya2 = boxes[i]
		xb1, yb1, xb2, yb2 = boxes[i+1]
		wb = max(xa2-xa1, xb2-xb1)
		hb = max(ya2-ya1, yb2-yb1)
		compared = np.ones((wb, hb))
		for x in range(min(xa2-xa1, xb2-xb1)):
			for y in range(min(ya2-ya1, yb2-yb1)):
				if matrix[xa1+x, ya1+y] == matrix[xb1+x, yb1+y]:
					compared[x, y] = 0
		matrix[y_start:y_start+wb, x_start:x_start+hb] = compared
		y_start += w+margin

	symbols = set([j for i in matrix for j in i])
	colors = {i: (random(), random(), random()) for i in symbols}
	plt.imshow([[colors[j] for j in i] for i in matrix])
	plt.axis('off')

if __name__ == '__main__':
	filename = '011orgs.csv'
	df = pd.read_csv(filename)
	organisms = dict()
	for (_, born, oid, parent, x, y, width, height, body, life, exitcodes) in df.values:
		org = OrganismData(oid, born, parent, x, y, width, height, body, life, exitcodes)
		organisms[oid] = org
		organisms[parent].add_child(org)

	for i, o in enumerate(organisms):
		origin_story(organisms, organisms[o])
		plt.savefig(f'imgs/{i}.png', dpi=300)
		plt.clf()