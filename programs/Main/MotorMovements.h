#pragma once

#include "Distance.h"

void motor_setup() {
  motors.initialize();
  // falls man global die Motor-Drehrichtung ändern möchte:
  motors.flipLeftMotor(false); // nur notwendig, wenn man true reinschreibt
  motors.flipRightMotor(true); // nur notwendig, wenn man true reinschreibt
}

void stop()
{
  motors.setSpeeds(0, 0);
}

void straight(float speed = 1) //drive straight
{
  if (digitalRead(motorPin)) {
    stop();
    bigState = STOP;
    return;
  }
  motors.flipLeftMotor(false);
  motors.flipRightMotor(true);
  motors.setSpeeds((int)(42 * speed),(int)(50 * speed)); //prevent motor drifting
}

// Moves the robot according to the angle. 
// IMPORTANTLY the robot won't move BY the angle, it should move right or left with a specific speed
// The idea would be that since the image data is "continuous", the angle will be recalculated quickly
// and the new speed overrides the old one
void move_as_angle(int angle) {
  // TODO aaaaaaa is that even a good idea?
}

void left(int turnBy=0, float speed = 1) //turn left
{
  stop();
  if (digitalRead(motorPin)) {
    bigState = STOP;
    return;
  }
  readDirection();
  int initialDirection = direction;
  motors.flipLeftMotor(true);
  motors.flipRightMotor(true);
  motors.setSpeeds((int)(70 * speed), (int)(75 * speed));
  if (turnBy!=0) {
    while ((((initialDirection - turnBy) + 360) % 360) != direction) {
      delay(1);
      readDirection();
      if (digitalRead(motorPin)) {
        stop();
        bigState = STOP;
        return;
      }
    }
    stop();
  }
}

void right(int turnBy=0, float speed = 1) //turn right
{
  stop();
  if (digitalRead(motorPin)) {
    bigState = STOP;
    return;
  }
  readDirection();
  int initialDirection = direction;
  motors.flipLeftMotor(false);
  motors.flipRightMotor(false);
  motors.setSpeeds(70 * speed, 75 * speed);
  if (turnBy != 0) {
    while (((initialDirection + turnBy) % 360) != direction) {
      delay(1);
      readDirection();
      if (digitalRead(motorPin)) {
        stop();
        bigState = STOP;
        return;
      }
    }
    stop();
  }
}

void straight_left(float speed = 1) //drive straight but pull left
{
  if (digitalRead(motorPin)) {
    stop();
    bigState = STOP;
    return;
  }
  // Configuration for left
  motors.flipLeftMotor(true);
  motors.flipRightMotor(true);
  motors.setSpeeds((int)(40 * speed), (int)(100 * speed));
}

void straight_right(float speed = 1) //drive straight but pull right
{
  if (digitalRead(motorPin)) {
    stop();
    bigState = STOP;
    return;
  }
  // Configuration for right
  motors.flipLeftMotor(false);
  motors.flipRightMotor(false);
  motors.setSpeeds((int)(100 * speed), (int)(40 * speed));
}


void left_to_line(float speed = 1, int turnBy = 70) {
  // // going left until it finds a line  
  // if (digitalRead(motorPin)) {
  //   stop();
  //   bigState = STOP;
  //   return;
  // }
  // readDirection();
  // int initialDirection = direction;
  // left(0, speed);
  // while ((calculatedReflection = calculateReflection()) != normalLine) {
  //   delay(10);
  //   readDirection();
  //   if (calculatedReflection == leftLine) {
  //     straight_left();
  //     break;
  //   } else if (calculatedReflection == rightLine) {
  //     straight_right();
  //     break;
  //   } else if (calculatedReflection == sideLeftLine) {
  //     state = left_side;
  //     break;
  //   } else if (calculatedReflection == sideRightLine) {
  //     state = right_side;
  //     break;
  //   }
  //   if ((((initialDirection - turnBy) + 360) % 360) == direction) {
  //     break;
  //   }

  //   if (digitalRead(motorPin)) {
  //   stop();
  //   bigState = STOP;
  //   return;
  // }
  // }
  // state = straight_driving;
}

void right_to_line(float speed = 1, int turnBy = 70) {
  // // going right until it finds a line  
  // if (digitalRead(motorPin)) {
  //   stop();
  //   bigState = STOP;
  //   return;
  // }
  // readDirection();
  // int initialDirection = direction;
  // right(0, speed);
  // while ((calculatedReflection = calculateReflection()) != normalLine) {
  //   delay(10);
  //   readDirection();
  //   if (calculatedReflection == leftLine) {
  //     straight_left();
  //     break;
  //   } else if (calculatedReflection == rightLine) {
  //     straight_right();
  //     break;
  //   }
  //   else if (calculatedReflection == sideLeftLine) {
  //     state = left_side;
  //     break;
  //   } else if (calculatedReflection == sideRightLine) {
  //     state = right_side;
  //     break;
  //   }
  //   if (((initialDirection + turnBy) % 360) == direction) {
  //     break;
  //   }

  //   if (digitalRead(motorPin)) {
  //   stop();
  //   bigState = STOP;
  //   return;
  // }
  // }
  // state = straight_driving;
}

void abstand_umfahren() {
  // digitalWrite(LED_BUILTIN, HIGH);
  // if (digitalRead(motorPin)) {
  //   stop();
  //   bigState = STOP;
  //   return;
  //   }

  // straight(-1);
  // while (distance_val < 90) {
  //   readDistance();
  //   delay(10);
  // }
  // stop();


  // right();
  // if (digitalRead(motorPin)) {
  //   stop();
  //   bigState = STOP;
  //   return;
  // }
  // delay(2000);

  // straight();
  // if (digitalRead(motorPin)) {
  //   stop();
  //   bigState = STOP;
  //   return;
  // }
  // delay(4000);
  

  // left();
  // if (digitalRead(motorPin)) {
  //   stop();
  //   bigState = STOP;
  //   return;
  // }
  // delay(2000);

  // straight();

  // if (digitalRead(motorPin)) {
  //   stop();
  //   bigState = STOP;
  //   return;
  // }
  // delay(6500);
  

  // left();
  // if (digitalRead(motorPin)) {
  //   stop();
  //   bigState = STOP;
  //   return;
  // }
  // delay(2000);

  // straight();

  // if (digitalRead(motorPin)) {
  //   stop();
  //   bigState = STOP;
  //   return;
  // }

  // delay(1200);
  // while ((calculatedReflection = calculateReflection()) == noLine) {
  //   if (digitalRead(motorPin)) {
  //     stop();
  //     bigState = STOP;
  //     return;
  //   }
  // }

  // if (digitalRead(motorPin)) {
  //   stop();
  //   bigState = STOP;
  //   return;
  // }
  // delay(1500);

  // if (digitalRead(motorPin)) {
  //   stop();
  //   bigState = STOP;
  //   return;
  // }
  // right_to_line(180);

  // for (int i = 0; i < 5; i++) distance_array[i] = 65535;
  // digitalWrite(LED_BUILTIN, LOW);
}