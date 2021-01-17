import pandas as pd
from arrays import read_organism
import common
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import matplotlib.lines as lines
import numpy as np


ITER = 0;

def display(array):
	# Create figure and axes
	plt.imshow([[common.colors[i] for i in j] for j in array])
	plt.axis('off')
	plt.savefig('display/%d.png'%ITER, dpi=400, bbox_inches='tight', pad_inches=0.1)
	plt.clf()

with open('output.txt', 'r') as f:
	data = f.readlines()
print(len(data))
n = 49
j = 0
field = []
iod = 0
for i in data:
	#print(iod, ITER)
	if len(i) == 50:
		field.append(i[:-1])
		j += 1;
	else:
		j = 0
	if len(field) == n:
		print(ITER)
		display(field)
		field = []
		ITER += 1
		j = 0
	iod += 1
