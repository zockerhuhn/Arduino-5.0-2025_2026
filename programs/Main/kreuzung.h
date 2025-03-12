#pragma once

void kreuzung(int sides /*- 1 is left, 0 is none, 1 is right*/) {
  if (!(digitalRead(motorPin))) {
    // drive forward slowly, check for greens
    digitalWrite(LED_BUILTIN, HIGH); // Activate Lamp to see when a Kreuzung is detected

    int green_occurences1 = 0; // right
    int green_occurences2 = 0; // left

    bool stopping = false;
    int stopping_in = -1;
    straight();
    while (!(stopping)) {
      if (stopping_in > 0) stopping_in--;
      if (stopping_in == 0) stopping = true;
      
      readColor();
      readColor2();

      if (isGreen()) {
        green_occurences1 += 1; 
        Serial.print("Found green 1 (right)\t");
        stopping_in = 2;
        digitalWrite(LEDG, LOW);
        delay(10);
        if (green_occurences2 >= 2) {
          digitalWrite(LEDR, HIGH);
          digitalWrite(LEDB, LOW);
        }
        else digitalWrite(LEDG, HIGH);
      }
      if (isGreen2()) {
        green_occurences2 += 1;
        Serial.print("Found green 2 (left)\t");
        stopping_in = 2;
        digitalWrite(LEDB, LOW);
        delay(10);
        if (green_occurences1 >= 2) {
          digitalWrite(LEDR, HIGH);
          digitalWrite(LEDG, LOW);
        }
        else digitalWrite(LEDB, HIGH);
      }

      calculatedReflection = calculateReflection();

      if (!(calculatedReflection == frontalLine || calculatedReflection == sideLeftLine ||  calculatedReflection == sideRightLine) && stopping_in < 0) {
        stopping_in = 3;
      }
      
      if (green_occurences1 >= 2 && green_occurences2 >= 2) stopping = true;

      if (green_occurences1 >= 4 && green_occurences2 < 2)  stopping = true;

      if (green_occurences2 >= 4 && green_occurences1 < 2)  stopping = true;

      delay(10);
      if (digitalRead(motorPin)) {
        stop();
        return;
      }
    }

    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LEDR, LOW);
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDB, LOW);


    // Handle the recorded greens
    if (green_occurences1 >= 2 && green_occurences2 >= 2) {
      // Turn
      Serial.print("turn\t");
      right(180, 2);
      // move forward just a bit
      straight(2);
      delay(100);
    }
    else if (green_occurences1 >= 2) {
      Serial.print("right\t");

      // Robot should be about above the geometric centre
      right(90, 1.8);
      straight(1.8); // then go straight a bit to avoid seeing a crossing again
      delay(200);     
    }
    else if (green_occurences2 >= 2) {
      Serial.print("left\t");

      left(90, 1.8);
      straight(1.8);
      delay(200);
    }
    else { // Did not find any green
      switch (sides) {
        case -1:
          right(45);
          left_to_line();
          break;
        case 1:
          left(45);
          right_to_line();
          break;
        default:
          break;
      }
    }
  }
}
