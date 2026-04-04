// libs
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

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

void readKey(){
    char letter;
    do{
        read(STDIN_FILENO, &letter, 1);
        write(STDOUT_FILENO, &letter, 1);
    }while(letter != 'Q');
    return;
}


// main funcition
int main(){
    // start
    struct termios original;
    enableRawMode(&original);
    readKey();
    //end
    desableRawMode(&original);
    return 0;
}
