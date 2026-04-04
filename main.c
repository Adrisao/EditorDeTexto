// libs
#include <stdio.h>
#include <termios.h>

// configure the terminal
void enableRawMode(struct termios *original){
    // save the orignal config
    tcgetattr(STDIN_FILENO, original);
    // create and copy the config
    struct termios raw;
    raw = *original;
    // desable the echo and canon
    raw.c_lflag &= ~(ECHO | ICANON);
    // apply
    tcsetattr(STDIN_FILENO, TCSAFLUSH,  &raw);
    return;
}

// Configure back the terminal
void desableRawMode(struct termios *original){
    // apply the original config
    tcsetattr(STDIN_FILENO, TCSAFLUSH, original);
    return;
}

// main funcition
int main(){
    // start
    struct termios original;
    enableRawMode(&original);

    //end
    desableRawMode(&original);
    return 0;
}
