
#include <stdio.h>
#include <stdlib.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>


extern void update(int startX, int startY, int width, int height);
extern long getTime();


int width = 5, height = 5, x, y;
long end = 0;
long start = 0;

EMSCRIPTEN_KEEPALIVE
EM_BOOL one_iteration() {
    x = rand() % 100;
    y = rand() % 100;
    if (getTime() - start > 5000) {
        start = getTime();
        update(x, y, width, height);
        //printf("Create new organism at (%d, %d) with width %d and height %d\n", x, y, width, height);
    }
    return EM_TRUE;
}

EMSCRIPTEN_KEEPALIVE
int run(){
    #ifdef __EMSCRIPTEN__
        srand(NULL);
        start = getTime();
        // printf("hello%d", 5);
        emscripten_set_main_loop(one_iteration, 100, 1);
    return 0;
    #else
        while (1) {
            one_iteration();
            //printf("Create new organism at (%d, %d) with width %d and height %d\n", x, y, width, height);
        }
    #endif
    return 0;
}

int main () {
    return 0;
}