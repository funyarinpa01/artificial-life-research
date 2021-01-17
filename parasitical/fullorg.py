from organism import Organism, log


class FullOrganism(Organism):
	instruction_set = Organism.instruction_set
	instruction_set.update({
		'&': {'vector': [4, 0], 'name': 'find_template'},
		'?': {'vector': [5, 0], 'name': 'if_not_zero'},
		'1': {'vector': [6, 0], 'name': 'one'},
		'0': {'vector': [6, 1], 'name': 'zero'},
		'-': {'vector': [6, 2], 'name': 'decrement'},
		'+': {'vector': [6, 3], 'name': 'increment'},
		'~': {'vector': [6, 4], 'name': 'subtract'},
		'L': {'vector': [7, 0], 'name': 'load_instance'},
		'W': {'vector': [7, 1], 'name': 'write_instance'},
	})

	def find_template(self):
		register = self.next_command(1)
		if not self.valid_register(register):
			return
		template = []
		x, y = self.absolute_pointer_position()
		dx, dy = self.direction()
		x += dx #pointer at register symbol
		y += dy #pointer at register symbol
		while x >= 0 and x < self.game.memory.get_width() and\
			 y >= 0 and y < self.game.memory.get_height():
			x += dx
			y += dy
			if self.game.memory.get_command(x, y) == '.':
				template.append(':')
			elif self.game.memory.get_command(x, y) == ':':
				template.append('.')
			else:
				break
		if len(template) == 0:
			return
		i = 0
		while x >= 0 and x < self.game.memory.get_width() and\
			 y >= 0 and y < self.game.memory.get_height() and i < len(template):
			x += dx
			y += dy
			if self.game.memory.get_command(x, y) == template[i]:
				i += 1
			else:
				i = 0
		if i != len(template):
			return # didn't find template
		self.set_register_value(register, x, y)

	def if_not_zero(self):
		register = self.next_command(1)
		if self.valid_register_component(register):
			component = register
			register = self.next_command(2)
			value = [self.get_register_component_value(register, component)]
			shift = 1
		else:
			value = self.get_register_value(register)
			shift = 0

		if any(value):
			self.move(2+shift)
			# |<-ptr	  |<-ptr
			# |		   |on the next step: >
			# ?xa>^ --- ?xa>^
			#
			# |<-ptr	 |<-ptr
			# |		  |on the next step: ^
			# ?a>^ --- ?a>^
		elif isinstance(value, list):	
			self.move(1+shift)
			# |<-ptr	|<-ptr
			# |		 |on the next step: >
			# ?a>^ --- ?a>^
			#
			# |<-ptr	   |<-ptr
			# |			|on the next step: ^
			# ?xa>^ --- ?xa>^

	def zero(self):
		print('ZERO', file=log)
		register = self.next_command(1)
		print('register', register, file=log)
		self.set_register_value(register, 0, 0)
		print('successfull', file=log)

	def one(self):
		register = self.next_command(1)
		self.set_register_value(register, 1, 1)

	def load_instance(self):
		register_from = self.next_command(1)
		register_to = self.next_command(2)
		x, y = self.get_register_value(register_from)
		x, y = self.instruction_set[self.game.memory.get_command(x, y)]['vector']
		#print('loaded instance from', x, y, self.game.memory.get_command(x, y), self.instruction_set[self.game.memory.get_command(x, y)])
		self.set_register_value(register_to, x, y)

	def write_instance(self):
		if self.child_size == [0, 0]:
			return

		register_where = self.next_command(1)
		register_what = self.next_command(2)
		#print(f"writing from {register_what} to {register_where}", file=log)
		what = self.get_register_value(register_what)
		x, y = self.get_register_value(register_where)
		#self.set_register_value(register_to, x, y)
		#print(f"looking for symbol {register_what} to {register_where}", file=log)
		#print(f"looking for symbol {(x, y)} {what}", file=log)
		for k in self.instruction_set:
			#print(f"checking key {k}, {self.instruction_set[k]['vector']} Vs {what}", file=log)
			if self.instruction_set[k]['vector'] == what:
				self.game.memory.set_command(x, y, k)
				#print(f'set command {k} on {x} {y} ')
				return
		#print(f"no such command", file=log)
		raise ValueError('non existent command')

	def increment(self):
		register = self.next_command(1)
		if register in self._components:
			component = register
			register = self.next_command(2)
			value = self.get_register_component_value(register, component)
			self.set_register_component_value(register, component, value+1)
		else:
			valuex, valuey = self.get_register_value(register)
			self.set_register_value(register, valuex+1, valuey+1)

	def decrement(self):
		register = self.next_command(1)
		if register in self._components:
			#print('decrementing register component')
			component = register
			register = self.next_command(2)
			#print(f'register {register} component {component}')
			value = self.get_register_component_value(register, component)
			#print(f'value {value}')
			self.set_register_component_value(register, component, value-1)
			#print('value2', self.get_register_component_value(register, component))
		else:
			#print('decrementing register')
			valuex, valuey = self.get_register_value(register)
			self.set_register_value(register, valuex-1, valuey-1)

	def subtract(self):
		registera = self.next_command(1)
		registerb = self.next_command(2)
		registerc = self.next_command(3)
		ax, ay = self.get_register_value(registera)
		bx, by = self.get_register_value(registerb)
		self.set_register_value(registerc, ax-bx, ay-by)
