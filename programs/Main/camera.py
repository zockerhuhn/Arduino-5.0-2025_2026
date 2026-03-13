import sensor
import time
import math
import rpc
import struct
import pyb

# OpenMV M7

# LEDs
red_led = pyb.LED(1)
green_led = pyb.LED(2)
blue_led = pyb.LED(3)

## Arduino-Camera-Communication
interface = rpc.rpc_uart_master(baudrate=115200)

# Flag controlling if data will be transferred
send_data = True

def send_to_arduino(vals):
    # Send the data to the arduino
    # an angle is saved as an integer degree

    # 360: invalid
    # 300: red
    # 90: left green
    # -90: right green
    # 180: turn
    # -90<x<90: angle
    # 391: turn left immediately
    # -391: turn right immediately

    print(*vals)
    if send_data:
        #green_led.on()
        result = interface.call("update_cam_data", struct.pack("<hhhhhhhhh", *vals))
        # Check if the arduino answers something valid
        if result is not None:
            #red_led.off()
            return True
        print("Response invalid!", result)
        #red_led.on()
    return False

## Debugging flags

# Printing debugging info
debug_print = False
debug_print_important = True

# Showing lines and blobs in the image
show_blob_info = True
# Showing basic found line and center
show_line_following = True

## Thresholds

# Tolerance for how much green blobs can "overlap" with vertical line
tolerance = 0.05

# Threshold which pixel brightness counts as black
GRAYSCALE_THRESHOLD = [(0, 50)] # once at maybe 90 or something, but lower is generally better

# Threshold which RGB values are considered green (l_lo, l_hi, a_lo, a_hi, b_lo, b_hi)
GREEN_THRESHOLD = [(20, 70, -40, -10, 0, 35)]

RED_THRESHOLD = [(10, 57, 28, 63, 15, 59)]
# (28, 38, -1, 62, 12, 59)

# Threshold how many pixels a blob must have to be relevant
pixel_threshold = 80
density_threshold = 0.45

## Image constants
width, height = 160, 120
# Height from which the image will be processed
cut_height = int(2/3 * height) + 2

img_center_x, img_center_y = 80, 60
start_pos_x = 80#103 # 80
start_pos_y = 80


## Adjustable ROI-parameters
num_roi_rows = 3
num_kreuzung_segments = 8
roi_width = int(width / num_kreuzung_segments)
roi_height = int(cut_height / num_roi_rows)

## 2D map of ROIs

ROIs = []

for i in range(num_roi_rows):
    # ROI is arranged in base x, base y, width, height
    ROIs.append([
        (x_level, int((i * cut_height)/num_roi_rows), roi_width, int(cut_height/num_roi_rows)) for x_level in range(0, width, roi_width)
    ])

def calculate_blob_array(roi_row) -> list[int]:
    """
    Calculate the array of blobs in the given roi-row
    """
    blob_array = [False] * len(roi_row)
    for i, r in enumerate(roi_row):
        blobs = img.find_blobs(
            [(255, 255)], roi=r, merge=True, pixels_threshold = pixel_threshold
        )
        if show_blob_info:
            img.draw_rectangle(*r, color=(255,0,0))
        if blobs:
            best_blob = max(blobs, key=lambda b: b.pixels())
            if best_blob.density() > density_threshold:
                if show_blob_info:
                    img.draw_edges(best_blob.min_corners(), blob_color)
                    img.draw_cross(best_blob.cx(), best_blob.cy(), blob_color)
                blob_array[i] = best_blob
    return blob_array

def find_avg_center(roi, blob_array):
    """
    Calculates the average center of every largest blob centroid coordinate
    in the specified list of ROIs.
    """
    centroid_sum_x = 0
    centroid_sum_y = 0
    weight_sum = 0
    last_blob = blob_array[0]
    for i, blob in enumerate(blob_array[1:]):
        if blob:
            if last_blob and show_blob_info:
                img.draw_line(blob.cx(), blob.cy(), last_blob.cx(), last_blob.cy(), blob_color, 2)
            last_blob = blob
            weight_sum += blob.pixels()
            centroid_sum_x += blob.pixels() * blob.cx()
            centroid_sum_y += blob.pixels() * blob.cy()
        else:
            last_blob = None

    if weight_sum:
        centroid_sum_x = int(centroid_sum_x/weight_sum)
        centroid_sum_y = int(centroid_sum_y/weight_sum)

    return centroid_sum_x, centroid_sum_y, weight_sum

def calculate_line_slope(pos1, pos2) -> int | None:
    """
    Calculates the slope of the line connecting the given positions.
    The slope is returned in degrees.
    """
    line_mag = math.sqrt((pos1[0] - pos2[0])**2 + (pos1[1] - pos2[1])**2)
    if line_mag > 0:
        return math.degrees(-math.asin((pos1[0] - pos2[0]) / line_mag))
    return None

def calculate_modified_line_slope(pos1, pos2) -> int | None:
    """
    Calculates the slope of the line connecting the average of the given positions with
    the "starting position" of the robot.
    """
    avg_pos = []
    avg_pos.append((pos1[0] + pos2[0]) / 2)
    avg_pos.append((pos1[1] + pos2[1]) / 2)

    line_mag = math.sqrt((avg_pos[0] - start_pos_x)**2 + (avg_pos[1] - start_pos_y)**2)
    if line_mag > 0:
        return -math.asin((avg_pos[0] - start_pos_x) / line_mag)
    return None

# Array that saves the blobs that are found in the specified roi
blob_arrays = [[0] * len(roi) for roi in ROIs]

# Array saving all found green blobs
green_blobs = []

## Drawing stuff
blob_color = (100, 255, 100)
angled_line_color = (0, 0, 0)


# Camera setup...
sensor.reset()  # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565)  # use grayscale.
sensor.set_framesize(sensor.QQVGA)  # use QQVGA for speed.
sensor.skip_frames(time=2000)  # Let new settings take effect.
sensor.set_auto_gain(False)  # must be turned off for color tracking
sensor.set_auto_whitebal(False)  # must be turned off for color tracking
clock = time.clock()  # Tracks FPS.

while True:
    green_blobs = []
    blob_left = 0
    blob_right = 0
    red_line_detected = False
    clock.tick()  # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot()

    # Check for red
    for blob in img.find_blobs(RED_THRESHOLD, roi=(0, 0, width, int(5/6 * height)), pixels_threshold=1000, area_threshold=800):
        # For red it isn't important if the line following gets broken, because it shouldn't run anyway
        img.draw_edges(blob.min_corners(), blob_color)
        img.draw_cross(blob.cx(), blob.cy(), blob_color)
        #img.draw_rectangle(*blob.rect(), color = (255,255,255), fill = True)

        #print(blob.area(), blob.pixels())

        # TODO be a bit more careful about declaring red,
        # but seems fine for now
        red_line_detected =  True

    #continue

    # Create green mask, only cutting out a small region near the wheels
    for blob in img.find_blobs(GREEN_THRESHOLD, roi=(0, 0, width, int(5/6 * height)), pixels_threshold=200, area_threshold=200):
        #original_image.draw_edges(blob.min_corners(), blob_color)
        #original_image.draw_cross(blob.cx(), blob.cy(), blob_color)
        green_blobs.append(blob)
        #original_image.draw_rectangle(*blob.rect(), color = (255,255,255), fill = True)
        img.flood_fill(
            blob.cx(),
            blob.cy(),
            seed_threshold=0.09,
            floating_thresholds=0.5,
            color=(255, 255, 255),
            invert=False,
            clear_background=False,
        )

    #continue

    # Crop the image and convert it to grayscale
    img.crop(roi=(0, 0, width, cut_height))
    img.to_grayscale().binary(GRAYSCALE_THRESHOLD)

    #continue

    if show_line_following:
        # Draw circle in the center
        img.draw_circle(img_center_x, img_center_y, 5, (50, 50, 50), 1, True)

    # Calculate centroids

    blob_arrays = [calculate_blob_array(roi_row) for roi_row in ROIs]

    num_blobs = 0
    for blob_array in blob_arrays:
        for blob in blob_array:
            if blob:
                num_blobs += 1

    # Find the vertical connections between roi rows
    v_connections = [[False] * num_kreuzung_segments] * (num_roi_rows - 1)
    for i in range(num_roi_rows - 1):
        for j, b1 in enumerate(blob_arrays[i]):
            b2 = blob_arrays[i+1][j]
            if b1 and b2:
                #line_length = math.sqrt((top_blob.cx() - mid_blob.cx())**2 + (top_blob.cy() - mid_blob.cy())**2)
                #vertical_line_array[i] = line_length

                v_connections[i][j] = (b1.h() * b2.h())**2 * (-1 * abs(2 * i / (num_kreuzung_segments) - 1) + 1) + 1
                #print(v_connections[i][j], end=" ")

                if show_blob_info:
                    img.draw_line(b1.cx(), b1.cy(), b2.cx(), b2.cy(), blob_color, 2)
            """else:
                print("None", end=" ")
        print("")"""
    # Find the vertical line that probably shows where the robot should be

    if any(v_connections[1]):
        if debug_print:
            print("Calculate x-position of the presumably followed line (vertical)")

        vline_pos = 1
        total_length = 0
        for i, total_line_length in enumerate(v_connections[1]):
            vline_pos += i * total_line_length
            total_length += total_line_length
        vline_pos /= total_length
        vline_pos += 0.5

        if debug_print:
            print("Calculated vertical line x-position:", vline_pos)

        vline_range = (math.floor(vline_pos - 0.5), math.ceil(vline_pos - 0.5))

        if debug_print:
            print(vline_range)

        if show_blob_info:
            img.draw_line(int(vline_pos * width / num_kreuzung_segments), cut_height, int(vline_pos * width / num_kreuzung_segments), cut_height - 2 * roi_height, color = blob_color, thickness = 3)
    else:
        vline_range = None

    # Try to calculate centroids from only connected blobs, but if there are not enough, take all blobs
    centroids = [None] * num_roi_rows
    weights = [0] * num_roi_rows
    total_counted_blobs = 0
    if any(v_connections[1]):
        # Find blob nearest to vline
        next_layer_blob_idxs = set()
        for i in range(num_kreuzung_segments):
            if vline_range[0] + i < num_kreuzung_segments and vline_range[0] + i >= 0:
                if blob_arrays[num_roi_rows - 1][vline_range[0] + i]:
                    next_layer_blob_idxs.add(vline_range[0] + i)
                    break
            elif vline_range[0] - i >= 0 and vline_range[0] - i < num_kreuzung_segments:
                if blob_arrays[num_roi_rows - 1][vline_range[0] - i]:
                    next_layer_blob_idxs.add(vline_range[0] - i)
                    break

        if len(next_layer_blob_idxs):
            # Go through the blobs of each row and compute the centroid
            for row_idx_1, blob_array in enumerate(reversed(blob_arrays)):
                row_idx = num_roi_rows - row_idx_1 - 1
                #print(row_idx, next_layer_blob_idxs)
                if len(next_layer_blob_idxs) == 0:
                    break
                curr_blob_idx = next_layer_blob_idxs.pop()
                # Collect all connected blob indices of this row in a set
                relevant_blob_idxs_per_row = set()
                for elem in next_layer_blob_idxs:
                    relevant_blob_idxs_per_row.add(elem)
                next_layer_blob_idxs.clear()
                for i in range(num_kreuzung_segments - curr_blob_idx):
                    if row_idx != 0 and v_connections[row_idx - 1][curr_blob_idx + i] != False:
                        next_layer_blob_idxs.add(curr_blob_idx + i)
                        #print("next: ", next_layer_blob_idxs)
                    if blob_array[curr_blob_idx + i]:
                        relevant_blob_idxs_per_row.add(curr_blob_idx + i)
                    else:
                        break
                for i in range(curr_blob_idx + 1):
                    if row_idx != 0 and v_connections[row_idx - 1][curr_blob_idx - i] != False:
                        next_layer_blob_idxs.add(curr_blob_idx - i)
                        #print("next: ", next_layer_blob_idxs)
                    if (blob_array[curr_blob_idx - i]):
                        relevant_blob_idxs_per_row.add(curr_blob_idx - i)
                    else:
                        break
                #print(relevant_blob_idxs_per_row)
                total_counted_blobs += len(relevant_blob_idxs_per_row)
                # Compute centroids
                weight_sum = 0
                centroid_sum_x = 0
                centroid_sum_y = 0
                for idx in relevant_blob_idxs_per_row:
                    blob = blob_array[idx]
                    if blob:
                        weight_sum += blob.pixels()
                        centroid_sum_x += blob.pixels() * blob.cx()
                        centroid_sum_y += blob.pixels() * blob.cy()
                if weight_sum != 0:
                    centroid_sum_x = int(centroid_sum_x/weight_sum)
                    centroid_sum_y = int(centroid_sum_y/weight_sum)
                    centroids[row_idx] = (centroid_sum_x, centroid_sum_y)
                    weights[row_idx] = weight_sum


    # If not that many blobs were counted
    if num_blobs == 0 or total_counted_blobs / num_blobs < 0.3:
        # Find centers for each ROI-row by simply summing everything:
        for i, roi in enumerate(ROIs):
            centroid_sum_x, centroid_sum_y, weight_sum = find_avg_center(roi, blob_arrays[i])
            if weight_sum:
                centroids[i] = (centroid_sum_x, centroid_sum_y)
                weights[i] = weight_sum

    for centroid in centroids:
        #print("centroid: ", centroid)
        if show_line_following and centroid != None:
            # Draw the average center cross
            img.draw_cross(*centroid, angled_line_color, size=10)

    angles = []
    # Combining knowledge
    if all(centroids):
        centroids.append((start_pos_x, start_pos_y))
        for i in range(len(centroids) - 2, -1, -1):
            if show_blob_info:
                img.draw_line(*centroids[i], *centroids[i+1], color=angled_line_color)
            angles.append(int(calculate_line_slope(centroids[i], centroids[i+1])))
        centroids.remove((start_pos_x, start_pos_y))
    else:
        centroids.append((start_pos_x, start_pos_y))
        for i in range(len(centroids) - 2, -1, -1):
            if centroids[i] and centroids[i+1]:
                if show_blob_info:
                    img.draw_line(*centroids[i], *centroids[i+1], color=angled_line_color)
                angles.append(int(calculate_line_slope(centroids[i], centroids[i+1])))
            else:
                angles.append(360)
        centroids.remove((start_pos_x, start_pos_y))



    # IDEA:
    # find angle of main line by finding angle between 0 and 2 center blob, only while
    # there are at most 2 connections between 1 and 2 (and 0 and 1?)

    # what use is it to know the angle of main line?
    # well, when we encounter a crossing, we'll (probably) know if we're askew
    # thats something, so lets just do that

    if blob_arrays[1].count(None) <= num_kreuzung_segments - 3 and centroids[0] and centroids[num_roi_rows - 1] and v_connections[1].count(False) >= num_kreuzung_segments - 3 and any(v_connections[1]):
        main_angle = calculate_line_slope(centroids[0], centroids[num_roi_rows - 1])
        if show_blob_info:
            img.draw_line(*centroids[0], *centroids[num_roi_rows - 1], color=angled_line_color)
        #print("Main angle:", main_angle)
    else:
        main_angle = 360
    angles.append(int(main_angle))



    # kreuzungsdetection:
    # when line splits its a kreuzung
    # which means: blobs left and right are multiple consecutive blobs
    # we know where the split must approximately be the location
    # of the vertical lines we just found

    # Determine where the vertical line to the kreuzung is
    # Look at the bottom connections only
    max_left_line_length = 0;
    max_right_line_length = 0;
    if any(v_connections[1]): # Line only exists if down lines are seen

        # Find split from the line
        # The following only searches for a completely horizontal split and therefore
        # only works if the robot reaches a crossing at approximately the correct angle
        # maybe TODO also make it work for wrong crossing-reaches

        # First find if a line exists left of the range
        line_lengths = []
        for blob_array in blob_arrays[1:]:
            left_line_length = 0
            for i in range(vline_range[0], 0, -1):
                if i >= 1 and i < num_kreuzung_segments:
                    if blob_array[i] and blob_array[i-1]:
                        left_line_length += 1
            if debug_print:
                print("Length of left line:", left_line_length)

            # Then find if a line exists right of the range
            right_line_length = 0
            for i in range(vline_range[1] + 1, num_kreuzung_segments):
                if i >= 1 and i < num_kreuzung_segments:
                    if blob_array[i] and blob_array[i-1]:
                        right_line_length += 1
            if debug_print:
                print("Length of right line:", right_line_length)

            line_lengths.append((left_line_length, right_line_length))
            max_left_line_length = max(max_left_line_length, left_line_length)
            max_right_line_length = max(max_right_line_length, right_line_length)


        # If lines of blobs continue on both sides or
        # if a line of blobs is found either left or right and a single blob continues at the top,
        # it's a crossing

        # TODO adequate kreuzungsdetection

        if line_lengths[0][0] >= 2 and line_lengths[0][1] >= 2:
            # Kreuzung on the top detected
            for blob in green_blobs:
                if blob.cy() > centroids[1][1]:
                    # TODO this tolerance is pretty important but can lead to bad stuff happening
                    if (blob.cx() - tolerance * roi_width < vline_pos * roi_width) and (blob.cx() + tolerance * roi_width > vline_pos * roi_width):
                        if line_lengths[0][0] > line_lengths[0][1]:
                            blob_left = 1
                        else:
                            blob_right = 1
                    elif blob.cx() - tolerance * roi_width < vline_pos * roi_width:
                        blob_left = 1
                    elif blob.cx() + tolerance * roi_width > vline_pos * roi_width:
                        blob_right = 1
        elif vline_range[0] < 8 and vline_range[1] < 8:
            if (blob_arrays[0][vline_range[0]] or blob_arrays[0][vline_range[1]]):
                # Left side kreuzung:
                if line_lengths[0][0] >= 3 or line_lengths[1][0] >= 3:
                    for blob in green_blobs:
                        if centroids[num_roi_rows-1]:
                            if blob.cy() > centroids[num_roi_rows-1][1]:
                                if (blob.cx() - tolerance * (roi_width + line_lengths[-1][0] * 3) < vline_pos * roi_width):
                                    blob_left = 1
                # Right side kreuzung
                elif line_lengths[0][1] >= 3 or line_lengths[1][1] >= 3:
                    for blob in green_blobs:
                        if centroids[num_roi_rows-1]:
                            if blob.cy() > centroids[num_roi_rows-1][1]:
                                if (blob.cx() + tolerance * (roi_width + line_lengths[-1][1] * 3) > vline_pos * roi_width):
                                    blob_right = 1
    elif centroids[num_roi_rows - 1]:
        max_left_line_length = 0
        for i in range(int(8 * centroids[num_roi_rows - 1][0] / width), 0, -1):
            if i >= 1 and i < num_kreuzung_segments:
                if blob_arrays[num_roi_rows - 1][i] and blob_arrays[num_roi_rows - 1][i-1]:
                    max_left_line_length += 1
        max_right_line_length = 0
        for i in range(int(8 * centroids[num_roi_rows - 1][0] / width) + 1, num_kreuzung_segments):
            if i >= 1 and i < num_kreuzung_segments:
                if blob_arrays[num_roi_rows - 1][i] and blob_arrays[num_roi_rows - 1][i-1]:
                    max_right_line_length += 1
    # Communicating with robot

    angle = 360
    # Send the angle and information about the left and right green spots
    # Remember that the green spots will only get checked if a kreuzung is present!

    # angle variable ranges from -90 to 90 (tho never actually the edges bc of how we calculate angles),
    # so any other values are invalid
    # We'll "reserve" some of these values to use for the kreuzung turns
    if (blob_left and blob_right):
        angle = 180
    elif blob_left:
        angle = 90
    elif blob_right:
        angle = -90
    if red_line_detected:
        angle = 300

    angles.append(angle)

    if centroids and centroids[num_roi_rows - 1]:
        angles.append(int(centroids[num_roi_rows - 1][0]) - start_pos_x)
    else:
        angles.append(360)
    angles.append(max_left_line_length)
    angles.append(max_right_line_length)
    angles.append(sum(weights))

    send_to_arduino(angles)

    if debug_print:
        print("FPS:", clock.fps())  # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
