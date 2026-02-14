
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
    bigState = STOP;
    return;
  }
  stop();
  
  //
  // for (int j = 0; j < 16; ++j) {
  //   straighten_by_wall();
  //   for (int i = 0; i < NUM_DISTANCE_VALS; i++) readDistance2();
  //   if (distance_val2 >= opfer_void_threshold) {
  //     left(80);
  //     straight();
  //     delay(2000);
  //     return;
  //   }
    
  //   int dur = 0;
  //   if (distance_val2 > opfer_wall_threshold) {
  //     while (distance_val2 > opfer_wall_threshold) {
  //       motors.setSpeeds(base_left_speed, 1.5 * base_right_speed);
  //       delay(1);
  //       dur++;
  //     }
  //     straighten_by_wall();
  //   }

  //   right(80);
  //   for (int i = 0; i < NUM_DISTANCE_VALS; i++) readDistance2();
  //   if (distance_val2 > opfer_wall_threshold) {
  //     left(80);
  //   }
  //   else {
  //     delay(1000);
  //   }
  //   straight();
  //   delay((int)(max(2650 - 1.5 * dur, 0)));
  // }
  //
 

  
  // Turn left, then drive a bit straight and turn left again so we face the wall to the left of the entry
  // left(80);
  // straight();
  // delay(1000);
  // left(80);

  
  // Drive until close to the wall
  
  straight();

  while (distance_val >= opfer_wall_threshold) {
    if (digitalRead(motorPin)) {
      bigState = STOP;
      return;
    }

    for (int i = 0; i < NUM_DISTANCE_VALS; i++) readDistance2();
    // Exit out of Opferraum if a void is gazed upon
    if (distance_val2 >= opfer_void_threshold) {
      left(80);
      straight();
      delay(2000);
      return;
    }

    for (int count = 0; distance_val >= opfer_wall_threshold; count++) {
      delay(1);
      readDistance();
      if (count % 70 == 0) {
        straighten_by_wall();
        straight();
      }
      readDistance2();
      if (distance_val2 >= opfer_void_threshold) {
        left(80);
        straight();
        delay(2000);
        return;
      }
    }
  }

  if (distance_val >= opfer_wall_threshold) {
    right(85);
    for (int i = 0; i < NUM_DISTANCE_VALS; i++) readDistance2();
    if (distance_val2 >= opfer_wall_threshold / 2) {
      
    }
  }

  for (int j = 0; j < 4; ++j) {

    // TODO make sure that this works with the corners and stuff
    right(85);
    straight();
    delay(2000);
    // straighten_by_wall();
    // straight();

    for (int i = 0; i < NUM_DISTANCE_VALS; i++) readDistance();
    for (int i = 0; i < NUM_DISTANCE_VALS; i++) readDistance2();

    for (int k = 0; k < 3; ++k) {  

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
      for (int count = 0; distance_val >= opfer_wall_threshold; count++) {
        delay(1);
        readDistance();
        if (count % 70 == 0) {
          straighten_by_wall();
          straight();
        }

        readDistance2();
        if (distance_val2 >= opfer_void_threshold) {
          left(80);
          straight();
          delay(2000);
          return;
        }
      } 
    }
  }
}
