import numpy as np

colors = {
    '.': (0.45, 0.45, 0.45),
    ':': (0.55, 0.55, 0.55),
    'a': (0, 0.35, 0.65),
    'b': (0, 0.6, 0.8),
    'c': (0, 0.8, 0.8),
    'd': (0, 0.8, 0.6),
    'x': (0.3, 0, 0.8),
    'y': (0.1, 0, 0.9),
    '^': (1, 0.5, 0),
    'v': (0.5, 1, 0),
    '>': (0, 1, 0.5),
    '<': (0, 0.5, 1),
    '&': (0.3, 0, 1),
    '?': (1, 0, 0.9),
    '1': (1, 1, 1),
    '0': (0, 0, 0),
    '-': (0, 0, 1),
    '+': (1, 0, 0),
    '~': (0, 0, 0.4),
    'L': (1, 0, 0.4),
    'W': (0, 1, 0.6),
    '@': (1, 0.2, 0),
    '$': (0, 0.7, 1),
    'S': (0.6, 0, 0.4),
    'P': (0.4, 0, 0.6),
    '#': (0.2, 0.2, 0.2),
    '*': (0.2, 0.4, 0.2),
    'o': (0.67, 0.38, 0.18),
}

instructions = {
    '.': [np.array([0, 0]), 'no_operation'],
    ':': [np.array([0, 1]), 'no_operation'],
    'a': [np.array([1, 0]), 'no_operation'],
    'b': [np.array([1, 1]), 'no_operation'],
    'c': [np.array([1, 2]), 'no_operation'],
    'd': [np.array([1, 3]), 'no_operation'],
    'x': [np.array([2, 0]), 'no_operation'],
    'y': [np.array([2, 1]), 'no_operation'],
    '^': [np.array([3, 0]), 'move_up'],
    'v': [np.array([3, 1]), 'move_down'],
    '>': [np.array([3, 2]), 'move_right'],
    '<': [np.array([3, 3]), 'move_left'],
    '&': [np.array([4, 0]), 'find_template'],
    '?': [np.array([5, 0]), 'if_not_zero'],
    '1': [np.array([6, 0]), 'one'],
    '0': [np.array([6, 1]), 'zero'],
    '-': [np.array([6, 2]), 'decrement'],
    '+': [np.array([6, 3]), 'increment'],
    '~': [np.array([6, 4]), 'subtract'],
    'L': [np.array([7, 0]), 'load_inst'],
    'W': [np.array([7, 1]), 'write_inst'],
    '@': [np.array([7, 2]), 'allocate_child'],
    '$': [np.array([7, 3]), 'split_child'],
    'S': [np.array([8, 0]), 'push'],
    'P': [np.array([8, 1]), 'pop'],
}

vectors = {
    'left': np.array([0, -1]),
    'right': np.array([0, 1]),
    'up': np.array([-1, 0]),
    'down': np.array([1, 0]),
}

max_iters_without_child = 500000
max_number_of_errors = 50000


width=100
height=100
memory_size = (width, height)
stack_length = 8

import time