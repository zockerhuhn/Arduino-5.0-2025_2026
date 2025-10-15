
void StraightenByWall() {
  digitalWrite(LEDR, LOW);
  if (digitalRead(motorPin)) {
    bigState = STOP;
    return;
  }
  left(30);
  right();
  if (digitalRead(motorPin)) {
    bigState = STOP;
    return;
  }
  readWriteDistanceArray();
  int smallest = INT_MAX;
  while ((readDistance() - smallest) < 5) {
    delay(1);
    smallest = min(readDistance(), smallest);
    if (digitalRead(motorPin)) {
      bigState = STOP;
      return;
    }
  }
  left(3); // compensate for the bit of turning that happened after the smallest was detected
  stop();
  digitalWrite(LEDR, HIGH);
}

// TODO update 
void opfer() {
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
  if (digitalRead(motorPin)) {
    bigState = STOP;
    return;
  }
  stop();
  
  for (int i = 0; i < 5; i++) readDistance();
  
  while (distance_val >= opfer_wall_threshold) {
    StraightenByWall();
    straight(1.8);
    if (digitalRead(motorPin)) {
      bigState = STOP;
      return;
    }
    delay(500);
    for (int i = 0; i < 5; i++) readDistance();
  }
  stop();
  delay(5000);
}
