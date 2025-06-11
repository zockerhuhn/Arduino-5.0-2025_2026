import sensor
import time
import math
import rpc
import struct

## Arduino-Camera-Communication
interface = rpc.rpc_uart_master(baudrate=115200)

# Flag controlling if data will be transferred
send_data = False

def send_to_arduino(*vals_to_send):
    # Send the data to the arduino
    # Angle is saved as an integer dezidegree(?),
    # which means we have a precision of 1/10 for an angle in degrees
    result = interface.call("sent", struct.pack("<HHHH", *vals_to_send))
    # Check if the arduino answers something valid
    if result is not None and len(result):
        return True
    print("Response invalid!", result)
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

# Threshold which pixel brightness counts as black
GRAYSCALE_THRESHOLD = [(0, 90)]

# Threshold which RGB values are considered green (l_lo, l_hi, a_lo, a_hi, b_lo, b_hi)
GREEN_THRESHOLD = [(20, 70, -40, -10, 0, 35)]

# Threshold how many pixels a blob must have to be relevant
pixel_threshold = 80
density_threshold = 0.45

## Image constants
width, height = 160, 120
# Height from which the image will be processed
cut_height = int(2/3 * height)
img_center_x, img_center_y = 80, 60

## Adjustable ROI-parameters

num_kreuzung_segments = 8
roi_width = int(width / num_kreuzung_segments)

## ROI at the top of the image

# ROI is arranged in base x, base y, width, height
top_roi = [
    (x_level, 0, roi_width, int(1/3 * height)) for x_level in range(0, width, roi_width)
]

## ROI in the middle

mid_roi = [
    (x_level, int(1/3 * height), roi_width, int(1/3 * height)) for x_level in range(0, width, roi_width)
]

def find_avg_center(roi, blob_array):
    """
    Calculates the average center of every largest blob centroid coordinate
    in the specified list of ROIs.
    """
    for i, r in enumerate(roi):
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
    The slope is returned in radians.
    """
    line_mag = math.sqrt((pos1[0] - pos2[0])**2 + (pos1[1] - pos2[1])**2)
    if line_mag > 0:
        return -math.asin((pos1[0] - pos2[0]) / line_mag)
    return None

# Array that saves the blobs that are found in the specified roi
blob_array_top = [0] * len(top_roi)
blob_array_mid = [0] * len(mid_roi)

# Array saving ...
vertical_line_array = [0] * num_kreuzung_segments

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
    clock.tick()  # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot().crop(roi=(0, 0, width, cut_height))

    #continue

    # Create green mask
    for blob in img.find_blobs(GREEN_THRESHOLD, pixels_threshold=200, area_threshold=200):
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

    img.to_grayscale().binary(GRAYSCALE_THRESHOLD)

    if show_line_following:
        # Draw circle in the center
        img.draw_circle(img_center_x, img_center_y, 5, (50, 50, 50), 1, True)

    if debug_print:
        print("Finding average center in top ROI")
    # top ROI
    blob_array_top = [False] * len(top_roi)
    top_values = find_avg_center(top_roi, blob_array_top)
    centroid_sum_x, centroid_sum_y, weight_sum_top = top_values

    if debug_print:
        print("Found center in top ROI!")

    if weight_sum_top:
        # Calculate the slope of that line
        #top_line_angle = calculate_line_slope((first_blob.cx(), first_blob.cy()), (last_blob.cx(), last_blob.cy()))
        #if top_line_angle != None:
        #    if top_line_angle > math.pi / 3 and span >= 6:
        #        print("Probably kreuzung! (top)")

        if show_line_following:
            # Draw the average center cross
            img.draw_cross(centroid_sum_x, centroid_sum_y, angled_line_color, size=10)

        top_center_pos = (centroid_sum_x, centroid_sum_y)

    if debug_print:
        print("Finding average center in middle ROI")
    # mid ROI
    blob_array_mid = [False] * len(mid_roi)
    mid_values = find_avg_center(mid_roi, blob_array_mid)
    centroid_sum_x, centroid_sum_y, weight_sum_mid = mid_values

    if debug_print:
        print("Found center in middle ROI!")

    if weight_sum_mid:
        #mid_line_angle = calculate_line_slope((first_blob.cx(), first_blob.cy()), (last_blob.cx(), last_blob.cy()))
        #if mid_line_angle != None:
        #    if mid_line_angle > math.pi / 3 and span >= 6:
        #        print("Probably kreuzung! (mid)")

        if show_line_following:
            img.draw_cross(centroid_sum_x, centroid_sum_y, angled_line_color, size=10)

        mid_center_pos = (centroid_sum_x, centroid_sum_y)


    # Combining knowledge
    if weight_sum_top and weight_sum_mid:
        if show_line_following:
            img.draw_line(*mid_center_pos, *top_center_pos, color=angled_line_color, thickness=2)

        if debug_print:
            print("Calculating angle of line...")
        # TODO the line slope is not too accurate because of the ROIfication of the image.
        # Try to fix this pls
        line_angle_rad = calculate_line_slope(mid_center_pos, top_center_pos)

        if debug_print_important:
            if line_angle_rad != None:
                print("Angle:", math.degrees(line_angle_rad))
            else:
                print("Line has no length, angle is undefined!")

    if debug_print:
        print("Finding top-middle connections...")

    vertical_line_array = [0] * len(vertical_line_array)
    for i, top_blob in enumerate(blob_array_top):
        mid_blob = blob_array_mid[i]
        if top_blob != False and mid_blob != False:
            if show_blob_info:
                img.draw_line(top_blob.cx(), top_blob.cy(), mid_blob.cx(), mid_blob.cy(), blob_color, 2)
            line_length = math.sqrt((top_blob.cx() - mid_blob.cx())**2 + (top_blob.cy() - mid_blob.cy())**2)
            vertical_line_array[i] = line_length

    if debug_print:
        for i, down_line_length in enumerate(vertical_line_array):
            if down_line_length:
                print("↓", end=", ")
            else:
                print("-", end=", ")
        print("\n")


    # kreuzungsdetection:
    # when line splits its a kreuzung
    # which means: blobs left and right are multiple consecutive blobs
    # we know where the split must approximately be the location
    # of the vertical lines we just found

    # Determine where the vertical line to the kreuzung is
    if any(vertical_line_array): # Line only exists if down lines are seen

        if debug_print:
            print("Calculate x-position of the presumably followed line (vertical)")

        vertical_line_pos = 0
        total_length = 0
        for i, down_line_length in enumerate(vertical_line_array):
            vertical_line_pos += i * down_line_length
            total_length += down_line_length
        vertical_line_pos /= total_length

        """
        first_line_pos = None
        last_line_pos = None
        for i, down_line_length in enumerate(vertical_line_array):
            if down_line_length:
                if not first_line_pos:
                    first_line_pos = i
                last_line_pos = i
        vertical_line_pos = (first_line_pos + last_line_pos) / 2
        """

        if debug_print:
            print("Calculated vertical line x-position:", vertical_line_pos)

        vertical_line_range = (math.floor(vertical_line_pos), math.ceil(vertical_line_pos))

        if debug_print:
            print(vertical_line_range)

        if show_blob_info:
            img.draw_line(int((vertical_line_range[0] + 0.5) * width / num_kreuzung_segments), 0, int((vertical_line_range[0] + 0.5) * width / num_kreuzung_segments), height, color = blob_color, thickness = 3)
            img.draw_line(int((vertical_line_range[1] + 0.5) * width / num_kreuzung_segments), 0, int((vertical_line_range[1] + 0.5) * width / num_kreuzung_segments), height, color = blob_color, thickness = 3)

        # Find split from the line

        # First find if a line exists left of the range
        left_line_length = 0
        for i in range(vertical_line_range[0], 0, -1):
            if blob_array_top[i] and blob_array_top[i-1]:
                left_line_length += 1
        if debug_print:
            print("Length of left line:", left_line_length)

        # Then find if a line exists right of the range
        right_line_length = 0
        for i in range(vertical_line_range[1] + 1, num_kreuzung_segments):
            if blob_array_top[i - 1] and blob_array_top[i]:
                right_line_length += 1
        if debug_print:
            print("Length of right line:", right_line_length)

        if debug_print_important:
            if left_line_length >= 2 and right_line_length >= 2:
                # Kreuzung detected
                for blob in green_blobs:
                # TODO ensure blobs are followed by black!!!
                    if blob.cx() < vertical_line_pos * roi_width:
                        blob_left = 1
                    else:
                        blob_right = 1
                if blob_left and blob_right:
                    print("Turn!")
                elif blob_left:
                    print("Left!")
                elif blob_right:
                    print("Right!")
                else:
                    print("Kein grün :(")
            else:
                print("Keine Kreuzung gefunden.")

    # Communicating with robot

    if send_data:
        # For now, only talk if there are actual infos
        if weight_sum_top and weight_sum_mid:
            # Send the angle and information about the left and right green spots
            # Remember that the green spots will only get checked if a kreuzung is present!
            angle = int(math.degrees(line_angle_rad) * 10)
            send_to_arduino(angle, blob_left, blob_right, vertical_line_pos - 4)
        else:
            # Angle is "invalid" as 60000
            send_to_arduino(60000, blob_left, blob_right, vertical_line_pos - 4)

    if debug_print:
        print("FPS:", clock.fps())  # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
