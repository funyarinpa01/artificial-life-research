from memory import memory

class Queue():
	def __init__(self):
		self.organisms = []

	def add(self, organism):
		self.organisms.append(organism)

	def __iter__(self):
		for organism in self.organisms:
			yield organism

	def __len__(self):
		return len(self.organisms)

	def update(self):
		self.organisms.sort()
		new_organisms = []
		for o in self.organisms:
			if o.dead:
				memory.free(o.start, o.size)
			else:
				new_organisms.append(o)
		self.organisms = new_organisms

	def __str__(self):
		return ' '.join([f"[{i.id} {i.errors}]" for i in self.organisms])

queue = Queue()