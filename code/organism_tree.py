class Node():
	def __init__(self, organism, parent):
		self.item = organism
		self.children = []


class OrganismTree():
	def __init__(self):
		self.tree = []

	def add_organism(parent_):