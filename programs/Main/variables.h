// CAMERA

// Messages between OpenMV Cam and Arduino are limited to 256 bytes
openmv::rpc_scratch_buffer<256> scratch_buffer;
// Register a maximum of 8 commands
openmv::rpc_callback_buffer<8> callback_buffer;

openmv::rpc_hardware_serial1_uart_slave openMvCam(115200);

// Flag tracking if data from the camera is received this loop
bool new_data = false;
int cycles_since_data = 0;

// Data received from OpenMV Cam
uint16_t received_cam_angle;

// Lines
enum Lines {
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
Lines calculatedReflection;

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

// OBSOLETE STATE MACHINE
enum BigState {
  OPFER,
  ABSTAND,
  DRIVING,
  STOP,
};
BigState bigState;

enum State {
  straight_driving,
  crossing,
  strive_left,
  strive_right,
  turn_left_90,
  turn_right_90,
};
State state = straight_driving;

// Debug modes:
enum DebugMode {
  LOG_NOTHING,
  LOG_COLOUR,
  LOG_LINE,
};
enum DebugMode debug = LOG_NOTHING;

// Opferlogic
int no_line_cycle_count = 0;