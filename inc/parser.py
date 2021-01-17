prefixes = {
	'ML': 'allocate memory chunk',
	'MA': 'activate memory chunk',
	'MF': 'free memory chunk',
	'MW': 'write command into memory',
	'O': 'organism action',
	'R': 'radiation',
	'Q': 'queue',
}

def memory_block_command_parser(c: str):
	'''
	input: c - command without the prefix

	'''
	numbers = c.split(' ')
	x, y, w, h = (int(i) for i in numbers[:4])
	return x, y, w, h

def allocation_parser(c: str):
	x, y, w, h = memory_block_command_parser(c)
	return {'x': x, 'y': y, 'width': w, 'height': h}

def activation_parser(c: str):
	x, y, w, h = memory_block_command_parser(c)
	return {'x': x, 'y': y, 'width': w, 'height': h}

def freeing_parser(c: str):
	x, y, w, h = memory_block_command_parser(c)
	return {'x': x, 'y': y, 'width': w, 'height': h}

def writing_parser(c: str):
	x, y, s = (c.split(' ')[:3])
	x, y = int(x), int(y)
	return {'x': x, 'y': y, 'symbol': s}

def radiation_parser(c: str):
	x, y, s = (c.split(' ')[:3])
	x, y = int(x), int(y)
	return {'x': x, 'y': y, 'symbol': s}

def queue_parser(c: str):
	oid, parentid, startx, starty, width, height = (c.split(' ')[:6])
	startx, starty, width, height = int(startx), int(starty), int(width), int(height)
	return oid, parentid, startx, starty, width, height

def organism_parser(c: str):
	oid, ptrx, ptry, command, exitcode = (c.split(' ')[:5])
	ptrx, ptry = int(ptrx), int(ptry)
	return oid, ptrx, ptrx, ptry, command, exitcode

def kill_parser(c: str):
	oid = c.split(' ')[0]
	return {'id': oid}

parsers = {
	'MW': writing_parser,
	'O': organism_parser,
	'Q': queue_parser,
}

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt


if __name__ == '__main__':
	OData = dict()
	filename = "log011.txt"
	memory_size = 512
	memory = np.zeros((memory_size, memory_size), dtype=str)
	memory.fill('.')
	log = []
	with open(filename, 'r', encoding='utf-8') as f:
		line = ' '
		organisms = {}
		i = 0

		while line:
			line = f.readline()
			log = {i: [] for i in prefixes}
			commands = [i for i in line.split('|') if i != '' and i != '\n']
			for c in commands:
				for p in prefixes:
					if c.startswith(p):	
						log[p].append(c)
						break
			for o in log['MW']:
				c = writing_parser(o[3:])
				memory[c['x'], c['y']] = c['symbol']
			for o in log['Q']:
				oid, parentid, x, y, w, h = queue_parser(o[2:])
				body = memory[x:x+w, y:y+h]
				OData[oid] = {\
					'born': i,
					'id': oid,
					'parent': parentid,
					'x': x, 
					'y': y,
					'width': w,
					'height': h,
					'body': ''.join(''.join(list(_)) for _ in body),
					'life': '',
					'exitcodes': ''}
			for o in log['O']:
				oid, ptrx, ptrx, ptry, command, exitcode = organism_parser(o[2:])
				OData[oid]['life'] += command
				OData[oid]['exitcodes'] += exitcode
			

			i += 1
			if i%1000 == 0:
				print(i)

	df = pd.DataFrame(OData)
	df = pd.DataFrame(df.values.T)
	df.to_csv('orgs011.csv')
