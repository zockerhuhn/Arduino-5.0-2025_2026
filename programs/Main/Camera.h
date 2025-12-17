#pragma once

void update_cam_data(void* in_data, size_t in_data_len) {
    if (2 * in_data_len == sizeof(in_data) /*my best guess is that python doesn't differentiate between signed and unsigned und just sends 4 bytes instead of 2, which we need to account for*/) {
        // Data is complete
        // Copying in_data buffer into receiving data structure
        // Serial.println("Writing received data to buffer");
        memcpy(&received_cam_angle, in_data, sizeof(received_cam_angle));
        
    }
    else {
        Serial.println("Invalid data length AAAAAAAAA" + String(in_data_len) + "/" + String(sizeof(in_data)));
        memcpy(&received_cam_angle, in_data, sizeof(received_cam_angle));
    }
}

void openmv_cam_setup() {
    // Calls update_cam_data when the OpenMV Cam requests it
    openMvCam.register_callback(F("update_cam_data"), update_cam_data);
    // Initialising the camera
    openMvCam.begin();
    Serial.println("Set up OpenMV Cam.");
}

void move_arr_back(uint16_t* arr, int size) {
    for (int i = 1; i < size; ++i) {
        arr[i-1] = arr[i];
    }
    // Write last array entry with invalid value
    arr[size-1] = 3600;
}

void append_to_window(uint16_t received_cam_angle) {
    move_arr_back(angle_array, NUM_ANGLE_VALS);
    angle_array[NUM_ANGLE_VALS - 1] = received_cam_angle;
}

void get_angle() {
    uint16_t to_average[NUM_ANGLE_VALS] = {0};
    int green_left_count = 0;
    int green_right_count = 0;
    int turn_count = 0;
    int red_count = 0;
    for (int i = 0; i < NUM_ANGLE_VALS; ++i) {
        uint16_t curr = angle_array[i];
        if (curr != 3600) {
            green_left_count += curr == 900;
            green_right_count += curr == -900;
            turn_count += curr == 1800;
            red_count += curr == 3000;
        }
    }
    if (red_count >= 2) {
        cam_angle = 3000;
        is_red = true;
    }
    else if (turn_count >= 3) {
        cam_angle = 1800;
        green_left = green_right = true;
    }
    else if (green_left_count >= 3) {
        cam_angle = 900;
        green_left = true;
    }
    else if (green_right_count >= 3) {
        cam_angle = -900;
        green_right = true;
    } else {
        // Leave cam_angle unchanged instead of potentially changing it to a wrong value
    }
}