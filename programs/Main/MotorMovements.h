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

void straight(double speed = 1) //drive straight
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

void left_to_line(int angle = 45, double back_factor = 1, double speed = 1) {
  double right_factor = 3 * abs(sin(angle * PI / 180));
  double left_factor = -back_factor * right_factor;
  motors.setSpeeds((int)(base_left_speed * speed * left_factor), (int)(base_right_speed * speed * right_factor));
  while (cam_angle > 10) {
    if (openMvCam.loop()) {
      append_to_window(received_cam_angle);
      get_angle();
    }
    delay(1);
    if (digitalRead(motorPin)) {
      bigState = STOP;
      return;
    }
  }
  stop();
}

void right_to_line(int angle = 45, double back_factor = 1, double speed = 1) {
  double left_factor = 2 * abs(sin(angle * PI / 180));
  double right_factor = -back_factor * left_factor;
  motors.setSpeeds((int)(base_left_speed * speed * left_factor), (int)(base_right_speed * speed * right_factor));
  while (cam_angle < -10) {
    if (openMvCam.loop()) {
      append_to_window(received_cam_angle);
      get_angle();
    }
    delay(1);
    if (digitalRead(motorPin)) {
      bigState = STOP;
      return;
    }
  }
  stop();
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

  if (angle >= 30) {
    right_factor = 1.5 * abs(sin(angle * PI / 180));
    // TODO find good way to calculate the other factor
    left_factor = -right_factor;//-(double)(right_factor / 2);
  }
  else if (angle <= -30) {
    left_factor = 1.5 * abs(sin(angle * PI / 180));
    right_factor = -left_factor;//-(double)(left_factor / 2);
  }

  if (angle >= 45) {
    right_factor = 0;
    left_factor = -right_factor;
    left_to_line(angle, 0.5);
  }
  else if (angle <= -45) {
    left_factor = 0;
    right_factor = -left_factor;
    right_to_line(angle, 0.5);
  }

  if (angle >= 60) {
    straight(-1);
    delay(300);
    left_to_line(angle, 2, 2);
  }
  else if (angle <= -60) {
    straight(-1);
    delay(300);
    right_to_line(angle, 2, 2);
  }

  Serial.println(String(left_factor) + " " + String(right_factor));
  motors.setSpeeds((int)(left_factor * base_left_speed), (int)(right_factor * base_right_speed));
}

void left(int turnBy=0, double speed = 1) //turn left
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

void right(int turnBy=0, double speed = 1) //turn right
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

void straight_left(int turnBy=0, double speed = 1) //turn left
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
  motors.setSpeeds((int)(-0.5 * base_left_speed * speed), (int)(2 * base_right_speed * speed));
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

void straight_right(int turnBy=0, double speed = 1) //turn right
{
  stop();
  if (digitalRead(motorPin)) {
    bigState = STOP;
    return;
  }
  readDirection();
  int initialDirection = direction;
  motors.setSpeeds((int)(2 * base_left_speed * speed), (int)(-0.5 * base_right_speed * speed));
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


void abstand_umfahren() {
  digitalWrite(LED_BUILTIN, HIGH);
  if (digitalRead(motorPin)) {
    stop();
    bigState = STOP;
    return;
  }

  straight_right(70);

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
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
  while (cam_angle == 360 || cam_angle < 20) {
    if (openMvCam.loop()) {
      append_to_window(received_cam_angle);
      get_angle();
    }
    motors.setSpeeds((int)(base_left_speed / 2), (int)(base_right_speed * 2));
    
    if (readDistance2() - obstacle_threshold > 35) {
      motors.setSpeeds((int)(base_left_speed / 3), (int)(base_right_speed * 2));
    }
    delay(1);
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