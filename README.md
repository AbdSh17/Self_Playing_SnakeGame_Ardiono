
# Self-Playing Snake Game – Arduino

Self-playing **Snake game** built with **Arduino UNO** using a **MAX7219 8x8 LED matrix**, powered by the **Breadth First Search (BFS)** algorithm to determine the next move.

---

## Table of Contents
- [What's in the Repo?](#whats-in-the-repo)
- [Game Demo](#game-demo)
- [About the Game](#about-the-game)
- [Snake Representation & Algorithm](#snake-representation--algorithm)
- [Components Required](#components-required)
- [Setup & Wiring](#setup--wiring)
- [Code & Installation](#code--installation)

---

## What's in the Repo?
- **Game Code** (Arduino Sketch)
- **Example Video** (Gameplay)
- **Circuit Photo** (Wiring Diagram)

---

## Game Demo
**[Add demo video link or embed here]**

---

## About the Game
This is a **classic arcade Snake game** controlled by the **Breadth First Search (BFS)** algorithm. The snake continues playing until it fully grows or gets trapped in a local path.

This game was developed as an experiment to observe how **BFS** performs in finding the shortest path, and how it impacts gameplay speed and memory usage.

---

## Snake Representation & Algorithm

### Snake Representation
The matrix is represented using an 8x8 struct:
```cpp
struct leds {
    byte state;         // can be snake, apple, or none
    byte previousLoc;   // previous node in the snake’s path
    bool isVisited;     // used by BFS algorithm
};
```

### Algorithm
As mentioned earlier, **BFS** is used to find the snake’s next move. It follows the path determined by BFS. If a future **collision** is detected, the snake reroutes to a safe block and recalculates a new path.

---

## Components Required
- **Arduino UNO**
- **MAX7219 8x8 LED Matrix**
- **Jumper Wires**

---

## Setup & Wiring
| Arduino UNO Pin | MAX7219 Pin     |
|------------------|-----------------|
| 5V               | VCC             |
| GND              | GND             |
| 10               | DIN             |
| 11               | CLK             |
| 12               | CS              |

---

## Code & Installation
1. **Clone this repository**
```sh
git clone https://github.com/AbdSh17/Self_Playing_SnakeGame_Ardiono
```
2. Open the `.ino` file in the Arduino IDE and upload it to your Arduino UNO.
