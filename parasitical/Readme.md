# Visualization

1. Store organism tree
2. Save all changes to memory
3. Save lifecycle of every organism
4. Save queue

## Optimizing this
1. we only save 1 tree
2. save time of creation of every organism (1 list of numbers)
3. change memory and EVERY change to it in form (address, new symbol)
4. save queue on every iteration (optimization - ?)

# Gathering statistics
1. identify sequences of "lifecycle" \ genome that match
	a. how do we choose them
	b. position of genome is important if we analyze in 2D
2. lifecycle - genome matching
3. regular expressions?
4. controlling for environment?



# Fixes for random
1. NEVER change ACTIVE memory. Only ALLOCATED should be.
2. 