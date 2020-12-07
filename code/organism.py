import numpy as np
import common
from memory import memory
from queue import queue

organisms = []

class Organism():
    GLOBAL_ID = 0
    def __init__(self, address: np.array, size: np.array, parent_id):
        self.id = Organism.GLOBAL_ID
        Organism.GLOBAL_ID += 1
        self.parent = parent_id

        self.ip = np.array(address)
        self.vector = np.array([0, 1], dtype=np.bool) #default value

        self.size = size

        self.start = np.array(address)
        self.regs = {'a': np.array([0, 0]),
                    'b': np.array([0, 0]),
                    'c': np.array([0, 0]),
                    'd': np.array([0, 0])}
        self.stack = []

        self.errors = 0

        self.reproduction_cycle = 0
        self.children = 0
        self.child_size = np.array([0, 0])
        self.child_start = np.array([0, 0])

        self.coords = {'x': 0, 'y': 1}

        self.dead = False

        organisms.append(self)
        queue.add(self)

    def no_operation(self):
        pass

    def move_up(self):
        self.vector = common.vectors['up']

    def move_down(self):
        self.vector = common.vectors['down']

    def move_right(self):
        self.vector = common.vectors['right']

    def move_left(self):
        self.vector = common.vectors['left']

    def ip_offset(self, offset: int = 0) -> np.array:
        return self.ip + offset * self.vector

    def inst(self, offset: int = 0) -> str:
        return memory.read(self.ip_offset(offset))

    def find_template(self):
        register = self.inst(1)
        template = []
        for i in range(2, max(self.size)):
            if self.inst(i) in ['.', ':']:
                template.append(':' if self.inst(i) == '.' else '.')
            else:
                break
        counter = 0
        for i in range(i, max(self.size)):
            if self.inst(i) == template[counter]:
                counter += 1
            else:
                counter = 0
            if counter == len(template):
                self.regs[register] = self.ip + i * self.vector
                break

    def if_not_zero(self):
        if self.inst(1) in self.coords.keys():
            value = self.regs[self.inst(2)][self.coords[self.inst(1)]]
            start_from = 1
        else:
            value = self.regs[self.inst(1)]
            start_from = 0

        if not np.any(value):
            self.ip = self.ip_offset(start_from + 1)
        else:
            self.ip = self.ip_offset(start_from + 2)


    def increment(self):
        if self.inst(1) in self.coords.keys():
            self.regs[self.inst(2)][self.coords[self.inst(1)]] += 1
        else:
            self.regs[self.inst(1)] += 1

    def decrement(self):
        if self.inst(1) in self.coords.keys():
            self.regs[self.inst(2)][self.coords[self.inst(1)]] -= 1
        else:
            self.regs[self.inst(1)] -= 1

    def zero(self):
        self.regs[self.inst(1)] = np.array([0, 0])

    def one(self):
        self.regs[self.inst(1)] = np.array([1, 1])

    def subtract(self):
        self.regs[self.inst(3)] = self.regs[self.inst(1)] - self.regs[self.inst(2)]

    def allocate_child(self):
        size = np.copy(self.regs[self.inst(1)])
        if (size <= 0).any():
            return
        is_space_found = False
        for i in range(2, max(common.memory_size)):
            if not memory.can_allocate_memory(size, self.ip_offset(i)):
                #print('no space')
                pass
            else:
                self.child_start = self.ip_offset(i)
                self.regs[self.inst(2)] = np.copy(self.child_start)
                is_space_found = True
                break
        if is_space_found:
            #print('found space')
            self.child_size = np.copy(self.regs[self.inst(1)])
            memory.allocate(self.child_size, self.child_start)

    def load_inst(self):
        self.regs[self.inst(2)] = common.instructions[
            memory.read(self.regs[self.inst(1)])
        ][0]

    def write_inst(self):
        if not np.array_equal(self.child_size, np.array([0, 0])):
            memory.write(self.regs[self.inst(1)], self.regs[self.inst(2)])

    def push(self):
        if len(self.stack) < common.stack_length:
            self.stack.append(np.copy(self.regs[self.inst(1)]))

    def pop(self):
        self.regs[self.inst(1)] = np.copy(self.stack.pop())

    def split_child(self):
        if not np.array_equal(self.child_size, np.array([0, 0])):
            #memory.free(self.child_start, self.child_size)
            self.__class__(self.child_start, self.child_size, self.id)
            self.children += 1
            self.reproduction_cycle = 0
        self.child_size = np.array([0, 0])
        self.child_start = np.array([0, 0])

    def __lt__(self, other):
        return self.errors < other.errors

    def die(self):
        self.dead = True

    def cycle(self):
        try:
            getattr(self, common.instructions[self.inst()][1])()
        except Exception:
            self.errors += 1

        if self.errors > common.max_number_of_errors or\
            self.reproduction_cycle > common.max_iters_without_child:
            self.die()
            return

        new_ip = self.ip + self.vector
        self.reproduction_cycle += 1

        if (new_ip < 0).any() or (new_ip - common.memory_size > 0).any():
            return None
        self.ip = np.copy(new_ip)
        return None

    def __str__(self):
        return "Organism %d" % self.id