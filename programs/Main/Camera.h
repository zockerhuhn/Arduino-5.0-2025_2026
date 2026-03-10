#pragma once

void update_cam_data(void* in_data, size_t in_data_len) {
    if (in_data_len == 3 * sizeof(in_data) /*weird mismatch*/) {
        // Data is complete
        // Copying in_data buffer into receiving data structure
        // Serial.println("Writing received data to buffer");
        memcpy(&received_cam_data, in_data, sizeof(received_cam_data));
        
    }
    else {
        Serial.println("Invalid data length AAAAAAAAA" + String(in_data_len) + "/" + String(sizeof(in_data)));
        // memcpy(&received_cam_data, in_data, sizeof(received_cam_data));
        // Serial.println(String(received_cam_data.angle1) + " " + String(received_cam_data.angle2) + " " + String(received_cam_data.angle3) + " " + String(received_cam_data.main_angle) + " " + String(received_cam_data.received_cam_angle) + " " + String(received_cam_data.dist_to_center));
    }
}

void openmv_cam_setup() {
    // Calls update_cam_data when the OpenMV Cam requests it
    openMvCam.register_callback(F("update_cam_data"), update_cam_data);
    // Initialising the camera
    openMvCam.begin();
    Serial.println("Set up OpenMV Cam.");
}

void move_arr_back(int16_t* arr, int size) {
    for (int i = 1; i < size; ++i) {
        arr[i-1] = arr[i];
    }
    // Write last array entry with invalid value
    arr[size-1] = 360;
}

void append_to_window(int16_t angle) {
    move_arr_back(angle_array, NUM_ANGLE_VALS);
    angle_array[NUM_ANGLE_VALS - 1] = angle;
    // Serial.print("Window: ");
    // for (int i = 0; i < NUM_ANGLE_VALS; ++i) Serial.print(String(angle_array[i]) + " ");
    // Serial.print("\n");
}

void get_angle() {
    cam_angle = 0;
    int count = 0;
    if (received_cam_data.angle1 != 360) {
        cam_angle += received_cam_data.angle1;
        count++;
    }
    if (received_cam_data.angle2 != 360) {
        cam_angle += received_cam_data.angle2;
        count++;
    }
    if (received_cam_data.angle3 != 360) {
        cam_angle += received_cam_data.angle3;
        count++;
    }
    if (received_cam_data.main_angle != 360) {
        cam_angle += received_cam_data.main_angle;
        count++;
    }
    if (count != 0) cam_angle /= count;
    else cam_angle = 360;

    



    Serial.print("cam_angle: " + String(cam_angle) + "\t");
    
    int green_left_count = 0;
    int green_right_count = 0;
    int left_count = 0;
    int right_count = 0;
    int turn_count = 0;
    int red_count = 0;
    int invalid_count = 0;
    
    for (int i = 0; i < NUM_ANGLE_VALS; ++i) {
        int16_t curr = angle_array[i];
        green_left_count += (int)(curr == 90);
        green_right_count += (int)(curr == -90);
        left_count += (int)(curr == 391);
        right_count += (int)(curr == -391);
        turn_count += (int)(curr == 180);
        red_count += (int)(curr == 300);
        invalid_count += (int)(curr == 360);
    }


    if (red_count >= (int)(2 * NUM_ANGLE_VALS / 3)) {
        kreuzung_angle = 300;
        is_red = true;
    }
    else if (turn_count >= (int)(NUM_ANGLE_VALS / 2)) {
        kreuzung_angle = 180;
        green_left = green_right = true;
    }
    else if (green_left_count >= (int)(NUM_ANGLE_VALS / 3)) {
        kreuzung_angle = 90;
        green_left = true;
    }
    else if (green_right_count >= (int)(NUM_ANGLE_VALS / 3)) {
        kreuzung_angle = -90;
        green_right = true;
    } 
    else if (left_count >= (int)(NUM_ANGLE_VALS / 2)) {
        kreuzung_angle = 89; //391;
    }
    else if (right_count >= (int)(NUM_ANGLE_VALS / 2)) {
        kreuzung_angle = -89; //-391;
    }
    else if (invalid_count >= (int)(0.83333333 * NUM_ANGLE_VALS)) {
        kreuzung_angle = 360;
    }

    Serial.print("kreuzung_angle: " + String(kreuzung_angle) + "\n");
}