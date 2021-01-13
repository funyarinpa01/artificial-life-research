
#include <stdio.h>
#include <stdlib.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>




int width = 5, height = 5, x, y;
double start = 0;

EMSCRIPTEN_KEEPALIVE
EM_BOOL one_iteration() {
    x = rand() % 90;
    y = rand() % 90;
    double curr = EM_ASM_DOUBLE({
        return getTime();
    });
    if (curr - start > 3000) {
        start = EM_ASM_DOUBLE({
        return getTime();
        });
        EM_ASM({
            update($0, $1, $2, $3);
        }, x, y, width, height);
        //printf("Create new organism at (%d, %d) with width %d and height %d\n", x, y, width, height);
    }
    return EM_TRUE;
}

EMSCRIPTEN_KEEPALIVE
int main(){
    #ifdef __EMSCRIPTEN__
        srand(10);
        start = EM_ASM_DOUBLE({
                return getTime();
            });
        emscripten_request_animation_frame_loop(one_iteration, 0);    
    #else
        while (1) {
            one_iteration();
            //printf("Create new organism at (%d, %d) with width %d and height %d\n", x, y, width, height);
        }
    #endif
    return 0;
}
