#include "JoystickControl.h"
const int move_range = 50;


void setupJoystick(int xaxis, int yaxis, int sw) {
  pinMode(xaxis, INPUT);
  pinMode(yaxis, INPUT);
  pinMode(sw, INPUT_PULLUP);
  Serial.begin(9600);
}

void get_xy(int *XY) {
  int X = analogRead(xaxis);
  int Y = analogRead(yaxis);

  XY[0] = map(X, 0, 1023, 0, 360);
  XY[1] = map(Y, 0, 1023, 0, 360);
}
