#pragma once

int findAverage(int *array, int size) {
  int avg = 0;
  for (int i = 0; i < size; i++) {
    if (array[i] < 8000 && array[i] > 10) {
      avg += array[i];
    }
    else {
      return max(array[i], 8190);
    }
  }
  return avg / size;
}

void logDistance() {
    Serial.println("distance value: " + String(distance_val));
}

void moveArrBack(int *array, int size) {
  for (int i = 1; i < size; i++) {
    array[i-1] = array[i];
  }
}

int readRawDistance() {
  // if (!tofSensor.timeoutOccurred()) {
  //   distance_val = tofSensor.readRangeContinuousMillimeters();
  // }
  // // logDistance();
  // // statt 65535 kann es auch passieren, dass sich der Wert einfach nicht mehr ändert
  // if (distance_val != 65535) {
  //   return distance_val;
  // }
  // // Fehler:
  // distance_val = LOST_CONNECTION;
  // Serial.println("ToF Verdrahtung prüfen! Roboter aus- und einschalten! " + String(tofSensor.readRangeContinuousMillimeters()));
  // return distance_val;
  return 500;
}

int readDistance(int num_average = 5) {
  readRawDistance();
  moveArrBack(distance_array, num_average);
  distance_array[4] = distance_val;

  distance_val = findAverage(distance_array, num_average);
  return distance_val;
}

int readWriteDistanceArray(int num_average = 5) {
  for (int i = 0; i < num_average; i++) readDistance(num_average);
  return distance_val;
}