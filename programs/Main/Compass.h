#pragma once

void readDirection() {
  Wire.beginTransmission(CMPS12);
  Wire.write(0X02);
  
  if (Wire.endTransmission(false)) {
    direction = -1; // set an invalid value when the transmission had an error
  } else {
    Wire.requestFrom(CMPS12, 2);
    direction = (int16_t)(Wire.read()<<8|Wire.read())/10.00; //Two bytes Yaw in range of (0 to 359 degrees)
  }
  // Serial.println("Direction: " + String(direction));
}