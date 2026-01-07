#include "RescueBoardMotors.h"

// we create a real object as opposed to the ZumoMotors library
RescueBoardMotors::RescueBoardMotors() {
}

// set up pins
void RescueBoardMotors::initialize() {
  pinMode(DIR_L, OUTPUT);
  pinMode(PWM_L, OUTPUT);

  pinMode(DIR_R, OUTPUT);
  pinMode(PWM_R, OUTPUT);
}

// enable/disable flipping of left motor
void RescueBoardMotors::flipLeftMotor(boolean flip) {
  flipLeft = flip;
}

// enable/disable flipping of right motor
void RescueBoardMotors::flipRightMotor(boolean flip) {
  flipRight = flip;
}

void RescueBoardMotors::doSetSpeed(int speed, uint8_t pwm_pin, uint8_t dir_pin, bool flip) {
  boolean reverse = 0;

  // speed setting
  if (speed < 0) {
    speed = -speed; // make speed a positive quantity
    reverse = 1;    // preserve the direction
  }
  analogWrite(pwm_pin, constrain(speed, 0, 255));

  // direction setting
  if (reverse ^ flip) {
    digitalWrite(dir_pin, HIGH);
  } else {
    digitalWrite(dir_pin, LOW);
  }
}

// set speed for left 2 motors; speed is a number between -255 and 255
void RescueBoardMotors::setLeftSpeed(int speed) {
  doSetSpeed(speed, PWM_L, DIR_L, flipLeft);
}

// set speed for right 2 motors; speed is a number between -255 and 255
void RescueBoardMotors::setRightSpeed(int speed) {
  doSetSpeed(speed, PWM_R, DIR_R, flipRight);
}

// set speed for both motors
void RescueBoardMotors::setSpeeds(int leftSpeed, int rightSpeed) {
  setLeftSpeed(leftSpeed);
  setRightSpeed(rightSpeed);
}
