#pragma once

void readColor() { //liest Farbsensor 1 aus und printed Ergebnis
  old_colour[0] = red;old_colour[1] = green;old_colour[2] = blue;old_colour[3] = brightness;
  rgbSensor.getRawData(&red, &green, &blue, &brightness);

  //Serial.println("R:" + String(red) + " G:" + String(green) + " B:" + String(blue) + " C:" + String(brightness));
  /** Dieser Mechanismus hier ist gefährlich, wenn es passieren kann, dass die Sensoren lange Zeit das selbe sehen:
   *  In meinen Versuchen habe ich oben den Gain von 4x auf 16x gestellt, um mehr Rauschen zu bekommen.
   *  Mit Timeout 5s sehe ich keine False-Negatives mehr: */
}

void readColor2() { //liest Farbsensor 2 aus und printed Ergebnis
  old_colour2[0] = red2;old_colour2[1] = green2;old_colour2[2] = blue2;old_colour2[3] = brightness2;
  rgbSensor2.getRawData(&red2, &green2, &blue2, &brightness2);

  //Serial.println("R:" + String(red2) + " G:" + String(green2) + " B:" + String(blue2) + " C:" + String(brightness2));
  /** Dieser Mechanismus hier ist gefährlich, wenn es passieren kann, dass die Sensoren lange Zeit das selbe sehen:
   *  In meinen Versuchen habe ich oben den Gain von 4x auf 16x gestellt, um mehr Rauschen zu bekommen.
   *  Mit Timeout 5s sehe ich keine False-Negatives mehr: */
}

bool isGreen() { // nimmt letzten ausgelesen Wert von Farbsensor 1 und return ob grün erkannt wurde als bool 
  return ((green-blueGreenThreshold) >= blue && (green-redGreenThreshold) >= red && brightness <= colorBrightMaxThreshold && brightness >= colorBrightMinThreshold);
}

bool isGreen2() { // nimmt letzten ausgelesen Wert von Farbsensor 2 und returnt ob grün erkannt wurde als bool
  return ((green2-blueGreenThreshold2) >= blue2 && (green2-redGreenThreshold2) >= red2 && brightness2 <= colorBrightMaxThreshold && brightness2 >= colorBrightMinThreshold);
}

bool isRed() {
  return ((1.3 * (blue + green) <= red + 300) || (1.3 * (old_colour[2] + old_colour[1]) <= old_colour[0] + 300));
}

bool isRed2() {
  return ((1.3 * (blue2 + green2) <= red2 + 300) || (1.3 * (old_colour2[2] + old_colour2[1]) <= old_colour2[0] + 300));
}

void redHandling() {
  readColor();
  readColor2();
  if ((isRed() && isRed2()) && (brightness <= colorBrightMaxThreshold + 800 || brightness2 <= colorBrightMaxThreshold + 800)) {
    
    stop();
    digitalWrite(LEDR, HIGH);
    
    // Serial.println("red"); 

    delay(8000); // More than 5 seconds

    if (digitalRead(motorPin)) {
      stop();
      return;
    }
  }
  digitalWrite(LEDR, LOW);
}
