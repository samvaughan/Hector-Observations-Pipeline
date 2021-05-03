from ..general_operations.trigonometry import rotational_matrix,convert_degrees_to_radians, convert_radians_to_degrees
from ..hector.constants import robot_center_x,robot_center_y,circular_rectangle_magnet_center_distance
from ..hector.probe import probe
from math import atan,sin,cos
import numpy as np
import pandas as pd
import csv
import string
import sys
import re

# Adjusting offset to move circular magnet closer to OR far from rectangular magnet
def hexaPositionOffset(all_magnets,offsetFile):

    # read offset excel file as a dataframe
    df = pd.read_excel(offsetFile,usecols=['Name','P','Q'])

    # drop the rows with NA values in Q column
    df.dropna(subset=['Q'], inplace=True)

    # omit the information row via string length check
    df_adjusted = df[df['Name'].str.len() <= 5]

    # reset the index of the adjusted dataframe with changes
    df_adjusted = df_adjusted.reset_index(drop=True)
    df_adjusted.index = df_adjusted.index + 1
    df_final = df_adjusted

    print(df_final)

    # create dictionaries to store the offset values of P and Q, and ang to store rectangular magnet orientation
    ang = {}
    offset_distance_P = {}
    offset_distance_Q = {}

    # loop through all_magnets list and offset dataframe to store offset values and rectangular magnet
    # orientation values in dictionary with key as index
    for i in all_magnets:

        for j in range(1,len(df_final)+1):

            if i.__class__.__name__ == 'rectangular_magnet' and i.hexabundle == df_final['Name'][j]:

                # getting the orientation of rectangular magnet with respect to North(up)
                ang[i.index] = i.orientation

                # storing the offset values in dictionary from the offset dataframe
                offset_distance_P[i.index] = df_final['P'][j] / 1000
                offset_distance_Q[i.index] = df_final['Q'][j] / 1000

                print('Index: '+str(i.index)+' , Hexabundle: '+str(i.hexabundle)+' , offset_P: '+str(df_final['P'][j]/1000))

                # print('orientation_rectangular = '+str(ang))
                # azAngs = convert_radians_to_degrees(i.azAngs)
                # print('azAngs_rectangular = '+str(azAngs))
                # ang_azAngs = convert_radians_to_degrees(i.rectangular_magnet_input_orientation)
                # print('ang_azAngs_rectangular = ' + str(ang_azAngs))
                #
                # rectangular_centre = i.center
                # print(rectangular_centre)

    print(ang)
    print(offset_distance_P)
    print(offset_distance_Q)

    print('HEREEEEEEE')
    print(range(len(all_magnets)))

    # calculate and make the changes to x and y positions of circular magnets as per offset values
    for i in range(len(all_magnets)):

        if all_magnets[i].__class__.__name__ == 'circular_magnet':

            print(all_magnets[i].index)
            print('Circular Before'+str(all_magnets[i].center))
            print('Offset P = '+str(offset_distance_P[all_magnets[i].index]))
            print('Offset Q = '+str(offset_distance_Q[all_magnets[i].index]))

            # adjusting the angle to ensure movement is about the rectangular magnet's centre axis
            angle_adjusted = 450 + ang[all_magnets[i].index]

            # check to adjust angle within 0 to 360 range
            if angle_adjusted > 360:
                angle_adjusted = angle_adjusted - 360

            # rotation matrix required for the offset adjustments of the circular magnets
            rotation_matrix_circle = rotational_matrix(convert_degrees_to_radians(angle_adjusted))

            # subtracting offset distance moves circular magnet in opposite direction to rectangular magnet
            # (P-parallel movement to rectangular magnet)
            all_magnets[i].center = (all_magnets[i].center[0] - rotation_matrix_circle[0][0] * offset_distance_P[all_magnets[i].index], \
                                     all_magnets[i].center[1] - rotation_matrix_circle[0][1] * offset_distance_P[all_magnets[i].index])

            # subtracting offset distance moves circular magnet downwards with respect to the rectangular magnet on its right side
            # (Q-perpendicular movement to rectangular magnet)
            all_magnets[i].center = (all_magnets[i].center[0] - rotation_matrix_circle[1][0] * offset_distance_Q[all_magnets[i].index], \
                                     all_magnets[i].center[1] - rotation_matrix_circle[1][1] * offset_distance_Q[all_magnets[i].index])



            # storing offset values in all_magnets list for final output file
            all_magnets[i].offset_P = offset_distance_P[all_magnets[i].index]
            all_magnets[i].offset_Q = offset_distance_Q[all_magnets[i].index]

            print('orientation_circular = ' + str(all_magnets[i].orientation))

            # recalculating the circular magnet orientation with offset adjusted centre coordinates
            all_magnets[i].circular_magnet_center = all_magnets[i].center
            all_magnets[i].orientation = probe.calculate_circular_magnet_orientation(all_magnets[i])

            print('Updated orientation_circular = ' + str(all_magnets[i].orientation))
            print('Circular After:'+str(all_magnets[i].center))

            # updating plotting view for Robot file centre coordinates
            all_magnets[i].view_y = all_magnets[i].center[0]
            all_magnets[i].view_x = - all_magnets[i].center[1]

    print('\n\n\n')

    # recalculate positions of x and y for rectangular magnets as per adjusted circular magnet coordinates
    for i in range(len(all_magnets)):

        if all_magnets[i].__class__.__name__ == 'rectangular_magnet':

            for j in range(len(all_magnets)):

                if all_magnets[j].__class__.__name__ == 'circular_magnet' and all_magnets[i].index == all_magnets[j].index:

                    print(all_magnets[i].index)
                    print('BEFORE:'+str(all_magnets[i].center))

                    # circular magnet centre is stored for rectangular magnets as well for calculation of center coordinates
                    all_magnets[i].circular_magnet_center = all_magnets[j].center
                    all_magnets[i].circular_rectangle_magnet_center_distance = circular_rectangle_magnet_center_distance

                    # recalculate rectangular magnet center coordinates as per updated circular magnet center
                    all_magnets[i].center = probe.calculate_rectangular_magnet_center_coordinates(all_magnets[i])
                    print('AFTER:'+str(all_magnets[i].center))

    print("DONEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE")
    # angle = 90 + convert_radians_to_degrees(atan(abs(rectangular_centre[1]-circular_centre[1])/abs(rectangular_centre[0]-circular_centre[0])))
    # print(angle)

    return all_magnets

## Created as a standalone function for the robot, so should not be required to implement in this pipeline
# thermal expansion related offset which will move the magnet pair as a whole based on certain coefficients
def magnetPair_radialPositionOffset(plate_file):

    offset_radialDistance = 20  # to be derived

    # T_observed > T_configured then radial inward movement by magnet
    # T_observed < T_configured then radial outward movement by magnet
    # ΔT = T_configured - T_observed
    # coeff. of thermal expansion, α=  1.2 × 10−6 K^(−1),[Common grades of Invar, measured between 20 °C and 100 °C]
    # L = L_initial x ( 1 + α ⋅ ΔT)

    # store magnet pair index and offset distance accordingly, to be derived
    # magnetPair_offset = [(14,-30),(4,-30),(12,-30),(9,-30)] # +ve value makes radial outward movement, and -ve value for radial inward movement

    csv_input = pd.read_csv(plate_file,skipinitialspace=True)
    # print(pd.read_csv(plate_file, index_col=0, nrows=0).columns.tolist())
    csv_input['magnetPair_offset'] = '0.000'
    csv_input.to_csv(plate_file, index=False, sep=' ', quoting=csv.QUOTE_NONE,escapechar=' ')

    return plate_file, magnetPair_offset

## Created as a standalone function for the robot, so should not be required to implement in this pipeline
# radial Position offset being adjusted in the extract_data.py file before all_magnets are being produced
def radialPositionOffset(list_of_probes,magnetPair_offset):

    # iterating through each magnet in the offset list and the probes list
    for item in magnetPair_offset:
        for each_probe in list_of_probes:

            # looking for match of items in offset and probes list
            if item[0] == each_probe.index:

                ### TEST PRINT
                print(each_probe.circular_magnet_center)

                # calculating the angle of the magnet with respect to x-axis
                theta = atan(each_probe.circular_magnet_center[1] / each_probe.circular_magnet_center[0])

                # calculating sine and cosine angle adjustment to offset distance for the radial movement
                # x values in positive range and -ve range move in opposite radial directions, so this step ensure same direction movement
                if each_probe.circular_magnet_center[0] >= 0:
                    each_probe.circular_magnet_center = (each_probe.circular_magnet_center[0] + (cos(theta) * item[1]), \
                                                         each_probe.circular_magnet_center[1] + (sin(theta) * item[1]))
                else:
                    each_probe.circular_magnet_center = (each_probe.circular_magnet_center[0] - (cos(theta) * item[1]), \
                                                         each_probe.circular_magnet_center[1] - (sin(theta) * item[1]))

                ### TEST PRINT
                print('RADIAL OFFSET CHANGEEE')
                print(each_probe.circular_magnet_center)

    return list_of_probes

def magnetOffsets_asColumns_toFile():

    #


    return