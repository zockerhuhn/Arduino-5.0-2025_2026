#pragma once
// #define PI 3.141592653589793238462643383279502884197169399375105820974944592307816406286

#include "Distance.h"
#include "Camera.h"
#include <math.h>

void motor_setup() {
  motors.initialize();
  // falls man global die Motor-Drehrichtung ändern möchte:
  motors.flipLeftMotor(false); 
  motors.flipRightMotor(false);
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
  motors.flipRightMotor(false);
  motors.setSpeeds((int)(base_left_speed * speed),(int)(base_right_speed * speed)); //prevent motor drifting
}

// Moves the robot according to the angle. 
// IMPORTANTLY the robot won't move BY the angle, it should move right or left with a specific speed
// The idea would be that since the image data is "continuous", the angle will be recalculated quickly
// and the new speed overrides the old one
void move_as_angle(int angle) {
  // TODO aaaaaaa is that even a good idea?
  if (digitalRead(motorPin)) {
    bigState = STOP;
    return;
  }
  // regardless, the speed should be related to the angle in such a way
  // that it is maximised at the angle 0 for both sides and be 0 at either extreme for the opposite side and 1 for the adjacent
  // This is basically the point of trigonometry
  double left_factor; double right_factor;
  left_factor = 1;
  right_factor = 1;

  if (angle > 10) {
    right_factor = 1.5 * cos(angle * PI / 180);
    left_factor = cos(angle * PI / 180);
  }
  else if (angle < -10) {
    left_factor = 1.5 * cos(angle * PI / 180);
    right_factor = cos(angle * PI / 180);
  }

  if (angle > 35) {
    right_factor = 1.5 * cos(angle * PI / 180);
    // TODO find good way to calculate the other factor
    left_factor = -right_factor;//-(double)(right_factor / 2);
  }
  else if (angle < -35) {
    left_factor = 1.5 * cos(angle * PI / 180);
    right_factor = -left_factor;//-(double)(left_factor / 2);
  }

  // TODO change movement a bit because of weight at the back
  if (angle >= 45) {
    right_factor = 2 * sin(angle * PI / 180);
    left_factor = -right_factor;
  }
  else if (angle <= -45) {
    left_factor = 2 * sin(angle * PI / 180);
    right_factor = -left_factor;
  }


  Serial.println(String(left_factor) + " " + String(right_factor));
  motors.setSpeeds((int)(left_factor * base_left_speed), (int)(right_factor * base_right_speed));
}

void left(int turnBy=0, float speed = 1) //turn left
{
  // This depends on the compass, which should therefore be strongly reconsidered
  // But the camera can probably correct the resulting error
  stop();
  if (digitalRead(motorPin)) {
    bigState = STOP;
    return;
  }
  readDirection();
  int initialDirection = direction;
  motors.setSpeeds(-(int)(base_left_speed * speed), (int)(base_right_speed * speed));
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
  motors.setSpeeds((int)(base_left_speed * speed), -(int)(base_right_speed * speed));
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

// void straight_left(float speed = 1) //drive straight but pull left
// {
//   if (digitalRead(motorPin)) {
//     stop();
//     bigState = STOP;
//     return;
//   }
//   // Configuration for left
//   motors.flipLeftMotor(true);
//   motors.flipRightMotor(true);
//   motors.setSpeeds((int)(40 * speed), (int)(100 * speed));
// }

// void straight_right(float speed = 1) //drive straight but pull right
// {
//   if (digitalRead(motorPin)) {
//     stop();
//     bigState = STOP;
//     return;
//   }
//   // Configuration for right
//   motors.flipLeftMotor(false);
//   motors.flipRightMotor(false);
//   motors.setSpeeds((int)(100 * speed), (int)(40 * speed));
// }


void abstand_umfahren() {
  digitalWrite(LED_BUILTIN, HIGH);
  if (digitalRead(motorPin)) {
    stop();
    bigState = STOP;
    return;
  }

  right(80);

  // right();
  // if (digitalRead(motorPin)) {
  //   bigState = STOP;
  //   return;
  // }
  // readWriteDistanceArray2();
  // int smallest = INT_MAX;
  // while ((readDistance2() - smallest) < 20) {
  //   delay(1);
  //   smallest = min(readDistance2(), smallest);
  //   // Serial.println(String(smallest) + " " + String(readDistance2()));
  //   if (digitalRead(motorPin)) {
  //     bigState = STOP;
  //     return;
  //   }
  // }

  // left(5);

  // stop();
  // delay(2000);

  for (int i = 0; i < NUM_ANGLE_VALS; i++) angle_array[i] = 360;
  cam_angle = 360;
  while (cam_angle == 360 || cam_angle < 20) {
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);
    if (openMvCam.loop()) {
      append_to_window(received_cam_angle);
      get_angle();
    }
    motors.setSpeeds((int)(base_left_speed / 2), (int)(base_right_speed * 2));
    
    if (readDistance2() - obstacle_threshold > 35) {
      motors.setSpeeds((int)(base_left_speed / 2), (int)(base_right_speed * 3));
    }
  }
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, LOW);

  move_as_angle(cam_angle);

  if (digitalRead(motorPin)) {
    stop();
    bigState = STOP;
    return;
  }
  // right(75);

  for (int i = 0; i < 5; i++) distance_array[i] = 65535;
  digitalWrite(LED_BUILTIN, LOW);
}