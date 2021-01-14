from ..general_operations.trigonometry import rotational_matrix,convert_degrees_to_radians, convert_radians_to_degrees
from ..hector.constants import robot_center_x,robot_center_y
from math import atan,sin,cos
import numpy as np
import pandas as pd
import string
import sys
import re

# Adjusting offset to move circular magnet closer to OR far from rectangular magnet
def hexaPositionOffset(all_magnets):

    offset_distance = 0.0  # to be derived from excel file

    for i in all_magnets:

        if i.__class__.__name__ == 'rectangular_magnet' and i.index == 8:

            # getting the orientation of rectangular magnet with respect to North(up)
            ang = i.orientation

            # check to adjust angle within 0 to -360 range
            if ang < 360:
                ang = ang + 360

            # print('orientation_rectangular='+str(ang))
            azAngs = convert_radians_to_degrees(i.azAngs)
            # print('azAngs_rectangular'+str(azAngs))
            ang_azAngs = convert_radians_to_degrees(i.rectangular_magnet_input_orientation)
            # print('ang_azAngs_rectangular' + str(ang_azAngs))


    for i in all_magnets:

        if i.__class__.__name__ == 'circular_magnet' and i.index == 8:

            # adjusting the angle to ensure movement is about the rectangular magnet's centre axis
            angle_adjusted = 450+ang

            # check to adjust angle within 0 to 360 range
            if angle_adjusted > 360:
                angle_adjusted = angle_adjusted - 360

            rotation_matrix_circle = rotational_matrix(convert_degrees_to_radians(angle_adjusted))
            # subtracting offset distance moves circular magnet closer to rectangular
            i.center = (i.center[0] - rotation_matrix_circle[0][0] * offset_distance, \
                                      i.center[1] - rotation_matrix_circle[0][1] * offset_distance)
            i.offset = offset_distance

            # print('orientation_circular=' + str(ang))
            azAngs = convert_radians_to_degrees(i.azAngs)
            # print('azAngs_circular=' + str(azAngs))


    return all_magnets


# thermal expansion related offset which will move the magnet pair as a whole based on certain coefficients
def magnetPair_radialPositionOffset(plate_file):

    ## read off a certain table from csv file or derived off some equations and calculations to identify the magnet pair
    ## to be adjusted with the determined offset values, VARIABLE 'offset_distance' used for testing function's usability

    offset_radialDistance = 20  # to be derived

    # store magnet pair index and offset distance accordingly, to be derived
    magnetPair_offset = []
    # magnetPair_offset = [(14,-30),(4,-30),(12,-30),(9,-30)] # +ve value makes radial outward movement, and -ve value for radial inward movement

    return plate_file, magnetPair_offset

# radial Position offset being adjusted in the extract_data.py file before all_magnets are being produced
def radialPositionOffset(list_of_probes,magnetPair_offset):

    for item in magnetPair_offset:
        for each_probe in list_of_probes:
            if item[0] == each_probe.index:

                print(each_probe.circular_magnet_center)

                theta = atan(each_probe.circular_magnet_center[1] / each_probe.circular_magnet_center[0])
                # x values in positive range and -ve range move in opposite radial directions, so this step ensure same direction movement
                if each_probe.circular_magnet_center[0] >= 0:
                    each_probe.circular_magnet_center = (each_probe.circular_magnet_center[0] + (cos(theta) * item[1]), \
                                                         each_probe.circular_magnet_center[1] + (sin(theta) * item[1]))
                else:
                    each_probe.circular_magnet_center = (each_probe.circular_magnet_center[0] - (cos(theta) * item[1]), \
                                                         each_probe.circular_magnet_center[1] - (sin(theta) * item[1]))
                print(each_probe.circular_magnet_center)

    return list_of_probes