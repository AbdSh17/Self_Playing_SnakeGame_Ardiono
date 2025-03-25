// =============================== INCLUDE ===============================
#include "JoystickControl.h"
#include "queue.h"
#include "stack.h"
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

const long appleInterval = 1000;
const long moveInterval = 50;
// =============================== CONST ===============================

// =============================== GLOBAL ===============================

volatile bool isPaused = false;

byte userX = 0, userY = 0;
byte length = 1;
byte userDirection = STABLE;

byte appleCount = 1;

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

extern int __heap_start, *__brkval;
int freeMemory() {
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

// Switch interrupt
void handleSwitch()
{
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

void setLedStates()
{
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

void clearVisitedStates()
{
    for (byte i = 0; i < ROWS; i++)
    {
        for (byte j = 0; j < COLUMNS; j++)
        {
            ledState[i][j].isVisited = false;
        }
    }
}

void printLedStates()
{
    Serial.println("=================================");
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLUMNS; j++)
        { // nextStep
            Serial.print(ledState[i][j].state);
            Serial.print(" ");
        }
        Serial.println("");
    }
    Serial.println("=================================\n");
}

stackHeader pathStack = initializeStackHeadder();

void setup()
{
    setupJoystick(A0, A1, sw);
    attachInterrupt(digitalPinToInterrupt(sw), handleSwitch, FALLING); // interrupt for handling switch pressing ;

    lc.shutdown(0, false); // Turn on the MAX7219
    lc.setIntensity(0, 8); // Set brightness (0 to 15)
    lc.clearDisplay(0);    // Clear the display

    setLedStates();
    ledState[3][4].state = apple;
    lc.setLed(0, 3, 4, true);

    // ledState[7][1].state = apple;
    // lc.setLed(0, 7, 1, true);

    // ledState[1][2].state = apple;
    // lc.setLed(0, 1, 2, true);

    // ledState[5][7].state = apple;
    // lc.setLed(0, 5, 7, true);

    // ledState[4][4].state = apple;
    // lc.setLed(0, 4, 4, true);

    // ledState[6][6].state = apple;
    // lc.setLed(0, 6, 6, true);

    // ledState[0][2].state = apple;
    // lc.setLed(0, 0, 2, true);

    lc.setLed(0, userY, userX, true);
    ledState[userY][userX].state = snake;

    Serial.begin(9600);
    delay(1000);
    getBFSPath();
    printStack(pathStack);
}

void restart()
{
    clearMatrix();
    freeStack(pathStack);
    appleCount = 0;
    setLedStates();
    userX = userY = 0;
    lc.setLed(0, userY, userX, true);
    ledState[userY][userX].state = snake;
}

int count = 0;
void loop()
{

    static unsigned long previousMillisApple = 0;
    static unsigned long previousMillisMove = 0;

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillisApple >= appleInterval)
    {
        previousMillisApple = currentMillis;
        set_apple();
        appleCount++;
    }

    if (currentMillis - previousMillisMove >= moveInterval)
    {
        previousMillisMove = currentMillis;

        byte direction = getDirection();
        static byte lastDirection = direction;

        bool lost = false;

        if (direction == STABLE)
        {
            Serial.print("AppleCount: ");
            Serial.print(appleCount);
            printStack(pathStack);
            printLedStates();

            // if (userX < 8 && ledState[userY][userX + 1].state != snake)
            // {
            //     direction = RIGHT;
            // }

            // else if (userY >= 0 && ledState[userY - 1][userX].state != snake)
            // {
            //     direction = UP;
            // }

            // else if (userY < 8 && ledState[userY + 1][userX].state != snake)
            // {
            //     direction = DOWN;
            // }

            // else if (userX >= 0 && ledState[userY][userX - 1].state != snake)
            // {
            //     Serial.println("===LEFTTTTT===");
            //     direction = LEFT;
            // }

            // ============================
            if (userY < 7 && ledState[userY + 1][userX].state == apple)
            {
                direction = DOWN;
                appleCount--;
            }

            else if (userY > 0 && ledState[userY - 1][userX].state == apple)
            {
                direction = UP;
                appleCount--;
            }

            else if (userX > 0 && ledState[userY][userX - 1].state == apple)
            {
                direction = LEFT;
                appleCount--;
            }

            else if (userX < 7 && ledState[userY][userX + 1].state == apple)
            {
                direction = RIGHT;
                appleCount--;
            }

            if(appleCount)
            {
              getBFSPath();
            }

            Serial.println(freeMemory());

        }

        if (direction == RIGHT)
        {
            lastDirection = RIGHT;
            lost = go_right();
        }

        else if (direction == LEFT)
        {
            lastDirection = LEFT;
            lost = go_left();
            // if(appleCount)
            //   Serial.println("===LEFTTTTT===");
        }

        else if (direction == UP)
        {
            lastDirection = UP;
            lost = go_up();
        }

        else if (direction == DOWN)
        {
            lastDirection = DOWN;
            lost = go_down();
        }

        if (lost)
        {
            printGameOver();
            restart();
        }
    }
}

void set_apple()
{
    byte row = 1, column = 0;
    randomSeed(40); // randomSeed(millis());

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

byte getDirection()
{
    byte head[2];

    if (isEmptyStack(pathStack))
    {
        return STABLE;
    }

    getTop(pathStack, head);
    pop(pathStack);
    if (userY == head[0])
    {
        if (userX < head[1])
            return RIGHT;
        else
            return LEFT;
    }
    else
    {
        if (userY < head[0])
            return DOWN;
        else
            return UP;
    }
    return STABLE;
}

void getBFSPath()
{
    byte path[2];
    bfs(path, 9, 9);
    freeStack(pathStack);
    push(pathStack, path[0], path[1]);
    Serial.print("GETBFSFUNCTION");
    printStack(pathStack);

    while (!(path[0] == userY && path[1] == userX))
    {
        bfs(path, path[0], path[1]);
        push(pathStack, path[0], path[1]);
    }
    pop(pathStack);
}

void bfs(byte *path, byte yAxis, byte xAxis)
{
    queueHeader q = initializeQueue();
    enQueue(q, userY, userX); // add my Current location to the queue
    ledState[userY][userX].isVisited = true; // set my current location as visited state
    byte head[2] = {0, 0}; // initalize empty head (X and Y)
    int count = 0; // to handle if got in the loop more than 64 (which is more than the 8X8 matrix)

    while (head[0] != 255)
    {
        GetQueueHead(q, head); // set head[0] = Y, head[1] = X;
        byte x = head[1], y = head[0]; // for more readability

        // DOWN state, (y + 1) is bellow me
        if (y < 7 && !ledState[y + 1][x].isVisited)  // 1- Not at the edge, 2- not visited
        {
            if (ledState[y + 1][x].state == apple || (y + 1 == yAxis && x == xAxis)) // 1- if it's an apple, 2- if it's the goal (sent by a fuction, not an apple but to do itterative bfs)
            {
                path[0] = y; // will be modified with the function
                path[1] = x; // will be modified with the function
                clearVisitedStates();
                freeQueue(q);
                return;
            }
            // if didn't find the path, then set the node as visited, add it to the queue
            ledState[y + 1][x].isVisited = true;
            enQueue(q, y + 1, x);
        }

        // UP
        if (y > 0 && !ledState[y - 1][x].isVisited)
        { // 1- Not at the edge, 2- not facing a snake, 3- not visited
            if (ledState[y - 1][x].state == apple || (y - 1 == yAxis && x == xAxis)) // 1- if it's an apple, 2- if it's the goal (not an apple but to do itterative bfs)
            {
                path[0] = y;
                path[1] = x;
                clearVisitedStates();
                freeQueue(q);
                return;
            }
            ledState[y - 1][x].isVisited = true;
            enQueue(q, y - 1, x);
        }

        // RIGHT
        if (x < 7 && !ledState[y][x + 1].isVisited)
        { // 1- Not at the edge, 2- not facing a snake, 3- not visited
            if (ledState[y][x + 1].state == apple || (x + 1 == xAxis && y == yAxis)) // 1- if it's an apple, 2- if it's the goal (not an apple but to do itterative bfs)
            {
                path[0] = y;
                path[1] = x;
                clearVisitedStates();
                freeQueue(q);
                return;
            }
            ledState[y][x + 1].isVisited = true;
            enQueue(q, y, x + 1);
        }

        // LEFT
        if (x > 0 && !ledState[y][x - 1].isVisited)
        { // 1- Not at the edge, 2- not facing a snake, 3- not visited
            if (ledState[y][x - 1].state == apple || (x - 1 == xAxis && y == yAxis)) // 1- if it's an apple, 2- if it's the goal (not an apple but to do itterative bfs)
            {
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
        if (count == 64)
        {
            Serial.print("loop");
            break;
        }
        dequeue(q);
    }
    clearVisitedStates();
    freeQueue(q);
}

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
        // getBFSPath();
        length++;
    }

    else
    {
        return true;
    }

    return false;
}

bool go_left()
{
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
        userX--;
        ledState[userY][userX + 1].state = snake;
        ledState[userY][userX].state = snake;
        ledState[userY][userX].previousLoc = RIGHT;
        // getBFSPath();
        length++;
    }

    else
    {
        return true;
    }

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
        userY--;
        ledState[userY + 1][userX].state = snake;
        ledState[userY][userX].state = snake;
        ledState[userY][userX].previousLoc = DOWN;
        // getBFSPath();
        length++;
    }

    else
    {
        return true;
    }

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
        userY++;
        ledState[userY - 1][userX].state = snake;
        ledState[userY][userX].state = snake;
        ledState[userY][userX].previousLoc = UP;
        // getBFSPath();
        length++;
    }

    else
    {
        return true;
    }

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

void clearMatrix()
{

    for (byte row = 0; row < 8; row++)
    {
        for (byte col = 0; col < 8; col++)
        {
            lc.setLed(0, row, col, false);
        }
    }
}

void print_location(byte userY, byte userX)
{
    Serial.print("( ");
    Serial.print(userY);
    Serial.print(", ");
    Serial.print(userX);
    Serial.println(" )");
}

// function that return the current joystick state
byte get_Direction()
{
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