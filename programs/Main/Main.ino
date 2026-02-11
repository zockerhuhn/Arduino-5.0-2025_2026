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
    - Motoren
  */

  distance_setup();

  openmv_cam_setup();

  motor_setup();

  debug = LOG_DISTANCE;
  bigState = DRIVING;
}

void loop()
{
  // TODO redesign everything for state machines

  // Occasionally (if new data is sent) updates the receiving data
  has_new_data = openMvCam.loop();
  // straight();
  digitalWrite(LEDR, (PinStatus)!has_new_data);
  digitalWrite(LEDG, (PinStatus)has_new_data);
  if (has_new_data) {
    cycles_since_data = 0;
    append_to_window(received_cam_angle);
    get_angle();
    // Serial.println("received angle: " + String(received_cam_angle));
  }
  else {
    Serial.println("No new data");
    cycles_since_data++;
    if (cycles_since_data > 20) {
      // Connection lost?
      stop();
      // Exit out of the function? nahh
      // for (int i = 0; i < NUM_ANGLE_VALS; ++i) angle_array[i] = 360;
      // cam_angle = 360;
    }
  }

  if (has_new_data) switch (bigState) {
    case STOP:
      stop();
      // check for red!!!
      if (is_red) {
        digitalWrite(LEDR, HIGH);
        delay(8000);
        digitalWrite(LEDR, LOW);
        for (int i = 0; i < NUM_ANGLE_VALS; ++i) angle_array[i] = 0;
        get_angle();
        bigState = DRIVING;
        break;
      }
      else {
        delay(100);
      
        // Reset LEDs
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(LEDR, LOW);
        digitalWrite(LEDG, LOW);
        digitalWrite(LEDB, LOW);

        // Set distance array to invalid value
        for (int i = 0; i < NUM_DISTANCE_VALS; i++) distance_array[i] = 65535;

        // Set angle array to zero (because driving straight is usually not too bad)
        for (int i = 0; i < NUM_ANGLE_VALS; i++) angle_array[i] = 0;

        // Debugging
        switch (debug) {
          case LOG_NOTHING:
            break;

          case LOG_LINE: 
            // TODO implement check for cam and look what line may be
            // Probably not really good or smart
            Serial.println("Angle:" +  String(cam_angle));
            break;

          case LOG_COLOUR:
            if (is_red) Serial.println("reeeeeeeeed"); 
            else {
              if (green_left && green_right) Serial.println("green green");
              else if (green_left) Serial.println("green -----");
              else if (green_right) Serial.println("----- green");
              else Serial.println("----- -----");
            }
            break;

          case LOG_DISTANCE:
            readRawDistance();
            logDistance();
            readRawDistance2();
            logDistance2();
        }
  
        if (!digitalRead(motorPin) && !is_red/*ensure no red is seen*/) {
          bigState = DRIVING;
        } else if (!digitalRead(motorPin) && is_red) {
          bigState = STOP;
        }
      }
      break;

    case OPFER:
      Serial.println("opfer");
      digitalWrite(LEDR, LOW);
      digitalWrite(LEDG, LOW);
      digitalWrite(LEDB, LOW);
      no_line_cycle_count = 0;
      opfer();
      bigState = DRIVING;
      break;
    
    case ABSTAND:
      // TODO check which direction is save to go
      // TODO umfahren accordingly
      abstand_umfahren();
      // TODO remember to implement that the line may not continue immediately 
      // behind the obstacle but could be just around the corner or smthng
      bigState = DRIVING;
      break;

    case DRIVING:
      if (digitalRead(motorPin)) {
        bigState = STOP;
      }
      if (no_line_cycle_count >= 350) {
        bigState = OPFER;
        break; // Jump prematurely out of the switch-case
      }

      readDistance();
      if (distance_val <= obstacle_threshold) {
        bigState = ABSTAND;
      }

      // Check for valid angle
      if (-90 < cam_angle && cam_angle < 90) {
        Serial.println("drive by angle: " + String(cam_angle));

        digitalWrite(LEDG, LOW);
        digitalWrite(LEDB, LOW);
        // Basically move according to the angle with specific speed
        move_as_angle(cam_angle);
      } else {
        // Kreuzungslogik
        if (cam_angle == 90) {
          // Turn by 90 degrees left
          digitalWrite(LEDG, HIGH);
          digitalWrite(LEDB, HIGH);
          digitalWrite(LEDR, LOW);
          Serial.println("Left!");
          straight();
          delay(1000);
          // Let cam correct the rest
          left(75);
          // Write invalid data to the vals
          for (int i = 0; i < NUM_ANGLE_VALS; ++i) angle_array[i] = 0;
          get_angle();
          break;
        }
        if (cam_angle == -90) {
          // Turn by 90 degrees right
          digitalWrite(LEDG, LOW);
          digitalWrite(LEDB, HIGH);
          digitalWrite(LEDR, HIGH);
          Serial.println("Right.");
          straight();
          delay(1000);
          right(75);
          for (int i = 0; i < NUM_ANGLE_VALS; ++i) angle_array[i] = 0;
          get_angle();
          break;
        }
        if (cam_angle == 180) {
          // Turn by 180 degrees
          Serial.println("Turn!");
          digitalWrite(LEDG, HIGH);
          digitalWrite(LEDB, HIGH);
          digitalWrite(LEDR, HIGH);
          // Not really important to be positioned exactly above crossing, so only driving a bit more forward
          straight();
          delay(700);
          left(160);
          for (int i = 0; i < NUM_ANGLE_VALS; ++i) angle_array[i] = 0;
          get_angle();
          break;
        }
        if (is_red) {
          digitalWrite(LEDG, LOW);
          digitalWrite(LEDB, LOW);
          digitalWrite(LEDR, HIGH);
          stop();
          bigState = STOP;
        }
        if (cam_angle == 360) {
          Serial.println("No line seen...");
          // No angle seen
          no_line_cycle_count++;
        }
      }

      // make sure processor isn't maxed out
      delay(10);
      break;
  }
}
//gyatt