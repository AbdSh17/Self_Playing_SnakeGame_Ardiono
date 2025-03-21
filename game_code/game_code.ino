// =============================== INCLUDE ===============================
#include "JoystickControl.h"
#include "queue.h"
#include <LedControl.h>
// =============================== INCLUDE ===============================

// =============================== DEFINE ===============================
const byte PAUSE = 0;
const byte LEFT = 1;
const byte UP = 2;
const byte RIGHT = 3;
const byte DOWN = 4;
const byte STABLE = 5;

const byte ROWS = 8;
const byte COLUMNS = 8;

const byte apple = 1;
const byte snake = 2;
const byte none = 0;
// =============================== DEFINE ===============================

// =============================== CONST ===============================
const int xaxis = A0;
const int yaxis = A1;
const int sw = 2;

const long appleInterval = 8000;
const long moveInterval = 1;
// =============================== CONST ===============================

// =============================== GLOBAL ===============================

volatile bool isPaused = false;

byte userX = 0, userY = 0;
byte length = 1;
byte userDirection = STABLE;

LedControl lc = LedControl(11, 13, 10, 1);
// =============================== GLOBAL ===============================

// =============================== DECLARATOIN ===============================
byte get_Direction();
void print_location(byte, byte);
bool isPoint(int, int);
bool go_down();
bool go_up();
bool go_left();
bool go_right();
void switchPoints(byte, byte, byte, byte);
void set_apple();
void printGameOver();

// =============================== DECLARATOIN ===============================

typedef struct leds *led;

struct leds
{
    byte state;
    byte previousLoc;
    bool isVisited;
};

leds ledState[ROWS][COLUMNS];

// Switch interrupt
void handleSwitch(){
    // ========== For handle bouncing ==========
    delay(10);
    static bool toggle = false;
    // =========================================
    if (!digitalRead(sw) && !toggle)
    {
        isPaused = !isPaused;
    }
    toggle = !toggle;
}

void setLedStates(){
    for (byte i = 0; i < ROWS; i++)
    {
        for (byte j = 0; j < COLUMNS; j++)
        {
            ledState[i][j].state = none;
            ledState[i][j].previousLoc = 0;
            ledState[i][j].isVisited = false;
        }
    }
}

void clearVisitedStates() {
    for (byte i = 0; i < ROWS; i++)
    {
        for (byte j = 0; j < COLUMNS; j++)
        {
            ledState[i][j].isVisited = false;
        }
    }
}

void printLedStates() {
    Serial.println("=================================");
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLUMNS; j++)
        { // nextStep
            Serial.print(ledState[i][j].previousLoc);
            Serial.print(" ");
        }
        Serial.println("");
    }
    Serial.println("=================================\n");
}


void setup()
{
    setupJoystick(A0, A1, sw);
    attachInterrupt(digitalPinToInterrupt(sw), handleSwitch, FALLING); // interrupt for handling switch pressing ;

    lc.shutdown(0, false); // Turn on the MAX7219
    lc.setIntensity(0, 8); // Set brightness (0 to 15)
    lc.clearDisplay(0);    // Clear the display

    setLedStates();
    ledState[3][7].state = apple;
    lc.setLed(0, userY, userX, true);
    ledState[userY][userX].state = snake;
}

void restart()
{
    clearMatrix();
    setLedStates();
    userX = userY = 0;
    lc.setLed(0, userY, userX, true);
    ledState[userY][userX].state = snake;
}


int countt = 0;
void loop()
{

    // static unsigned long previousMillisApple = 0;
    // static unsigned long previousMillisMove = 0;

    // unsigned long currentMillis = millis();

    // if (currentMillis - previousMillisApple >= appleInterval){
    //     previousMillisApple = currentMillis;
    //     set_apple();
    // }

    // if (currentMillis - previousMillisMove >= moveInterval){
    //     previousMillisMove = currentMillis;

    //     byte joystickDirection = get_Direction();
    //     static byte lastDirection = joystickDirection;

    //     if (joystickDirection == STABLE)
    //     {
    //         joystickDirection = lastDirection;
    //     }

    //     bool lost = false;

    //     if (joystickDirection == RIGHT)
    //     {
    //         lastDirection = RIGHT;
    //         lost = go_right();
    //     }

    //     else if (joystickDirection == LEFT)
    //     {
    //         lastDirection = LEFT;
    //         lost = go_left();
    //     }

    //     else if (joystickDirection == UP)
    //     {
    //         lastDirection = UP;
    //         lost = go_up();
    //     }

    //     else if (joystickDirection == DOWN)
    //     {
    //         lastDirection = DOWN;
    //         lost = go_down();
    //     }

    //     Serial.print("DIRECTION:   ");
    //     Serial.println(joystickDirection);

    //     if (lost)
    //     {
    //         printGameOver();
    //         restart();
    //         joystickDirection = lastDirection = STABLE;
    //     }
    // }
  // enQueue(q, 1, 6);
  // printQueue(q);
  delay(3000);
  if (countt < 2) {
    getBFSPath();
  }
  countt++;
}

void set_apple()
{
    byte row = 1, column = 0;

    while (1)
    {
        row = random(0, 8);
        column = random(0, 8);
        if (!ledState[row][column].state)
            break;
    }
    lc.setLed(0, row, column, true);
    ledState[row][column].state = apple;
}
queueHeader getBFSPath() {
  byte path[2];
  bfs(path, 9, 9);
  queueHeader pathQueue = initializeQueue();
  enQueue(pathQueue, path[0], path[1]);

  while(!(path[0] == userY && path[1] == userX))  
  {
    bfs(path, path[0], path[1]);
    enQueue(pathQueue, path[0], path[1]);
  }

  printQueue(pathQueue);
  return pathQueue;
}

void bfs(byte *path, byte yAxis, byte xAxis) {
  queueHeader q = initializeQueue();
  enQueue(q, userY, userX); 
  ledState[userY][userX].isVisited = true;  
  byte head[2] = {0, 0};
  int count = 0;

  while(head[0] != 255) {
    GetQueueHead(q, head);
    byte x = head[1], y = head[0];  

    if (y < 7 && ledState[y + 1][x].state != snake && !ledState[y + 1][x].isVisited) {  
      if (ledState[y + 1][x].state == apple || (y + 1 == yAxis && x == xAxis)) {  
        path[0] = y;  
        path[1] = x;  
        clearVisitedStates();
        freeQueue(q);
        return;
      }
      ledState[y + 1][x].isVisited = true;  
      enQueue(q, y + 1, x);  
    }

    if (y > 0 && ledState[y - 1][x].state != snake && !ledState[y - 1][x].isVisited) {  
      if (ledState[y - 1][x].state == apple || (y - 1 == yAxis && x == xAxis)) {  
        path[0] = y;  
        path[1] = x;  
        clearVisitedStates();
        freeQueue(q);
        return;
      }
      ledState[y - 1][x].isVisited = true;  
      enQueue(q, y - 1, x);  
    }

    if (x < 7 && ledState[y][x + 1].state != snake && !ledState[y][x + 1].isVisited) {  
      if (ledState[y][x + 1].state == apple || (x + 1 == xAxis && y == yAxis)) {  
        path[0] = y;  
        path[1] = x;  
        clearVisitedStates();
        freeQueue(q);
        return;
      }
      ledState[y][x + 1].isVisited = true;  
      enQueue(q, y, x + 1);  
    }

    if (x > 0 && ledState[y][x - 1].state != snake && !ledState[y][x - 1].isVisited) {  
      if (ledState[y][x - 1].state == apple || (x - 1 == xAxis && y == yAxis)) {  
        path[0] = y;  
        path[1] = x;  
        clearVisitedStates();
        freeQueue(q);
        return;
      }
      ledState[y][x - 1].isVisited = true;  
      enQueue(q, y, x - 1);  
    }

    count++;
    if(count == 64) {
      Serial.print("loop");
      break;
    }
    // printQueue(q);
    // Serial.println("=====================================\n");

    dequeue(q);
  }
  // Serial.print("ggg");
  clearVisitedStates();
  freeQueue(q);
}


/*
07:38:25.814 -> ( 0, 0 ) - 
07:38:25.814 -> ( 0, 0 ) - ( 3, 4 ) - ( 3, 2 ) - ( 4, 3 ) - ( 2, 3 ) - 
07:38:25.879 -> =====================================
07:38:25.913 -> 
07:38:25.913 -> ( 3, 4 ) - ( 3, 2 ) - ( 4, 3 ) - ( 2, 3 ) - ( 3, 4 ) - ( 3, 2 ) - ( 4, 3 ) - ( 2, 3 ) - 

*/

void switchPoints(byte userX0, byte userY0, byte userX1, byte userY1)
{
    lc.setLed(0, userY0, userX0, false);
    ledState[userY0][userX0].state = none;

    lc.setLed(0, userY1, userX1, true);
    ledState[userY1][userX1].state = snake;

    ledState[userY1][userX1].previousLoc = ledState[userY0][userX0].previousLoc;
}

bool go_right()
{

    if (userX == 7)
    {
        return false;
    }

    byte state = getState(userY, userX + 1);

    if (!state)
    {
        switchPoints(userX, userY, userX + 1, userY);
        byte tempY = userY, tempX = userX;
        userX++;
        if (ledState[userY][userX].previousLoc)
        {
            ledState[userY][userX].previousLoc = LEFT;
        }

        for (byte i = 1; i < length; i++)
        {

            if (!ledState[tempY][tempX].previousLoc)
            {
                ledState[tempY][tempX].state = none;
            }

            if (ledState[tempY][tempX].previousLoc == RIGHT)
            {
                switchPoints(tempX + 1, tempY, tempX, tempY);
                if (ledState[tempY][tempX].previousLoc)
                {
                    ledState[tempY][tempX].previousLoc = RIGHT;
                }
                tempX++;
            }

            else if (ledState[tempY][tempX].previousLoc == LEFT)
            {
                switchPoints(tempX - 1, tempY, tempX, tempY);
                if (ledState[tempY][tempX].previousLoc)
                {
                    ledState[tempY][tempX].previousLoc = LEFT;
                }
                tempX--;
            }

            else if (ledState[tempY][tempX].previousLoc == UP)
            {
                switchPoints(tempX, tempY - 1, tempX, tempY);
                if (ledState[tempY][tempX].previousLoc)
                {
                    ledState[tempY][tempX].previousLoc = UP;
                }
                tempY--;
            }

            else if (ledState[tempY][tempX].previousLoc == DOWN)
            {
                switchPoints(tempX, tempY + 1, tempX, tempY);
                if (ledState[tempY][tempX].previousLoc)
                {
                    ledState[tempY][tempX].previousLoc = DOWN;
                }
                tempY++;
            }
        }
    }

    else if (state == apple)
    {
        userX++;
        ledState[userY][userX - 1].state = snake;
        ledState[userY][userX].state = snake;
        ledState[userY][userX].previousLoc = LEFT;

        length++;
    }

    else
    {
        return true;
    }

    print_location(userY, userX);
    printLedStates();
    return false;
}

bool go_left(){

    if (userX == 0)
    {
        return false;
    }

    byte state = getState(userY, userX - 1);

    if (!state)
    {
        switchPoints(userX, userY, userX - 1, userY);
        byte tempY = userY, tempX = userX;
        userX--;
        if (ledState[userY][userX].previousLoc)
        {
            ledState[userY][userX].previousLoc = RIGHT;
        }

        for (byte i = 1; i < length; i++)
        {

            if (ledState[tempY][tempX].previousLoc == RIGHT)
            {
                switchPoints(tempX + 1, tempY, tempX, tempY);
                if (ledState[tempY][tempX].previousLoc)
                {
                    ledState[tempY][tempX].previousLoc = RIGHT;
                }
                tempX++;
            }

            else if (ledState[tempY][tempX].previousLoc == LEFT)
            {
                switchPoints(tempX - 1, tempY, tempX, tempY);
                if (ledState[tempY][tempX].previousLoc)
                {
                    ledState[tempY][tempX].previousLoc = LEFT;
                }
                tempX--;
            }

            else if (ledState[tempY][tempX].previousLoc == UP)
            {
                switchPoints(tempX, tempY - 1, tempX, tempY);
                if (ledState[tempY][tempX].previousLoc)
                {
                    ledState[tempY][tempX].previousLoc = UP;
                }
                tempY--;
            }

            else if (ledState[tempY][tempX].previousLoc == DOWN)
            {
                switchPoints(tempX, tempY + 1, tempX, tempY);
                if (ledState[tempY][tempX].previousLoc)
                {
                    ledState[tempY][tempX].previousLoc = DOWN;
                }
                tempY++;
            }
        }
    }
    else if (state == apple)
    {
        Serial.print("\nfwefegegvewgewbgewgew");
        userX--;
        ledState[userY][userX + 1].state = snake;
        ledState[userY][userX].state = snake;
        ledState[userY][userX].previousLoc = RIGHT;

        length++;
    }

    else
    {
        return true;
    }

    print_location(userY, userX);
    printLedStates();
    return false;
}

bool go_up()
{

    if (userY == 0)
    {
        return false;
    }

    byte state = getState(userY - 1, userX);

    if (!state)
    {
        switchPoints(userX, userY, userX, userY - 1);
        byte tempY = userY, tempX = userX;
        userY--;
        if (ledState[userY][userX].previousLoc)
        {
            ledState[userY][userX].previousLoc = DOWN;
        }

        for (byte i = 1; i < length; i++)
        {
            if (ledState[tempY][tempX].previousLoc == LEFT)
            {
                switchPoints(tempX - 1, tempY, tempX, tempY);
                if (ledState[tempY][tempX].previousLoc)
                {
                    ledState[tempY][tempX].previousLoc = LEFT;
                }
                tempX--;
            }

            else if (ledState[tempY][tempX].previousLoc == RIGHT)
            {
                switchPoints(tempX + 1, tempY, tempX, tempY);
                if (ledState[tempY][tempX].previousLoc)
                {
                    ledState[tempY][tempX].previousLoc = RIGHT;
                }
                tempX++;
            }

            else if (ledState[tempY][tempX].previousLoc == UP)
            {
                switchPoints(tempX, tempY - 1, tempX, tempY);
                if (ledState[tempY][tempX].previousLoc)
                {
                    ledState[tempY][tempX].previousLoc = UP;
                }
                tempY--;
            }

            else if (ledState[tempY][tempX].previousLoc == DOWN)
            {
                switchPoints(tempX, tempY + 1, tempX, tempY);
                if (ledState[tempY][tempX].previousLoc)
                {
                    ledState[tempY][tempX].previousLoc = DOWN;
                }
                tempY++;
            }
        }
    }
    else if (state == apple)
    {
        Serial.print("\nfwefegegvewgewbgewgew");
        userY--;
        ledState[userY + 1][userX].state = snake;
        ledState[userY][userX].state = snake;
        ledState[userY][userX].previousLoc = DOWN;

        length++;
    }

    else
    {
        return true;
    }

    print_location(userY, userX);
    printLedStates();
    return false;
}

bool go_down()
{
    if (userY == 7)
    {
        return false;
    }

    byte state = getState(userY + 1, userX);

    if (!state)
    {
        switchPoints(userX, userY, userX, userY + 1);
        byte tempY = userY, tempX = userX;
        userY++;
        if (ledState[userY][userX].previousLoc)
        {
            ledState[userY][userX].previousLoc = UP;
        }

        for (byte i = 1; i < length; i++)
        {

            if (ledState[tempY][tempX].previousLoc == LEFT)
            {
                switchPoints(tempX - 1, tempY, tempX, tempY);
                if (ledState[tempY][tempX].previousLoc)
                {
                    ledState[tempY][tempX].previousLoc = LEFT;
                }
                tempX--;
            }

            else if (ledState[tempY][tempX].previousLoc == RIGHT)
            {
                switchPoints(tempX + 1, tempY, tempX, tempY);
                if (ledState[tempY][tempX].previousLoc)
                {
                    ledState[tempY][tempX].previousLoc = RIGHT;
                }
                tempX++;
            }

            else if (ledState[tempY][tempX].previousLoc == UP)
            {
                switchPoints(tempX, tempY - 1, tempX, tempY);
                if (ledState[tempY][tempX].previousLoc)
                {
                    ledState[tempY][tempX].previousLoc = UP;
                }
                tempY--;
            }

            else if (ledState[tempY][tempX].previousLoc == DOWN)
            {
                switchPoints(tempX, tempY + 1, tempX, tempY);
                if (ledState[tempY][tempX].previousLoc)
                {
                    ledState[tempY][tempX].previousLoc = DOWN;
                }
                tempY++;
            }
        }
    }

    else if (state == apple)
    {
        Serial.print("\nfwefegegvewgewbgewgew");
        userY++;
        ledState[userY - 1][userX].state = snake;
        ledState[userY][userX].state = snake;
        ledState[userY][userX].previousLoc = UP;

        length++;
    }

    else
    {
        return true;
    }

    print_location(userY, userX);
    printLedStates();
    return false;
}

byte getState(byte userY, byte userX)
{
    return ledState[userY][userX].state;
}

void printGameOver()
{
    // Clear the LED matrix
    clearMatrix();

    // Define a simple "Game Over" pattern (each row is an 8-bit representation)
    byte G[8] = {
        B11111100, // Row 1
        B10000000, // Row 2
        B10000000, // Row 3
        B10011110, // Row 4
        B10000010, // Row 5
        B10000010, // Row 6
        B10000010, // Row 7
        B11111100  // Row 8
    };

    byte A[8] = {
        B01111100, // Row 1
        B10000010, // Row 2
        B10000010, // Row 3
        B11111110, // Row 4
        B10000010, // Row 5
        B10000010, // Row 6
        B10000010, // Row 7
        B10000010  // Row 8
    };

    byte M[8] = {
        B10000010, // Row 1
        B11000110, // Row 2
        B10101010, // Row 3
        B10010010, // Row 4
        B10000010, // Row 5
        B10000010, // Row 6
        B10000010, // Row 7
        B10000010  // Row 8
    };

    byte E[8] = {
        B11111100, // Row 1
        B10000000, // Row 2
        B10000000, // Row 3
        B11111100, // Row 4
        B10000000, // Row 5
        B10000000, // Row 6
        B10000000, // Row 7
        B11111100  // Row 8
    };

    byte O[8] = {
        B01111100, // Row 1
        B10000010, // Row 2
        B10000010, // Row 3
        B10000010, // Row 4
        B10000010, // Row 5
        B10000010, // Row 6
        B10000010, // Row 7
        B01111100  // Row 8
    };

    byte V[8] = {
        B10000010, // Row 1
        B10000010, // Row 2
        B10000010, // Row 3
        B10000010, // Row 4
        B10000010, // Row 5
        B10000010, // Row 6
        B01000100, // Row 7
        B00101000  // Row 8
    };

    byte R[8] = {
        B11111100, // Row 1
        B10000010, // Row 2
        B10000010, // Row 3
        B11111100, // Row 4
        B10010000, // Row 5
        B10001000, // Row 6
        B10000100, // Row 7
        B10000010  // Row 8
    };

    clearMatrix();

    // Display "GAME OVER" letter by letter
    displayLetter(G, 0); // 'G'
    delay(500);          // Wait for half a second
    displayLetter(A, 1); // 'A'
    delay(500);
    displayLetter(M, 2); // 'M'
    delay(500);
    displayLetter(E, 3); // 'E'
    delay(500);
    displayLetter(O, 4); // 'O'
    delay(500);
    displayLetter(V, 5); // 'V'
    delay(500);
    displayLetter(E, 6); // 'E'
    delay(500);
    displayLetter(R, 7); // 'R'
    delay(2000);         // Keep the message for a while
}

void displayLetter(byte letter[8], int matrixColumn)
{
    for (byte i = 0; i < 8; i++)
    {
        lc.setRow(0, i, letter[i]); // Display the letter
    }
}

void clearMatrix() {
    for (byte row = 0; row < 8; row++)
    {
        for (byte col = 0; col < 8; col++)
        {
            lc.setLed(0, row, col, false);
        }
    }
}

void print_location(byte userY, byte userX){
    Serial.print("( ");
    Serial.print(userY);
    Serial.print(", ");
    Serial.print(userX);
    Serial.println(" )");
}


// function that return the current joystick state
byte get_Direction(){
    int XY[2]; // array holding X, Y axis
    get_xy(XY);
    int x = XY[0], y = XY[1];

    if (isPaused)
    {
        // Serial.println("PAUSE");
        return PAUSE;
    }

    else if (abs(x - 180) > move_range || abs(y - 180) > move_range)
    {
        if (abs(x - 180) > abs(y - 180))
        {
            if (x > 180)
            {
                // Serial.println("LEFT");
                return LEFT;
            }
            else
            {
                // Serial.println("RIGHT");
                return RIGHT;
            }
        }
        else
        {
            if (y > 180)
            {
                // Serial.println("UP");
                return UP;
            }
            else
            {
                // Serial.println("DOWN");
                return DOWN;
            }
        }
    }
    else
    {
        return STABLE;
    }
}