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
  if (angle == 360) {
    angle = 45;
  }  
  double right_factor = 3 * abs(sin(angle * PI / 180));
  double left_factor = -back_factor * right_factor;
  motors.setSpeeds((int)(base_left_speed * speed * left_factor), (int)(base_right_speed * speed * right_factor));
  while (received_cam_data.main_angle > 10 || received_cam_data.main_angle == 360) {
    if (openMvCam.loop()) {
      append_to_window(received_cam_data.kreuzung_data);
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
  if (angle == 360) {
    angle = 45;
  } 
  double left_factor = 2 * abs(sin(angle * PI / 180));
  double right_factor = -back_factor * left_factor;
  motors.setSpeeds((int)(base_left_speed * speed * left_factor), (int)(base_right_speed * speed * right_factor));
  while (received_cam_data.main_angle < -10 || received_cam_data.main_angle == 360) {
    if (openMvCam.loop()) {
      append_to_window(received_cam_data.kreuzung_data);
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
  // ehh probably stupid

  double left_factor; double right_factor;
  left_factor = 1.0;
  right_factor = 1.0;

  double turn_factor = 3; 

  if (angle < 0) {
    right_factor -= turn_factor * ((double)angle/90.0*(double)angle/90.0);
  } else {
    left_factor -= turn_factor * ((double)angle/90.0*(double)angle/90.0);
  }
  
  // if (angle >= 0) {
  //   right_factor = 1.5 * cos(angle * PI / 180);
  //   left_factor = cos(angle * PI / 180);
  // }
  // else if (angle < -0) {
  //   left_factor = 1.5 * cos(angle * PI / 180);
  //   right_factor = cos(angle * PI / 180);
  // }

  // if (angle >= 30) {
  //   right_factor = 1.5 * abs(sin(angle * PI / 180));
  //   // TODO find good way to calculate the other factor
  //   left_factor = -right_factor;//-(double)(right_factor / 2);
  // }
  // else if (angle <= -30) {
  //   left_factor = 1.5 * abs(sin(angle * PI / 180));
  //   right_factor = -left_factor;//-(double)(left_factor / 2);
  // }

  // if (angle >= 45) {
  //   right_factor = 0;
  //   left_factor = -right_factor;
  //   left_to_line(angle, 0.5);
  // }
  // else if (angle <= -45) {
  //   left_factor = 0;
  //   right_factor = -left_factor;
  //   right_to_line(angle, 0.5);
  // }

  // if (angle >= 50) {
  //   straight(-1);
  //   delay(300);
  //   // left(35);
  //   left_to_line(angle, 3, 2);
  // }
  // else if (angle <= -50) {
  //   straight(-1);
  //   delay(300);
  //   // right(35);
  //   right_to_line(angle, 3, 2);
  // }

  // Serial.println(String(left_factor) + " " + String(right_factor));
  motors.setSpeeds((int)(left_factor * base_left_speed), (int)(right_factor * base_right_speed));
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
  motors.setSpeeds((int)(-0.25 * base_left_speed * speed), (int)(base_right_speed * speed));
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
  motors.setSpeeds((int)(base_left_speed * speed), (int)(-0.25 * base_right_speed * speed));
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

  readDirection();
  int start_direction = direction;
  Serial.println("Start: " + String(start_direction));

  straight_right(70, 1.5);

  for (int i = 0; i < NUM_ANGLE_VALS; i++) angle_array[i] = 360;
  clear_cam_data();
  cam_angle = 360;
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
  readDirection();
  while (!openMvCam.loop()) delay(1);
  append_to_window(received_cam_data.kreuzung_data);
  get_angle();
  Serial.println(String(direction) + " " + String(received_cam_data.num_pixels));
  // (((start_direction - direction) + 360) % 360 > 10 && (((start_direction - direction) + 360) % 360 < 70 && ((start_direction - direction) + 360) % 360 > 110)))
  while ((received_cam_data.num_pixels < 1300 && !(received_cam_data.angle1 != 360 && received_cam_data.angle1 > 20 && received_cam_data.angle2 != 360 && received_cam_data.angle2 < -20))) {
    readDirection();
    Serial.println(String(direction) + " " + String(received_cam_data.num_pixels) + " " + String(received_cam_data.line_right) + " " + String(((start_direction - direction) + 360) % 360 > 10));
    if (openMvCam.loop()) {
      append_to_window(received_cam_data.kreuzung_data);
      get_angle();
    }
    motors.setSpeeds((int)(base_left_speed / 2), (int)(2 * base_right_speed));
    delay(1);
  }

  delay(1700);
  right(100);
  straight(-1);
  delay(500);
  // right_to_line();

  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);

  digitalWrite(LEDB, LOW);

  for (int i = 0; i < NUM_DISTANCE_VALS; i++) distance_array[i] = 65535;
  clear_cam_data();
  digitalWrite(LED_BUILTIN, LOW);
}
