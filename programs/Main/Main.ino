/** importiert Arduino automatisch, muss man also hier nicht unbedingt auch noch mal importieren: */
#include <Arduino.h>

/**
 * !!! Immer darauf achten, dass unten in der Statusleiste...
 *     ... das richtige Arduino Board eingestellt ist
 *     ... das richtige "Sketch File" ausgewählt ist (das ändert sich nämlich nicht automatisch)
 *     ... die richtige C/C++ Konfiguration eingestellt ist (sonst gibt es noch mehr "rede swiggels")
 *
 * :: Externen RGB Farbsensor auslesen ::
 * :: Serial Plotter ausprobieren ::
 * :: Hauptprogramm-Schleife in Zustände unterteilen ::
 * :: Programm in Funktionen unterteilen ::
 *
 * Hardware-Aufbau:
 * TCS34725:    Arduino Due / Arduino Nano RP2040 Connect:
 *     [LED <-- (kann man optional an irgendeinen Digital-Pin oder den INT Pin vom Sensor anschließen,
 *              um die LED abzuschalten)]
 *     [INT  --> (kann man optional an irgendeinen Digital-Pin anschließen,
 *                wenn man den Sensor effizienter auslesen will)]
 *      SDA <-> SDA
 *      SCL <-- SDA
 *     [3V3 --> (2,3V Spannungsversorgung, falls man sowas irgendwo benötigt)]
 *      GND <-- GND
 *      VIN <-- 3V3
 * Der QTR-6RC ist ein externer Sensor, der an jeden Digital Pin angeschlossen werden kann (Siehe SENSOR_LEISTE_PINS).
 * !!! Beim Arduino Nano RP2040 Connect darf der Sensor nicht an Pins A6 oder A7 angeschlossen werden,
 *     da diese nur Eingänge sind und die Bibliothek den Pin sowohl als Ein- als auch als Ausgang verwendet.
 *----------------------------------------------------------------------------------------------------------------------
 * Der VL53L0X ist ein externer Sensor, kann also entweder
 * - an   Bus I2C0 ("Wire")
 * - oder Bus I2C1 ("Wire1") verbunden werden.
------------------------------------------------------------------------------------------------------------------------
 * Hardware-Aufbau:
 * Arduino Nano RP2040 Connect (!! Arduino Due geht nicht !!)
 * <nichts weiter>
 * Der LSM6DSOX ist immer an Bus I2C0 ("Wire") verbunden. Das kann nicht geändert werden.
 *
 * Der LSM6DSOX Sensor hängt am I2C Bus (SDA/SCL) mit 7-Bit Adresse 0x6A.
 * Die Adresse kann nicht verändert werden. Man kann also keinen anderen Sensor mit derselben Adresse
 * an den selben Bus anschließen.
*/

#include "includes.h"     // all libraries
#include "variables.h"    // all declarations and variables
#include "Calibration.h" // calibration values for reflection sensors, color sensors and potentially compass sensor

#include "Compass.h" // commands for a compass
#include "MotorMovements.h"    //predefined motor movements
#include "Camera.h"        
#include "Kreuzung.h"      //command for handling crosssections
#include "Opfer.h"              //Du Opfer

#include "Distance.h"            // Abstand, noch nicht einsortiert zwischen die restlichen includes


void setup()
{
  delay(5000);                       // Wichtig für den Abstandssensor
  pinMode(LED_BUILTIN, OUTPUT);      // Pin D13
  pinMode(motorPin, INPUT_PULLDOWN); // define pinmode for switch on the side of the bot
  pinMode(calibrationPin, INPUT);      // define pinmode for calibration button

  // Set the color LEDS as outputs
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  // Turn of any "lingering" LEDs
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, LOW);

  Serial.begin(115200);
  // I2C Bus 1x für alle Bus-Teilnehmer initialisieren (sonst crasht das Betriebssystem)
  Wire.begin();           // Bus I2C0
  Wire.setClock(1000000); // 1MHz Kommunikationsgeschwindigkeit
  Wire1.begin();          // Bus I2C1

  // REIHENFOLGE:
  /*
    - Abstandssensor (?)
    - Kamera (?)
  */
  
  // // ABSTANDSSENSOR-INITIALISIEREN
  // Serial.println("Initialisierung des 1-Kanal ToF kann bis zu 10 Sekunden dauern...");
  // tofSensor.setBus(&Wire);
  // tofSensor.setAddress(NEW_TOF_ADDRESS);
  // if (!tofSensor.init()) {
  //     delay(5000); // damit wir Zeit haben den Serial Monitor zu öffnen nach dem Upload
  //     Serial.println("ToF Verdrahtung prüfen! Roboter aus- und einschalten! Programm Ende.");
  //     while (1);
  // }
  // // Einstellung: Fehler, wenn der Sensor länger als 500ms lang nicht reagiert
  // tofSensor.setTimeout(500);
  // // Reichweiter vergrößern (macht den Sensor ungenauer)
  // tofSensor.setSignalRateLimit(0.1);
  // tofSensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  // tofSensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
  // // lasse Sensor die ganze Zeit an
  // tofSensor.startContinuous();
  // // Initialise the default values for the "window", should be in variables but won't work there
  // for (int i = 0; i < NUM_DISTANCE_VALS; i++) distance_array[i] = 65535;
  // Serial.println("Initialisierung Abstandssensor abgeschlossen");

  // openmv_cam_setup();

  // motor_setup();

  debug = LOG_REFLECTANCE;
  bigState = DRIVING;
}

void loop()
{
  // TODO redesign everything for state machines

  // Occasionally (if new data is sent) updates the receiving data
  new_data = openMvCam.loop();
  if (new_data) {
    Serial.print("angle: " + String(received_cam_angle) + "\n");
    // TODO code...
    // move_as_angle(received_cam_angle);

  } 
  else {
    Serial.println("No new data");
  }

  
  // switch (bigState) {
  //   case STOP:
  //     stop();
  //     // check for red!!!
  //     if (false /*TODO check for red*/) { 
  //       digitalWrite(LEDR, HIGH);
  //       // TODO implement waiting 8 seconds with Chrono or smthng
  //     }
  //     else {
  //       delay(100);
      
  //       // Reset LEDs
  //       digitalWrite(LED_BUILTIN, LOW);
  //       digitalWrite(LEDR, LOW);
  //       digitalWrite(LEDG, LOW);
  //       digitalWrite(LEDB, LOW);

  //       // Set distance array to invalid value
  //       for (int i = 0; i < 5; i++) distance_array[i] = 65535;

  //       // Debugging
  //       switch (debug) {
  //         case LOG_NOTHING:
  //           break;

  //         case LOG_DISTANCE: 
  //           readDistance();
  //           logDistance();
  //           break;

  //         case LOG_LINE: 
  //           // TODO implement check for cam and look what line may be
  //           break;
  //       }
  
  //       if (!digitalRead(motorPin) /*TODO remember to check that red is true too*/) {
  //         bigState = DRIVING;
  //       }   
  //     }
  //     break;

  //   case OPFER:
  //     Serial.println("opfer");
  //     digitalWrite(LEDR, LOW);
  //     digitalWrite(LEDG, LOW);
  //     digitalWrite(LEDB, LOW);
  //     no_line_cycle_count = 0;
  //     opfer();
  //     bigState = DRIVING;

  //   case DRIVING:
  //     if (digitalRead(motorPin)) {
  //       bigState = STOP;
  //     }
  //     if (no_line_cycle_count >= 35) {
  //       bigState = OPFER;
  //       break; // Jump prematurely out of the switch-case
  //     }

  //     // if (left_line_cycle_count > right_line_cycle_count) {
  //     //   digitalWrite(LEDR, HIGH);
  //     //   digitalWrite(LEDG, HIGH);
  //     //   digitalWrite(LEDB, LOW);
  //     // }
  //     // if (right_line_cycle_count > left_line_cycle_count) {
  //     //   digitalWrite(LEDR, HIGH);
  //     //   digitalWrite(LEDG, LOW);
  //     //   digitalWrite(LEDB, HIGH);
  //     // }
  //     // if (left_line_cycle_count == right_line_cycle_count) {
  //     //   digitalWrite(LEDR, LOW);
  //     //   digitalWrite(LEDG, LOW);
  //     //   digitalWrite(LEDB, LOW);
  //     // }

  //     readDistance();
  //     if (distance_val <= obstacle_threshold) {
  //       abstand_umfahren(); // TODO stateify abstand
  //     }
  //     switch (state) {
  //       case crossing:
  //         if (left_line_cycle_count > right_line_cycle_count) kreuzung(-1);
  //         if (right_line_cycle_count > left_line_cycle_count) kreuzung(1);
  //         else kreuzung(0); // TODO stateify kreuzung
  //         left_line_cycle_count = 0;
  //         right_line_cycle_count = 0;
  //         state = straight_driving;
  //         break;

  //       case straight_driving:
  //         calculatedReflection = calculateReflection();

  //         if (calculatedReflection != noLine) {
  //           no_line_cycle_count = 0;
  //           if (left_line_cycle_count >= 1) {
  //             left_line_cycle_count--;
  //           }
  //           if (right_line_cycle_count >= 1) {
  //             right_line_cycle_count--;
  //           } 
  //         }

  //         if (calculatedReflection != normalLine) {
  //           switch (calculatedReflection) {
  //             case frontalLine:
  //               state = crossing;
  //               break;
  
  //             case leftLine: // TODO find out how to group cases
  //               state = turn_left_to_line;
  //               break;
  //             case hardLeftLine:
  //               state = turn_left_to_line;
  //               break;
  
  //             case rightLine:
  //               state = turn_right_to_line;
  //               break;
  //             case hardRightLine:
  //               state = turn_right_to_line;
  //               break;
  
  //             case sideLeftLine:
  //               state = left_side;
  //               break;
  //             case sideRightLine:
  //               state = right_side;
  //               break;
  
  //             case noLine:
  //               no_line_cycle_count++;
  //               break;
  //           }
  //           break; // Exit out from straight driving early
  //         } else {
  //           straight(2);
  //         }

  //         break;
        
  //       case turn_left_to_line:
  //         left_line_cycle_count++;
  //         left_to_line(1.6);
  //         break;  

  //       case turn_right_to_line:
  //         right_line_cycle_count++;
  //         right_to_line(1.6);
  //         break;

  //       case left_side:
  //       // PROBLEM!!!!! Seems to be that not aaaaalways side triggers at t-crossings
  //         digitalWrite(LED_BUILTIN, HIGH);
  //         left_line_cycle_count++;
  //         straight(1.6);
  //         readReflection();
  //         while (reflectance_array[5] > reflectionBlackThreshold && reflectance_array[4] > reflectionBlackThreshold) {
  //           readReflection();
  //         }
  //         delay(200);
  //         left();

  //         // Check for black on the other side
  //         readReflection();
  //         logReflection();
  //         while (reflectance_array[0] < reflectionBlackThreshold - 200 && reflectance_array[1] < reflectionBlackThreshold - 200) /*reduce threshold by 200 because it SHOULD be more sensitive, since only white should be there*/ {
  //           // Read the data
  //           readReflection();
  //           readColor();
  //           readColor2();
  //           // log stuff
  //           logReflection();
  //           Serial.println(String(red2) + " " + String(green2) + " " + String(blue2) + " " + String(isGreen2()));
  //           // Check for green right
  //           if (isGreen()) {
  //             digitalWrite(LEDG, HIGH);
  //             right(90, 1.8);
  //             digitalWrite(LEDG, LOW);
  //             break;
  //           }
  //           if (isGreen2()) {
  //             digitalWrite(LEDB, HIGH);
  //             left(90, 1.8);
  //             digitalWrite(LEDB, LOW);
  //             break;
  //           }
  //           if (digitalRead(motorPin)) {
  //             bigState = STOP;
  //             break;
  //           }
  //         }
  //         Serial.println();

  //         digitalWrite(LED_BUILTIN, LOW);
  //         state = straight_driving;
  //         break;

  //       case right_side:
  //         digitalWrite(LED_BUILTIN, HIGH);
  //         right_line_cycle_count++;
  //         straight(1.6);
  //         readReflection(); // not quite straight enough
  //         while (reflectance_array[0] > reflectionBlackThreshold && reflectance_array[1] > reflectionBlackThreshold) {
  //           readReflection();
  //         }
  //         delay(200);
  //         right();

  //         // Check for black on the other side
  //         readReflection();
  //         logReflection();
  //         while (reflectance_array[5] < reflectionBlackThreshold - 200 && reflectance_array[4] < reflectionBlackThreshold - 200) {
  //           // Read the data
  //           readReflection();
  //           readColor();
  //           readColor2();
  //           // log stuff
  //           logReflection();
  //           Serial.println(String(red) + " " + String(green) + " " + String(blue) + " " + String(isGreen()));
  //           // Check for green
  //           if (isGreen()) {
  //             digitalWrite(LEDG, HIGH);
  //             right(90, 1.8);
  //             digitalWrite(LEDG, LOW);
  //             break;
  //           }
  //           if (isGreen2()) {
  //             digitalWrite(LEDB, HIGH);
  //             left(90, 1.8);
  //             digitalWrite(LEDB, LOW);
  //             break;
  //           }
  //           if (digitalRead(motorPin)) {
  //             bigState = STOP;
  //             break;
  //           }
  //         }
  //         Serial.println();

  //         digitalWrite(LED_BUILTIN, LOW);
  //         state = straight_driving;
  //         break;
  //     }

  //     readColor();
  //     readColor2();
  //     if (isRed()) {
  //       stop();
  //       bigState = STOP;
  //     }

  //     delay(1); // don't max out processor
  //     break;
  // }
}
//gyatt