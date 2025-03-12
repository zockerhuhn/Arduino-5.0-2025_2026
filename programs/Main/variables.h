//REFLEKTIONSSENSOREN
int varrechts = 0;
int varlinks = 0;
const uint8_t SENSOR_BAR_NUM_SENSORS = 6;
const uint8_t SENSOR_BAR_PINS[] = {D6, D0, D1, D7, D8, D9};
// hier speichern wir die 6 Reflektionssensorwerte ab:
uint16_t reflectance_array[SENSOR_BAR_NUM_SENSORS];
QTRSensors reflectanceSensor = QTRSensors();

enum Reflections {
  sideRightLine,
  sideLeftLine,
  hardLeftLine,
  hardRightLine,
  leftLine,
  rightLine,
  frontalLine,
  extremeLine,
  normalLine,
  noLine,
};

Reflections calculatedReflection;
// hier speichern wir die Farbsensorwerte ab:
// Roh-Werte (Es gibt auch kalibierte Werte, aber die sind sehr langsam auszulesen):
uint16_t red, green, blue, brightness;
uint16_t red2, green2, blue2, brightness2;

uint16_t old_colour[4];
uint16_t old_colour2[4];

//FARBSENSOREN
/** Sensor sehr schnell einstellen (ungenauer):
 *  Gain 4x fand ich am besten, aber dann sind die Werte so stabil,
 *  dass die Fehlerdetektion immer ausgelöst hat (siehe unten "helligkeitStatischStoppuhr.hasPassed"). */
Adafruit_TCS34725 rgbSensor = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_60MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 rgbSensor2 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_60MS, TCS34725_GAIN_4X);

// Declaring values for calibration, will be reset
uint16_t average_r, average_g, average_b, average_c,  average_r2, average_g2, average_b2, average_c2;

//MOTOREN
// Dieses Objekt repräsentiert 2 Motor-Kanäle (1..2 Motoren pro Kanal):
RescueBoardMotors motors = RescueBoardMotors();
#define motorPin D12
#define calibrationPin A6

//ABSTANDSSENSOR
const uint16_t LOST_CONNECTION = -1;
uint16_t last_distance_val = LOST_CONNECTION;
VL53L0X tofSensor = VL53L0X();
const uint8_t NEW_TOF_ADDRESS = 0x30;

// hier speichern wir 5 TOF-Sensorwerte ab:
const int NUM_DISTANCE_VALS = 5;
int distance_array[NUM_DISTANCE_VALS]; 
int distance_val;

int obstacle_threshold = 80;
int wallscan_threshold = 20;
int opfer_wall_threshold = 100;

//KOMPASSSENSOR
#define CMPS12 0x60
uint16_t direction;
uint16_t current_direction;


// State machine
enum BigState {
  OPFER,
  DRIVING,
  STOP,
  CALIBRATION,
};
BigState bigState;

enum State {
  straight_driving,
  crossing,
  turn_left_to_line,
  turn_right_to_line,
  turn_left_90,
  turn_right_90,
  left_side,
  right_side,
};
State state = straight_driving;

// Debug modes:
enum DebugMode {
  LOG_NOTHING,
  LOG_DISTANCE,
  LOG_COLOUR,
  LOG_REFLECTANCE,
  LOG_LINE,
};
enum DebugMode debug = LOG_NOTHING;

// Kreuzungslogic
int last_kreuzung = 0; // TODO implement this so kreuzung doesn't trigger immediately after another kreuzung

// Opferlogic
int no_line_cycle_count = 0;
int left_line_cycle_count = 0;
int right_line_cycle_count = 0;
int kreuzung_cooldown  = 0;