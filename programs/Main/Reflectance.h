#pragma once

void logReflection() { // print reflection sensor values
  Serial.print("Reflectance Array Values: ");
  for (int i = 0; i < SENSOR_BAR_NUM_SENSORS; i++)
  {
    Serial.print(String(reflectance_array[i]) + '\t'); // alles in eine Zeile
  }
  Serial.println(); // neue Zeile beginnen
}


void readReflection() { //read reflectionsensor
  reflectanceSensor.read(reflectance_array);
}


Reflections calculateReflection() { //read reflection and return processed result 
  readReflection();
  // logReflection();
  if ((reflectance_array[0] >= reflectionBlackThreshold && (reflectance_array[1] >= reflectionBlackThreshold || reflectance_array[2] >= reflectionBlackThreshold)) && (reflectance_array[5] >= reflectionBlackThreshold && (reflectance_array[4] >= reflectionBlackThreshold || reflectance_array[3] >= reflectionBlackThreshold))) {
    return frontalLine;
  }
  else if ((reflectance_array[0] >= reflectionBlackThreshold || reflectance_array[1] >= reflectionBlackThreshold) && (reflectance_array[5] >= reflectionBlackThreshold || reflectance_array[4] >= reflectionBlackThreshold)) {
    return extremeLine;
  }
  else if ((reflectance_array[0] >= reflectionBlackThreshold && reflectance_array[1] >= reflectionBlackThreshold && reflectance_array[2] >= reflectionBlackThreshold)) {
    return sideRightLine;
  }
  else if ((reflectance_array[3] >= reflectionBlackThreshold && reflectance_array[4] >= reflectionBlackThreshold && reflectance_array[5] >= reflectionBlackThreshold)) {
    return sideLeftLine;
  }
  else if (reflectance_array[5] >= reflectionBlackThreshold) {
    return hardLeftLine;
  }
  else if (reflectance_array[0] >= reflectionBlackThreshold) {
    return hardRightLine;
  }
  else if (reflectance_array[4] >= reflectionBlackThreshold) {
    return leftLine;
  }
  else if (reflectance_array[1] >= reflectionBlackThreshold) {
    return rightLine;
  }
  else if ((reflectance_array[2] >= reflectionBlackThreshold) || (reflectance_array[3] >= reflectionBlackThreshold)) {
    return normalLine;
  }
  else {
    return noLine;
  }
}