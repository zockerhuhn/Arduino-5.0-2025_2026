
void straighten_by_wall() {
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
  readWriteDistanceArray2();
  int smallest = INT_MAX;
  while ((readDistance2() - smallest) < 5) {
    delay(1);
    smallest = min(readDistance2(), smallest);
    if (digitalRead(motorPin)) {
      bigState = STOP;
      return;
    }
  }
  left(5); // compensate for the bit of turning that happened after the smallest was detected
  stop();
  digitalWrite(LEDR, HIGH);
}

void opfer() {
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
  while (digitalRead(motorPin)) {
    // bigState = STOP;
    // return;
    delay(1);
  }
  stop();
  // Turn left, then drive a bit straight and turn left again so we face the wall to the left of the entry
  left(80);
  straight();
  delay(1000);
  left(80);
  straight();
  // Drive until close to the wall
  while (distance_val >= opfer_wall_threshold) {
    if (digitalRead(motorPin)) {
      bigState = STOP;
      return;
    }
    for (int i = 0; i < NUM_DISTANCE_VALS; i++) readDistance();
    delay(10);
  }

  for (int j = 0; j < 4; ++j) {

    // TODO make sure that this works with the corners and stuff
    right(80);
    straighten_by_wall();

    for (int i = 0; i < NUM_DISTANCE_VALS; i++) readDistance();
    for (int i = 0; i < NUM_DISTANCE_VALS; i++) readDistance2();

    straight();
    while (distance_val >= opfer_wall_threshold) {  

      // Exit out of Opferraum if a void is gazed upon
      if (distance_val2 >= opfer_void_threshold) {
        left(80);
        straight();
        delay(2000);
        return;
      }

      for (int i = 0; i < NUM_DISTANCE_VALS; i++) readDistance();
      for (int i = 0; i < NUM_DISTANCE_VALS; i++) readDistance2();

      // TODO adapt delay so the robot covers approximately one "wall tile" 
      delay(800);
    }


  }
}
