#pragma once

void distance_setup() {
  // ABSTANDSSENSOR INITIALISIEREN
  Serial.println("Initialisierung des ersten 1-Kanal ToF kann bis zu 10 Sekunden dauern...");
  tofSensor.setBus(&Wire);
  tofSensor.setAddress(0x29);
  if (!tofSensor.init()) {
      delay(5000); // damit wir Zeit haben den Serial Monitor zu öffnen nach dem Upload
      Serial.println("ToF 1 Verdrahtung prüfen! Roboter aus- und einschalten! Programm Ende.");
      while (1);
  }
  // Einstellung: Fehler, wenn der Sensor länger als 500ms lang nicht reagiert
  tofSensor.setTimeout(500);
  // Reichweiter vergrößern (macht den Sensor ungenauer)
  tofSensor.setSignalRateLimit(0.1);
  tofSensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  tofSensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
  // lasse Sensor die ganze Zeit an
  tofSensor.startContinuous();
  // Initialise the default values for the "window", should be in variables but won't work there
  for (int i = 0; i < NUM_DISTANCE_VALS; i++) distance_array[i] = 65535;
  Serial.println("Initialisierung Abstandssensor 1 abgeschlossen");

  
  // ABSTANDSSENSOR 2 INITIALISIEREN
  Serial.println("Initialisierung des zweiten 1-Kanal ToF kann bis zu 10 Sekunden dauern...");
  tofSensor2.setBus(&Wire1);
  tofSensor2.setAddress(0x29);
  if (!tofSensor2.init()) {
      // delay(5000); // damit wir Zeit haben den Serial Monitor zu öffnen nach dem Upload
      Serial.println("ToF 2 Verdrahtung prüfen! Roboter aus- und einschalten! Programm Ende.");
      while (1);
  }
  // Einstellung: Fehler, wenn der Sensor länger als 500ms lang nicht reagiert
  tofSensor2.setTimeout(500);
  // Reichweiter vergrößern (macht den Sensor ungenauer)
  tofSensor2.setSignalRateLimit(0.1);
  tofSensor2.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  tofSensor2.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
  // lasse Sensor die ganze Zeit an
  tofSensor2.startContinuous();
  // Initialise the default values for the "window", should be in variables but won't work there
  for (int i = 0; i < NUM_DISTANCE_VALS; i++) distance_array2[i] = 65535;
  Serial.println("Initialisierung Abstandssensor 2 abgeschlossen");
}

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
void logDistance2() {
    Serial.println("distance2 value: " + String(distance_val2));
}

void moveArrBack(int *array, int size) {
  for (int i = 1; i < size; i++) {
    array[i-1] = array[i];
  }
}

int readRawDistance() {
  if (!tofSensor.timeoutOccurred()) {
    distance_val = tofSensor.readRangeContinuousMillimeters();
  }
  // logDistance();
  // statt 65535 kann es auch passieren, dass sich der Wert einfach nicht mehr ändert
  if (distance_val != 65535) {
    return distance_val;
  }
  // Fehler:
  distance_val = LOST_CONNECTION;
  Serial.println("ToF 1 Verdrahtung prüfen! Roboter aus- und einschalten! " + String(tofSensor.readRangeContinuousMillimeters()));
  return distance_val;
}


int readRawDistance2() {
  if (!tofSensor2.timeoutOccurred()) {
    distance_val2 = tofSensor2.readRangeContinuousMillimeters();
  }
  // logDistance();
  // statt 65535 kann es auch passieren, dass sich der Wert einfach nicht mehr ändert
  if (distance_val2 != 65535) {
    return distance_val2;
  }
  // Fehler:
  distance_val2 = LOST_CONNECTION;
  Serial.println("ToF 2 Verdrahtung prüfen! Roboter aus- und einschalten! " + String(tofSensor2.readRangeContinuousMillimeters()));
  return distance_val2;
}

int readDistance(int num_average = NUM_DISTANCE_VALS) {
  readRawDistance();
  moveArrBack(distance_array, num_average);
  distance_array[4] = distance_val;

  distance_val = findAverage(distance_array, num_average);
  return distance_val;
}

int readDistance2(int num_average = NUM_DISTANCE_VALS) {
  readRawDistance2();
  moveArrBack(distance_array2, num_average);
  distance_array2[4] = distance_val2;

  distance_val2 = findAverage(distance_array2, num_average);
  logDistance2();
  return distance_val2;
}

int readWriteDistanceArray(int num_average = NUM_DISTANCE_VALS) {
  for (int i = 0; i < num_average; i++) readDistance(num_average);
  return distance_val;
}

int readWriteDistanceArray2(int num_average = NUM_DISTANCE_VALS) {
  for (int i = 0; i < num_average; i++) readDistance2(num_average);
  return distance_val;
}