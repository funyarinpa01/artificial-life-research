from fullorg import FullOrganism
from time import sleep

class Parasite(FullOrganism):
    instruction_set = FullOrganism.instruction_set
    instruction_set.update({'*': {'vector': [8, 1], 'name': 'drop_child'},'o': {'vector': [8, 2], 'name': 'find_cycle'}})

    def __init__(self, parent_id, startx, starty, w, h, game_reference):
        super().__init__(parent_id, startx, starty, w, h, game_reference)
        self.tired = 0
        self.directions = [(0, -1), (-1, 0), (1, 0), (0, 1)]
        self.c = 0

    def find_cycle(self):
        '''
        choose the direction in which to look for the cycle
        direction: >
        |----------------------|
        |          v <<<<<<    |
        |          v      ^    | We will, hopefully,
        |>START>   v      ^    | find something
        |          v      ^    | life the looped
        |          v      ^    | frame to the right
        |          >>>>>>>^    |
        |----------------------|
        '''
        print('parasite is looking for cycle')
        self.c = (self.c+1)%4
        dx, dy = self.directions[self.c]
        print(dx, dy)
        x, y = self.position()
        if dx == 0 and dy == 1:
            y += self.size()[1]
        elif dx == 1 and dy == 0:
            x += self.size()[0]
        cycled = []
        dirs = []
        for iteration in range(1000):
            cycled.append((x, y))
            dirs.append((dx, dy))
            x += dx
            y += dy
            if self.game.memory.get_command(x, y) == '>':
                dx, dy = 0, 1
            if self.game.memory.get_command(x, y) == '<':
                dx, dy = 0, -1
            if self.game.memory.get_command(x, y) == '^':
                dx, dy = -1, 0
            if self.game.memory.get_command(x, y) == 'v':
                dx, dy = 1, 0
            for (di, dj), (i, j) in zip(cycled, dirs):
                if i == x and j == y:
                    print('parasite found cycle', i, j)
                    self.tired = 100
                    self._ptr = [i, j]
                    self._direction = [di, dj]
                    sleep(2)
                    return
            
        self.tired = 10

    def cycle(self):
        print('parasite cycling', self.absolute_pointer_position())
        if self.tired > 0:
            self.tired -= 1
            print('tired')
            return
        super().cycle()
        print(self)

    def drop_child(self):
        '''
        look for free space and store its position\size
        to the same registers host organism does
        '''
        print('parasite dropping child')
        self.c = (self.c+1)%4
        startx, starty = self.position()
        w, h = self.size()
        dx, dy = self.directions[self.c%2]
        is_space_found = False
        x, y = self.position()
        width, height = self.size()
        while x >= 0 and x < self.game.memory.get_width() and y >= 0 and y < self.game.memory.get_height():
            print('looking for place', x, y)
            if self.game.memory.is_free(x, y, width, height):
                #print("found space")
                self.child_start = (x, y)
                is_space_found = True
                break
            x += dx
            y += dy
        if is_space_found:
            print('found space')
            self.game.memory.allocate(x, y, width, height)
            self._ptr = [0, 1]
            self._direction = [0, 1]
