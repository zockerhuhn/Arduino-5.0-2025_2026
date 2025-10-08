#pragma once

void update_cam_data(void* in_data, size_t in_data_len) {
    if (in_data_len == sizeof(in_data)) {
        // Data is complete
        // Copying in_data buffer into receiving data structure
        Serial.println("Writing received data to buffer");
        memcpy(&received_cam_angle, in_data, sizeof(received_cam_angle));
        
    }
    else {
        Serial.println("Invalid data length AAAAAAAAA");
    }
}

void openmv_cam_setup() {
    // Calls update_cam_data when the OpenMV Cam requests it
    openMvCam.register_callback(F("update_cam_data"), update_cam_data);
    // Initialising the camera
    openMvCam.begin();
    Serial.println("Set up OpenMV Cam.");
}

bool isRed() {
    return cam_data.red;
}