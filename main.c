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
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
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


// get and write
void readKey(){
    // vars
    char seq; //used if it needs a sequence
    char letter; // the main char
    char doWrite = 1; // if I should write
    char keepIt = 1; // continue te loop
    // main loop
    do{
        // reset
        doWrite = 1;
        letter = -1;

        // read the main char
        read(STDIN_FILENO, &letter, 1);

        // if it's a valid char
        if (letter == -1) continue;

        // backspace
        if(letter == 8 || letter == 127){
            //remove the last char
            write(STDOUT_FILENO, "\b \b", 3);
            doWrite = 0;
        }

        //arrows and esc
        if (letter == 27){
            doWrite = 0;
            seq = -1;
            // read the sequence
            read(STDIN_FILENO, &seq, 1);
            // if it's just the esc
            if (seq == -1){keepIt = 0; continue;}
            // if it's not the esc
            if (seq == '['){
                // get the next char of sequence
                read(STDIN_FILENO, &seq, 1);
                switch(seq){
                // up arrow
                case 'A':
                    write(STDOUT_FILENO, "UP", 2);
                    break;
                // down arrow
                case 'B':
                    write(STDOUT_FILENO, "DOWN", 4);
                    break;
                // right arrow
                case 'C':
                    write(STDOUT_FILENO, "RIG", 3);
                    break;
                // left arrow
                case 'D':
                    write(STDOUT_FILENO, "lef", 3);
                    break;
                // error or other key not implemented yet
                default:
                    write(STDOUT_FILENO, "err", 3);
                }
            }
        }

        //Should I write it?
        if (doWrite) write(STDOUT_FILENO, &letter, 1);
    }while(keepIt);
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
