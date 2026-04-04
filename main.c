// libs
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

// keys
#define ESC 27
#define UP_ARROW    'A'
#define DOWN_ARROW  'B'
#define RIGHT_ARROW 'C'
#define LEFT_ARROW  'D'

// bool values
#define TRUE 1
#define FALSE 0

// buffer
#define BUFFERSIZE 15
char buffer[BUFFERSIZE + 1];
int bufferSize = 0;

// cursor
int cursorPositionX = 0;

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
void disableRawMode(struct termios *original){
    // apply the original config
    tcsetattr(STDIN_FILENO, TCSAFLUSH, original);
    return;
}

// atulize the cursor position
void attCursor(){
    write(STDOUT_FILENO, "\r", 1);
    for (int i = 0; i < cursorPositionX; i++) write(STDOUT_FILENO, "\x1b[C", 3);
    return;
}

// move the letters for inserction
void mov(){
    if (bufferSize == BUFFERSIZE) return;
    for (int i = bufferSize; i > cursorPositionX; i--) buffer[i] = buffer[i - 1];
    bufferSize ++;
    return;
}

// move back the letters to remove (backspace)
void movBack(){
    if (bufferSize == 0) return;
    int i;
    for (i = cursorPositionX - 1; i < bufferSize; i++) buffer[i] = buffer[i+1];
    bufferSize --;
    buffer[bufferSize] = '\0';
    return;
}

// move back the letters to remove (DEL)
void delMovBack(){
    if (bufferSize == 0) return;
    int i;
    for (i = cursorPositionX; i < bufferSize; i++) buffer[i] = buffer[i+1];
    bufferSize --;
    buffer[bufferSize] = '\0';
    return;
}

// print line
void print(){
    if (bufferSize > BUFFERSIZE) return;
    write(STDOUT_FILENO, "\r", 1);
    write(STDOUT_FILENO, "\x1b[K", 3);
    write(STDOUT_FILENO, buffer, bufferSize);
    attCursor();
    return;
}

void addChar(char *letter){
    mov();
    buffer[cursorPositionX] = *letter;
    if (cursorPositionX < BUFFERSIZE) cursorPositionX ++;
    attCursor();
    return;
}

// backspace
void backspaceFunction(){
    if(bufferSize == 0 || cursorPositionX == 0) return;
    movBack();
    cursorPositionX --;
    return;
}

//get the key
char readKey(char *key){
    return read(STDIN_FILENO, key, 1);
}

// arrows and others keys
void arrows(){
    char seq;
    // get the next char of sequence
    char isValid = readKey(&seq);
    //if (isValid != 1) return;
    switch(seq){
    // up arrow
    case UP_ARROW:
        write(STDOUT_FILENO, "UP", 2);
        break;
    // down arrow
    case DOWN_ARROW:
        write(STDOUT_FILENO, "DOWN", 4);
        break;
    // right arrow
    case RIGHT_ARROW:
        if (cursorPositionX >= bufferSize) break;
        if (cursorPositionX >= 100) break;
        cursorPositionX ++;
        attCursor();
        break;
    // left arrow
    case LEFT_ARROW:
        if (cursorPositionX == 0) break;
        cursorPositionX --;
        attCursor();
        break;
        //DEL key
    case '3':
        isValid = readKey(&seq);
        if (isValid != 1) return;
        if (seq == '~'){
            if(cursorPositionX == bufferSize) break;
           delMovBack();
           print();
           attCursor();
        }
        break;
        // error or other key not implemented yet
    default:
        write(STDOUT_FILENO, "err", 3);
    }
    return;
}

// mainloop
void mainloop(){
    // vars
    char seq; //used if it needs a sequence
    char letter; // the main char
    char doWrite = TRUE; // if I should write
    char keepIt = TRUE; // continue te loop
    int isValid; // if the key is valid
    // main loop
    do{
        // reset
        doWrite = TRUE;

        // read the main char
        isValid = readKey(&letter);

        // if it's a valid char
        if (isValid != 1) continue;

        // backspace
        if(letter == 8 || letter == 127){
            //remove the last char
            backspaceFunction();
            print();
            doWrite = FALSE;
        }

        //arrows and esc
        if (letter == ESC){
            doWrite = FALSE;
            // read the sequence
            isValid =  readKey(&seq);
            // if it's just the esc and break the program
            if (isValid != 1){
                keepIt = FALSE;
                continue;
                }
            // if it's not the esc
            if (seq == '['){
                arrows();
            }
        }

        //Should I write it?
        if (doWrite){
            addChar(&letter);
            print();
        };

    }while(keepIt);
    return;
}


// main funcition
int main(){
    // start
    buffer[0] = '\0';
    buffer[BUFFERSIZE] = '\0';
    struct termios original;
    // enable raw
    enableRawMode(&original);

    // working
    mainloop();

    //end
    disableRawMode(&original);

    printf("\n - BUFFER: %s", buffer);
    return 0;
}
