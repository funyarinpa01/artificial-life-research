import sys
import time

def delete_last_line():
    "Use this function to delete the last line in the STDOUT"

    #cursor up one line
    sys.stdout.write('\x1b[1A')

    #delete last line
    sys.stdout.write('\x1b[2K')

n = 5
for i in range(5):
	for j in range(n):
		print('#'*j, end=' ')
	n -= 1
	for j in range(n):
		#print('\x1b[1A')
		print('\r\r')
