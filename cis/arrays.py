import numpy as np

def read_organism(filename: str):
	with open(filename) as genome_file:
		genome = np.array([list(line.strip()) for line in genome_file])
	return genome

