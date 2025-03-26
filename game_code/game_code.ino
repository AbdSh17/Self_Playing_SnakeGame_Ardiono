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
int freeMemory()
{
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

int count = 0;
bool collegianAvoidanceFlag = false;
byte direction = STABLE;

void restart()
{
    clearMatrix();
    freeStack(pathStack);
    appleCount = 0;
    count = 0;
    collegianAvoidanceFlag = false;
    direction = STABLE;
    setLedStates();
    userX = userY = 0;
    lc.setLed(0, userY, userX, true);
    ledState[userY][userX].state = snake;
}


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

        direction = getDirection();
        
        static byte lastDirection = false;

        bool lost = false;

        // if we reached the path, waiting for new apple to respone
        if (direction == STABLE)
        {
            printStack(pathStack);
            Serial.print("Apple Count: ");
            Serial.println(appleCount);
            printLedStates();

            direction = checkSurroundings(apple);

            if(direction != STABLE)
            {
              appleCount--;
            }

            // if there's an apple exist and our state is stable, then move your ass and go find it
            if (appleCount)
            {
                getBFSPath();
            }
        }

        if (direction == RIGHT)
        {
            if (ledState[userY][userX + 1].state == snake)
            {
              collegianAvoidanceFlag = true;
              // if there's no problem with up or down, go againts snake movment
              if(userY > 0 && userY < 7 && ledState[userY + 1][userX].state != snake && ledState[userY - 1][userX].state != snake)
              {
                direction = ledState[userY][userX + 1].previousLoc;
                if (direction == UP)
                {
                  lost = go_down();
                }
                else
                {
                  lost = go_up();
                }
              }
              
              // RIGHT
              else if (userY < 7 && ledState[userY + 1][userX].state != snake)
              {
                lost = go_down();
              }

              // LEFT
              else if(userY > 0 && ledState[userY - 1][userX].state != snake)
              {
                lost = go_up();
              }

              else
              {
                lost = go_left();
              }

              getBFSPath();
              count++;
            }
            else
            {
              collegianAvoidanceFlag = false;
              lastDirection = RIGHT;
              lost = go_right();
              count = 0;
            }
        }

        else if (direction == LEFT)
        {
          if (ledState[userY][userX - 1].state == snake)
            {
              collegianAvoidanceFlag = true;
              // if there's no problem with up or down, go againts snake movment
              if(userY > 0 && userY < 7 && ledState[userY + 1][userX].state != snake && ledState[userY - 1][userX].state != snake)
              {
                direction = ledState[userY][userX - 1].previousLoc;
                if (direction == UP)
                {
                  lost = go_down();
                }
                else
                {
                  lost = go_up();
                }
              }
              
              // RIGHT
              else if (userY < 7 && ledState[userY + 1][userX].state != snake)
              {
                // direction = DOWN;
                lost = go_down();
              }

              // LEFT
              else if(userY > 0 && ledState[userY - 1][userX].state != snake)
              {
                // direction = UP;
                lost = go_up();
              }

              else
              {
                lost = go_right();
              }
              getBFSPath();
              count++;
            }
            else
            {
              collegianAvoidanceFlag = false;
              lastDirection = LEFT;
              lost = go_left();
              count = 0;
            }
        }

        else if (direction == UP)
        {
            if (ledState[userY - 1][userX].state == snake)
            {
              collegianAvoidanceFlag = true;
              // if there's no problem with left or right, go againts snake movment
              if(userX > 0 && userX < 7 && ledState[userY][userX + 1].state != snake && ledState[userY][userX - 1].state != snake)
              {
                direction = ledState[userY - 1][userX].previousLoc;
                if (direction == RIGHT)
                {
                  lost = go_right();
                }
                else
                {
                  lost = go_left();
                }
              }
              
              // RIGHT
              else if (userX < 7 && ledState[userY][userX + 1].state != snake)
              {
                // direction = RIGHT;
                lost = go_right();
              }

              // LEFT
              else if(userX > 0 && ledState[userY][userX - 1].state != snake)
              {
                // direction = LEFT;
                lost = go_left();
              }

              else
              {
                lost = go_down();
              }
              getBFSPath();
              count++;
            }
            else
            {
              collegianAvoidanceFlag = false;
              lastDirection = UP;
              lost = go_up();
              count = 0;
            }
        }

        else if (direction == DOWN)
        {
            if (ledState[userY + 1][userX].state == snake)
            {
              collegianAvoidanceFlag = true;
              // if there's no problem with left or right, go againts snake movment
              if(userX > 0 && userX < 7 && ledState[userY][userX + 1].state != snake && ledState[userY][userX - 1].state != snake)
              {
                direction = ledState[userY + 1][userX].previousLoc;
                if (direction == RIGHT)
                {
                  lost = go_right();
                }
                else
                {
                  lost = go_left();
                }
              }

              // RIGHT
              else if (userX < 7 && ledState[userY][userX + 1].state != snake)
              {
                // direction = RIGHT;
                lost = go_right();
              }

              // LEFT
              else if(userX > 0 && ledState[userY][userX - 1].state != snake)
              {
                // direction = LEFT;
                lost = go_left();
              }
              
              else
              {
                lost = go_up();
              }

              getBFSPath();
              count++;
            }
            else 
            {
              collegianAvoidanceFlag = false;
              lastDirection = DOWN;
              lost = go_down();
              count = 0;
            }
        }

        if (lost || count == 20)
        {
          printGameOver();
          restart();
        }
    }
}

void set_apple()
{
    byte row = 1, column = 0;
    randomSeed(50); // randomSeed(millis());

    while (1)
    {
        row = random(0, 8);
        column = random(0, 8);
        if (!ledState[row][column].state) // while the random state is not an apple or snake (just none)
            break;
    }

    lc.setLed(0, row, column, true);
    ledState[row][column].state = apple;
}

byte getDirection()
{
    byte head[2]; // an array will be modified to get head[1], head[0] = Y,X

    if (isEmptyStack(pathStack)) // if the stack is empty, then we reached the path previously
    {
        return STABLE;
    }

    getTop(pathStack, head); // head[1], head[0] = Y,X
    pop(pathStack);

    /*
      EX)
      CURRENT: 0,0 - HEAD: 0,1
      userX > X - userY == Y, then move right
    */
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
    return STABLE; // if there's an error and everything equal;
}

// function to fill the global stack (pat stack) with the path
void getBFSPath()
{
    byte path[2];                      // An array will be sent with the function and modified inside the function;
    bfs(path, 9, 9);                   // bfs, modify the path, make the goal X,Y is 9,9 (out of range) so it will only stop when find an apple
    freeStack(pathStack);              // empty the global stack so we erfil again with new paths
    push(pathStack, path[0], path[1]); // push first path, the point right before the goal (apple)

    Serial.print("GETBFSFUNCTION");
    printStack(pathStack);

    // while the path is not the current place i'm in, keep gives the new paths
    /*
      EX)
      GOAL: 3,3 - LOCATION: 0,0
      First iteration path[0], path[1] = 3,2 (goal = 3,3)
      Second iteration path[0], path[1] = 3,1 (goal = 3,2)
      Third iteration path[0], path[1] = 3,0 (goal = 3,1)
      pla pla pla.......
    */

    while (!(path[0] == userY && path[1] == userX))
    {
        bfs(path, path[0], path[1]);
        push(pathStack, path[0], path[1]); // keep pushing the new path to the stack
    }
    pop(pathStack); // pop the root (0,0 in the previous example)
}

// breadth first path, algorithim to find the shortest path
void bfs(byte *path, byte yAxis, byte xAxis) {
    queueHeader q = initializeQueue();
    enQueue(q, userY, userX);                // add my Current location to the queue
    ledState[userY][userX].isVisited = true; // set my current location as visited state
    byte head[2] = {0, 0};                   // initalize empty head (X and Y)
    int count = 0;                           // to handle if got in the loop more than 64 (which is more than the 8X8 matrix)

    while (head[0] != 255)
    {
        GetQueueHead(q, head);         // set head[0] = Y, head[1] = X;
        byte x = head[1], y = head[0]; // for more readability

        // DOWN state, (y + 1) is bellow me
        if (y < 7 && !ledState[y + 1][x].isVisited) // 1- Not at the edge, 2- not visited
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
        if (y > 0 && !ledState[y - 1][x].isVisited) // 1- Not at the edge, 2- not visited
        {
            if (ledState[y - 1][x].state == apple || (y - 1 == yAxis && x == xAxis)) // 1- if it's an apple, 2- if it's the goal (sent by a fuction, not an apple but to do itterative bfs)
            {
                path[0] = y; // will be modified with the function
                path[1] = x; // will be modified with the function
                clearVisitedStates();
                freeQueue(q);
                return;
            }
            ledState[y - 1][x].isVisited = true;
            enQueue(q, y - 1, x);
        }

        // RIGHT
        if (x < 7 && !ledState[y][x + 1].isVisited) // 1- Not at the edge, 2- not visited
        {
            if (ledState[y][x + 1].state == apple || (x + 1 == xAxis && y == yAxis)) // 1- if it's an apple, 2- if it's the goal (sent by a fuction, not an apple but to do itterative bfs)
            {
                path[0] = y; // will be modified with the function
                path[1] = x; // will be modified with the function
                clearVisitedStates();
                freeQueue(q);
                return;
            }
            ledState[y][x + 1].isVisited = true;
            enQueue(q, y, x + 1); // if new and not the goal, add to the fringe
        }

        // LEFT
        if (x > 0 && !ledState[y][x - 1].isVisited) // 1- Not at the edge, 2- not visited
        {
            if (ledState[y][x - 1].state == apple || (x - 1 == xAxis && y == yAxis)) // 1- if it's an apple, 2- if it's the goal (sent by a fuction, not an apple but to do itterative bfs)
            {
                path[0] = y; // will be modified with the function
                path[1] = x; // will be modified with the function
                clearVisitedStates();
                freeQueue(q);
                return;
            }
            ledState[y][x - 1].isVisited = true;
            enQueue(q, y, x - 1);
        }

        count++;
        if (count == 64) // 8X8 Limit is 64 itteration;
        {
            Serial.print("loop");
            break;
        }
        dequeue(q);
    }
    clearVisitedStates();
    freeQueue(q);
}

byte checkSurroundingsForNotState(byte state)
{
    if (userY < 7 && ledState[userY + 1][userX].state != state) // 1- if not edge, 2- is an apple
    {
        return DOWN;
    }

    // UP
    else if (userY > 0 && ledState[userY - 1][userX].state != state) // 1- if not edge, 2- is an apple
    {
        return UP;
    }

    // LEFT
    else if (userX > 0 && ledState[userY][userX - 1].state != state) // 1- if not edge, 2- is an apple
    {
        return LEFT;
    }

    // RIGHT
    else if (userX < 7 && ledState[userY][userX + 1].state != state) // 1- if not edge, 2- is an apple
    {
        return RIGHT;
    }
    return STABLE;
}

byte checkSurroundingsWithDirection(byte state, byte direction)
{
    // DOWN
    if (userY < 7 && ledState[userY + 1][userX].state == state && direction == DOWN) // 1- if not edge, 2- is an apple
    {
        return DOWN;
    }

    // UP
    else if (userY > 0 && ledState[userY - 1][userX].state == state && direction == UP) // 1- if not edge, 2- is an apple
    {
        return UP;
    }

    // LEFT
    else if (userX > 0 && ledState[userY][userX - 1].state == state && direction == LEFT) // 1- if not edge, 2- is an apple
    {
        return LEFT;
    }

    // RIGHT
    else if (userX < 7 && ledState[userY][userX + 1].state == state && direction == RIGHT) // 1- if not edge, 2- is an apple
    {
        return RIGHT;
    }
    return STABLE;
}


byte checkSurroundings(byte state)
{
    if (userY < 7 && ledState[userY + 1][userX].state == state) // 1- if not edge, 2- is an apple
    {
        return DOWN;
    }

    // UP
    else if (userY > 0 && ledState[userY - 1][userX].state == state) // 1- if not edge, 2- is an apple
    {
        return UP;
    }

    // LEFT
    else if (userX > 0 && ledState[userY][userX - 1].state == state) // 1- if not edge, 2- is an apple
    {
        return LEFT;
    }

    // RIGHT
    else if (userX < 7 && ledState[userY][userX + 1].state == state) // 1- if not edge, 2- is an apple
    {
        return RIGHT;
    }
    return STABLE;
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