// CAMERA

// Messages between OpenMV Cam and Arduino are limited to 256 bytes
openmv::rpc_scratch_buffer<256> scratch_buffer;
// Register a maximum of 8 commands
openmv::rpc_callback_buffer<8> callback_buffer;

openmv::rpc_hardware_serial1_uart_slave openMvCam(115200);

// Flag tracking if data from the camera is received this loop
bool has_new_data = false;
int cycles_since_data = 0;

// Data received from OpenMV Cam
int16_t received_cam_angle;
const int NUM_ANGLE_VALS = 6;
int16_t angle_array[NUM_ANGLE_VALS];
int16_t cam_angle;

// Kreuzungs-Data
bool green_left = false;
bool green_right = false;
bool is_red = false;

//MOTOREN
// Dieses Objekt repräsentiert 2 Motor-Kanäle (1..2 Motoren pro Kanal):
RescueBoardMotors motors = RescueBoardMotors();
#define motorPin D12
#define calibrationPin A6

// TODO tune!!!
int base_left_speed = 125;
int base_right_speed = 85;

//ABSTANDSSENSOR 1
const uint16_t LOST_CONNECTION = -1;
uint16_t last_distance_val = LOST_CONNECTION;
VL53L0X tofSensor = VL53L0X();
VL53L0X tofSensor2 = VL53L0X();

// hier speichern wir 5 TOF-Sensorwerte ab:
const int NUM_DISTANCE_VALS = 5;
int distance_array[NUM_DISTANCE_VALS]; 
int distance_val;
int distance_array2[NUM_DISTANCE_VALS]; 
int distance_val2;

int obstacle_threshold = 130;
int opfer_wall_threshold = 150;
int opfer_void_threshold = 500;

//KOMPASSSENSOR
#define CMPS12 0x60
uint16_t direction;
uint16_t current_direction;

// STATE MACHINE
enum BigState {
  OPFER,
  ABSTAND,
  DRIVING,
  STOP,
};
BigState bigState;

// Debug modes:
enum DebugMode {
  LOG_NOTHING,
  LOG_COLOUR,
  LOG_LINE,
  LOG_DISTANCE,
};
enum DebugMode debug = LOG_NOTHING;

// Opferlogic
int no_line_cycle_count = 0;