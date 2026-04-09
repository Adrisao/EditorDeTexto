// libs
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <string.h>

// keys
#define ESC 27
#define UP_ARROW    'A'
#define DOWN_ARROW  'B'
#define RIGHT_ARROW 'C'
#define LEFT_ARROW  'D'
#define ENTER1      '\n'
#define ENTER2      '\r'

// bool values
#define TRUE 1
#define FALSE 0

// buffer
#define BUFFERSIZE 2045
char buffer[BUFFERSIZE + 1];
// the real buffer size
int bufferSize = 0;
// lines positions
#define MAXLINES 20
int lines[MAXLINES];
char validLine[MAXLINES];

// cursor virtual
int virtualCursor = 0;

int visualCursor[2] = {1, 1};

// save file
void saveFile(){
    char deuGood = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (deuGood == -1) return;
    write(deuGood, buffer, bufferSize);
    close(deuGood);
}

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

void lookLinesFinal(){
    visualCursor[0] = 0;
    visualCursor[1] = 0;
    for (int i = 0; i < virtualCursor; i ++){
        if (buffer[i] == '\n'){
            visualCursor[0] = 0;
            visualCursor[1] ++;
        }else visualCursor[0] ++;
    }
    return;
}

// atulize the cursor position
void attCursor() {
    char text[32];
    lookLinesFinal(); // X e Y base 0
    sprintf(text, "\x1b[%d;%dH", visualCursor[1] + 1, visualCursor[0] + 1);
    write(STDOUT_FILENO, text, strlen(text));
}

// move the letters for inserction
void mov(){
    if (bufferSize == BUFFERSIZE) return;
    for (int i = bufferSize; i > virtualCursor; i--) buffer[i] = buffer[i - 1];
    bufferSize ++;
    return;
}

// move back the letters to remove (backspace)
void movBack(){
    if (bufferSize == 0) return;
    int i;
    for (i = virtualCursor - 1; i < bufferSize; i++) buffer[i] = buffer[i+1];
    bufferSize --;
    buffer[bufferSize] = '\0';
    return;
}

// move back the letters to remove (DEL)
void delMovBack(){
    if (bufferSize == 0) return;
    int i;
    for (i = virtualCursor; i < bufferSize; i++) buffer[i] = buffer[i+1];
    bufferSize --;
    buffer[bufferSize] = '\0';
    return;
}

// print line
void print(){
    if (bufferSize > BUFFERSIZE) return;
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    write(STDOUT_FILENO, buffer, bufferSize);
    attCursor();
    return;
}

// add the char at the buffer
void addChar(char *letter){
    mov();
    buffer[virtualCursor] = *letter;
    if (virtualCursor < BUFFERSIZE) virtualCursor ++;
    attCursor();
    return;
}

// backspace
void backspaceFunction(){
    if(bufferSize == 0 || virtualCursor == 0) return;
    movBack();
    virtualCursor --;
    return;
}

//get the key
char readKey(char *key){
    return read(STDIN_FILENO, key, 1);
}

// up arrow key function
void moveUp(){
    int i = virtualCursor;
    while (i > 0 && buffer[i] != '\n') i --;
    if (i > 0) i --;
    while (i > 0 && buffer[i] != '\n') i --;
    if (i != 0) i ++;
    virtualCursor = i;
    attCursor();
    return;
}

void moveDown(){
    int i = virtualCursor;
    int diff;
    while(i < bufferSize && buffer[i] != '\n') i ++;
    if (i < bufferSize) i ++;
    virtualCursor = i;
    attCursor();
    return;
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
        //write(STDOUT_FILENO, "UP", 2);
        moveUp();
        break;
    // down arrow
    case DOWN_ARROW:
        //write(STDOUT_FILENO, "DOWN", 4);
        //printf("Cursor: %d.\n", virtualCursor);
        moveDown();
        break;
    // right arrow
    case RIGHT_ARROW:
        if (virtualCursor >= bufferSize) break;
        virtualCursor ++;
        attCursor();
        break;
    // left arrow
    case LEFT_ARROW:
        if (virtualCursor == 0) break;
        virtualCursor --;
        attCursor();
        break;
        //DEL key
    case '3':
        isValid = readKey(&seq);
        if (isValid != 1) return;
        if (seq == '~'){
            if(virtualCursor == bufferSize) break;
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

// enter function processing
void enterFunc(){
    char data = '\n';
    addChar(&data);
    print();
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

        // block the break line
        if (letter == ENTER1 || letter == ENTER2){
            enterFunc();
            continue;
        };

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
    // save the terminal
    write(STDOUT_FILENO, "\x1b[?1049h", 8);
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    // start
    buffer[0] = '\0';
    print();
    buffer[BUFFERSIZE] = '\0';
    struct termios original;
    lines[0] = 0; //first line
    validLine[0] = 1; // first line is valid
    for (int i = 1; i < MAXLINES; i++) validLine[i] = 0; // others lines do not exist yet
    // enable raw
    enableRawMode(&original);

    // working
    mainloop();

    //end
    disableRawMode(&original);

    saveFile();

    // return to terminal
    write(STDOUT_FILENO, "\x1b[?1049l", 8);
    //printf("\n - BUFFER: %s", buffer);
    return 0;
}
