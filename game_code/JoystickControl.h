#ifndef JoystickControl_h
#define JoystickControl_h

#include <Arduino.h>

extern const int xaxis;      
extern const int yaxis;      
extern const int sw;        
extern const int move_range; 

void setupJoystick(int, int, int);
void get_xy(int *XY);

#endif