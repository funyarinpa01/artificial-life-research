log = open('orglog.txt', 'w')

class Organism:
	__ID = 0
	_components = {'x': 0, 'y': 1}
	_register_symbols = ['a', 'b', 'c', 'd']
	instruction_set = {\
		'.': {'vector': [0, 0], 'name': 'no_operation'},
		':': {'vector': [0, 1], 'name': 'no_operation'},
		'a': {'vector': [1, 0], 'name': 'no_operation'},
		'b': {'vector': [1, 1], 'name': 'no_operation'},
		'c': {'vector': [1, 2], 'name': 'no_operation'},
	    'd': {'vector': [1, 3], 'name': 'no_operation'},
		'x': {'vector': [2, 0], 'name': 'no_operation'},
		'y': {'vector': [2, 1], 'name': 'no_operation'},
		'^': {'vector': [3, 0], 'name': 'up'},
		'v': {'vector': [3, 1], 'name': 'down'},
		'>': {'vector': [3, 2], 'name': 'right'},
		'<': {'vector': [3, 3], 'name': 'left'},
		'@': {'vector': [7, 2], 'name': 'allocate_child'},
		'$': {'vector': [7, 3], 'name': 'split_child'},
		'S': {'vector': [8, 0], 'name': 'push'},
		'P': {'vector': [8, 1], 'name': 'pop'},
	}
	def __init__(self, parent_id, startx, starty, w, h, game_reference):
		self._position = [startx, starty]
		self._size = (w, h)
		self._ptr = [0, 0]
		self._id = Organism.__ID
		Organism.__ID += 1
		self._direction = (1, 0)
		self._stack = []
		self._stack_size = 8
		self._registers = {
			'a': [0, 0],
			'b': [0, 0],
			'c': [0, 0],
			'd': [0, 0],
		}
		self._child_position = [0, 0]
		self._child_size = [0, 0]
		self._children = 0
		self._reproduction_cycle = 0
		self._dead = False
		self._errors = 0
		self._life = []
		self.game = game_reference
		self.game.queue.add(self)
		self.birth = game_reference.iteration

	def die(self):
		self._dead = True

	def dead(self):
		return self._dead

	def reproduction_cycle(self):
		return self._reproduction_cycle

	def errors(self):
		return self._errors

	def life(self):
		return self._life

	def __lt__(self, other):
		return self.errors() < other.errors()

	def __str__(self):
		return f"Organism {self._id}\nregisters {self._registers}\nstack {self._stack}\n"

	def position(self):
		return self._position

	def size(self):
		return self._size

	def id(self):
		return self._id

	def absolute_pointer_position(self):
		x, y = self.pointer_position()
		return x+self._position[0], y+self._position[1]

	def pointer_position(self):
		return self._ptr

	def no_operation(self):
		return

	def direction(self):
		return self._direction

	def up(self):
		#print('up')
		self._direction = (-1, 0)

	def down(self):
		#print('down')
		self._direction = (1, 0)

	def right(self):
		#print('right')
		self._direction = (0, 1)

	def left(self):
		#print('left')
		self._direction = (0, -1)

	def get_register_value(self, symbol):
		if symbol not in self._registers:
			raise ValueError(f"{symbol} invalid register")
		return self._registers[symbol]

	def get_register_component_value(self, register, component):
		if component not in Organism._components:
			raise ValueError(f"{component} invalid component")
		if register not in self._registers:
			raise ValueError(f"{component} invalid register")
		return self._registers[register][Organism._components[component]]

	def set_register_value(self, symbol, x, y):
		self._registers[symbol] = [x, y]

	def set_register_component_value(self, register, component, value: int):
		self._registers[register][Organism._components[component]] = value

	def confirm_allocation_of_child(self, x, y, width, height):
		#print('chechking allocation')
		if not self.game.memory.is_free(x, y, width, height):
			# can't allocate this chunk of memory
			return
		self.child_size = (width, height)
		self.child_start = (x, y)
		#print('allocating memory')
		self.game.memory.allocate(x, y, width, height)
		#print('allocated memory')

	def allocate_child(self):
		'''
		a default allocation function
		'''
		size_register = self.next_command(1)
		width, height = self.get_register_value(size_register)
		#print('allocating', width, height)
		if width <= 0 or height <= 0:
			#print("invalid area")
			return

		is_space_found = False
		x, y = self.absolute_pointer_position()
		#print(x, y)
		dx, dy = self.direction()
		while x >= 0 and x < self.game.memory.get_width() and\
			y >= 0 and y < self.game.memory.get_height():
			#print('looking for place', x, y)
			if self.game.memory.is_free(x, y, width, height):
				#print("found space")
				self.child_start = (x, y)
				position_register = self.next_command(2)
				#print('will write position to', position_register)
				self.set_register_value(position_register, x, y)
				is_space_found = True
				break
			x += dx
			y += dy

		if is_space_found:
			self.child_size = (width, height)
			self.game.memory.allocate(x, y, width, height)

	@classmethod
	def valid_register(cls, symbol):
		return (symbol in cls._register_symbols)

	@classmethod
	def valid_register_component(cls, symbol):
		return (symbol in cls._components)

	def next_command(self, n: int):
		x, y = self.absolute_pointer_position()
		dx, dy = self.direction()
		#print('gettingg command on', x+dx*n, y+dy*n)
		return self.game.memory.get_command(x+dx*n, y+dy*n)

	def push(self):
		if len(self._stack) == self._stack_size:
			print("full stack", file=log)
			raise ValueError("can't push into full stack")
		register = self.next_command(1)
		#print(f"pushing {self.get_register_value(register).copy()}", file=log)
		self._stack.append(self.get_register_value(register).copy())
		#print(f"pushed {self._stack[self._stacktop]}", file=log)

	def pop(self):
		register = self.next_command(1)
		x, y = self._stack.pop()
		self.set_register_value(register, x, y)

	def split_child(self):
		if not (self.child_size == [0, 0]):
			startx, starty = self.child_start
			width, height = self.child_size
			self.game.memory.activate(startx, starty, width, height)
			self.__class__(self._id, startx, starty, width, height, self.game)
			self._children += 1
			self._reproduction_cycle = 0
		self.child_size = [0, 0]
		self.child_start = [0, 0]

	def move(self, n):
		x, y = self.pointer_position()
		dx, dy = self.direction()
		self._ptr = [x+dx*n, y+dy*n]

	def cycle(self):
		try:
			command = self.next_command(0)
			x, y = self.pointer_position()
			#print(command, file=open('hello.txt', 'a'))
			try:
				getattr(self, self.instruction_set[command]['name'])()
				#print(f"O {self._id} {(x, y)} {command} 1")
				#print(f"O {self._id} {(x, y)} {command} 1", end='|', file=log)
				self._life.append((command, 1))
			except Exception as e:
				#print(f"O {self._id} {(x, y)} {command} 0", end='|', file=log)
				print(e)
				self._life.append((command, 0))
				self._errors += 1
		except Exception as e:
			#print(f"O {self._id} {self.pointer_position()} FAIL {self.errors()}", end='|', file=log)
			self._errors += 1

		self.move(1)
		self._reproduction_cycle += 1
