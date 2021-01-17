#include <stdint.h>
#include <stdio.h>

void main() {
	int j = 0;
	for (int i = 0; j < 50; i+=100000000) {
		unsigned int x = i;
		printf("%u %d\n", i, x);
		j++;
	}
}