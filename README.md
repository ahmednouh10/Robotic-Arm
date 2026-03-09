
# 3-DOF Robotic Arm – Manual & Teaching Mode (Arduino + Bluetooth)

## Overview

This project implements a **3-DOF robotic arm controlled using Arduino and servo motors**.
The system supports **two operation modes**:

1. **Real-Time Control Mode (Manual Mode)** – Control each servo motor directly via **Bluetooth serial commands**.
2. **Teaching / Record Mode** – Record a sequence of movements step-by-step and replay them continuously to perform **repetitive tasks**.

This approach is commonly used in **industrial robotic teaching systems**, where a user manually moves the robot once and then the robot repeats the task automatically.

---

## Features

* Real-time servo control via **Bluetooth serial communication**
* **Motor selection system** to control each joint independently
* **Two operating modes**

  * Manual control
  * Movement recording & playback
* **Path recording** with delay timing between steps
* **Smooth servo motion interpolation**
* **Looped playback** for repetitive operations
* Up to **50 recorded movement steps**
* Button-controlled **record and playback modes**

---

## Hardware Requirements

* Arduino (Uno / Nano / Mega)
* 3-DOF or 4-servo robotic arm
* 3–4 Servo Motors
* HC-05 / HC-06 Bluetooth Module
* Push Button ×2
* External 5V power supply for servos
* Jumper wires

---

## Pin Configuration

| Component       | Arduino Pin |
| --------------- | ----------- |
| Servo Motor 1   | 5           |
| Servo Motor 2   | 3           |
| Servo Motor 3   | 9           |
| Servo Motor 4   | 10          |
| Playback Button | 4           |
| Record Button   | 6           |

Buttons use **INPUT_PULLUP** configuration.

---

## Operating Modes

### 1. Manual Control Mode (Bluetooth)

In this mode, motors are controlled in **real-time** through Bluetooth serial commands.

First select the motor, then move it.

#### Motor Selection

| Command | Action         |
| ------- | -------------- |
| `1`     | Select Motor 1 |
| `2`     | Select Motor 2 |
| `3`     | Select Motor 3 |
| `4`     | Select Motor 4 |

#### Motor Movement

| Command | Action                       |
| ------- | ---------------------------- |
| `F`     | Move motor forward (+5°)     |
| `B`     | Move motor backward (-5°)    |
| `+`     | Fine forward movement (+1°)  |
| `-`     | Fine backward movement (-1°) |

---

### 2. Teaching / Recording Mode

This mode allows the robot to **learn a motion path** and repeat it.

#### Recording Steps

1. Press the **Record Button** to enter recording mode.
2. Move the robotic arm using Bluetooth commands.
3. Press the **Record Button again** to save the current position.
4. Repeat until the full path is recorded.

Each recorded step stores:

* Servo positions
* Time delay between movements

Maximum stored steps = **50 movements**

---

### Playback Mode

1. Press the **Playback Button**.
2. The robot will move through the recorded steps.
3. Movements are performed with **smooth interpolation**.
4. After the last step, the sequence **loops continuously**.

To stop playback:

Double press the **Playback Button**.

---

## Motion Interpolation

To achieve smooth robotic motion, the system gradually moves between recorded positions over **500 ms** using linear interpolation:

```
currentPosition = startPosition + (targetPosition - startPosition) * progress
```

This avoids sudden servo jumps and produces natural arm motion.

---

## Data Structure Used

Each movement step is stored as:

```
struct Movement {
  byte motor1;
  byte motor2;
  byte motor3;
  byte motor4;
  unsigned long delayTime;
};
```

This structure records both **position and timing**, allowing accurate reproduction of the original motion.

---

## Example Applications

* Repetitive pick-and-place tasks
* Agricultural robotic spraying arms
* Educational robotics experiments
* Robotic manipulation research
* Industrial teaching systems

---

## Future Improvements

* Mobile app for Bluetooth control
* Inverse kinematics support
* ROS2 integration
* Computer vision object detection
* EEPROM memory storage for permanent paths


