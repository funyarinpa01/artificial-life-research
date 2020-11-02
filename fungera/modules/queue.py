from copy import copy
import modules.common as c


class Queue:
    def __init__(self):
        self.organisms = []
        self.archive = []
        self.index = None

    def add_organism(self, organism):
        self.organisms.append(organism)
        if self.index is None:
            self.index = 0
            self.organisms[self.index].is_selected = True

    def get_organism(self):
        try:
            return self.organisms[self.index]
        except IndexError:
            if len(self.organisms) == 0:
                raise Exception('No more organisms alive!')
            return self.organisms[0]

    def select_next(self):
        if self.index + 1 < len(self.organisms):
            self.organisms[self.index].is_selected = False
            self.organisms[self.index].update()
            self.index += 1
            self.organisms[self.index].is_selected = True
            self.organisms[self.index].update()

    def select_previous(self):
        if self.index - 1 >= 0:
            self.organisms[self.index].is_selected = False
            self.organisms[self.index].update()
            self.index -= 1
            self.organisms[self.index].is_selected = True
            self.organisms[self.index].update()

    def cycle_all(self):
        for organism in copy(self.organisms):
            organism.cycle()

    def kill_organisms(self):
        sorted_organisms = sorted(self.organisms, reverse=True)
        ratio = int(len(self.organisms) * c.config['kill_organisms_ratio'])
        for organism in sorted_organisms[:ratio]:
            organism.kill()
        self.organisms = sorted_organisms[ratio:]

    def update_all(self):
        for organism in copy(self.organisms):
            organism.update()

    def toogle_minimal(self):
        organisms = self.organisms
        self.organisms = []
        for organism in organisms:
            organism.toogle()


queue = Queue()
