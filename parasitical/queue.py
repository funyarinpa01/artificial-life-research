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

	def __str__(self):
		return ' '.join([f"[{i.id()} {i.errors()}]" for i in self.organisms])
