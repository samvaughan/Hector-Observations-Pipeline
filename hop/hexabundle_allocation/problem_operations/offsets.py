from ..general_operations.trigonometry import rotational_matrix, convert_degrees_to_radians, convert_radians_to_degrees
from ..hector.constants import robot_center_x, robot_center_y, circular_rectangle_magnet_center_distance
from ..hector.probe import probe
from math import atan,sin,cos,sqrt,degrees,radians,pi
import numpy as np
import pandas as pd
import csv
import string
import sys
import re


# Adjusting offset to move circular magnet closer to OR far from rectangular magnet
def hexaPositionOffset(all_magnets, offsetFile):

    # read offset excel file as a dataframe
    df = pd.read_excel(offsetFile, usecols=['Name', 'P', 'Q'], engine='openpyxl')

    # drop the rows with NA values in Q column
    df.dropna(subset=['Q'], inplace=True)

    # omit the information row via string length check
    df_adjusted = df[df['Name'].str.len() <= 5]

    # reset the index of the adjusted dataframe with changes
    df_adjusted = df_adjusted.reset_index(drop=True)
    df_adjusted.index = df_adjusted.index + 1
    df_final = df_adjusted

    #print(df_final)

    # create dictionaries to store the offset values of P and Q, and ang to store rectangular magnet orientation
    ang = {}
    offset_distance_P = {}
    offset_distance_Q = {}

    circular_magnets = [magnet for magnet in all_magnets if magnet.__class__.__name__ == "circular_magnet"]
    rectangular_magnets = [magnet for magnet in all_magnets if magnet.__class__.__name__ == "rectangular_magnet"]

    # Add a column for the angles
    rectangular_magnet_orientation = {r.index: r.orientation for r in rectangular_magnets}
    df_final["rectangular_magnet_orientation"] = df_final.index.map(rectangular_magnet_orientation)

    #import ipdb; ipdb.set_trace()
    # loop through all_magnets list and offset dataframe to store offset values and rectangular magnet
    # orientation values in dictionary with key as index
    for i in all_magnets:

        for j in range(1, len(df_final) + 1):

            if i.__class__.__name__ == 'rectangular_magnet' and i.hexabundle == df_final['Name'][j]:
                # getting the orientation of rectangular magnet with respect to North(up)
                ang[i.index] = i.orientation

                # storing the offset values in dictionary from the offset dataframe
                offset_distance_P[i.index] = df_final['P'][j] / 1000
                offset_distance_Q[i.index] = df_final['Q'][j] / 1000

                print('Index: ' + str(i.index) + ' , Hexabundle: ' + str(i.hexabundle) + ' , offset_P: ' + str(
                    df_final['P'][j] / 1000))

                print('orientation_rectangular = '+str(ang))
                azAngs = convert_radians_to_degrees(i.azAngs)
                print('azAngs_rectangular = '+str(azAngs))
                ang_azAngs = convert_radians_to_degrees(i.rectangular_magnet_input_orientation)
                print('ang_azAngs_rectangular = ' + str(ang_azAngs))
                
                rectangular_centre = i.center
                print(rectangular_centre)

    print(ang)
    print(offset_distance_P)
    print(offset_distance_Q)


    #print('HEREEEEEEE')
    #print(range(len(all_magnets)))

    # calculate and make the changes to x and y positions of circular magnets as per offset values

    for magnet in circular_magnets:

        # print(all_magnets[i].index)
        # print('Circular Before'+str(all_magnets[i].center))
        # print('Offset P = '+str(offset_distance_P[all_magnets[i].index]))
        # print('Offset Q = '+str(offset_distance_Q[all_magnets[i].index]))

        # adjusting the angle to ensure movement is about the rectangular magnet's centre axis
        angle_adjusted = 450 + rectangular_magnet_orientation[magnet.index]#df_final.loc[magnet.index, 'rectangular_magnet_orientation']

        # check to adjust angle within 0 to 360 range
        if angle_adjusted > 360:
            angle_adjusted = angle_adjusted - 360

        P_offset = offset_distance_P[magnet.index]/1000.0
        Q_offset = offset_distance_P[magnet.index]/1000.0

        # rotation matrix required for the offset adjustments of the circular magnets
        rotation_matrix_circle = rotational_matrix(convert_degrees_to_radians(angle_adjusted))

        # subtracting offset distance moves circular magnet in opposite direction to rectangular magnet
        # (P-parallel movement to rectangular magnet)
        magnet.center = (magnet.center[0] - rotation_matrix_circle[0][0] * P_offset, magnet.center[1] - rotation_matrix_circle[0][1] * P_offset)

        # subtracting offset distance moves circular magnet downwards with respect to the rectangular magnet on its right side
        # (Q-perpendicular movement to rectangular magnet)
        magnet.center = (magnet.center[0] - rotation_matrix_circle[1][0] * Q_offset, magnet.center[1] - rotation_matrix_circle[1][1] * Q_offset)

        # storing offset values in all_magnets list for final output file
        magnet.offset_P = P_offset
        magnet.offset_Q = Q_offset

        # print('orientation_circular = ' + str(magnet.orientation))

        # recalculating the circular magnet orientation with offset adjusted centre coordinates
        magnet.circular_magnet_center = magnet.center
        magnet.orientation = probe.calculate_circular_magnet_orientation(magnet)

        # Added by Sam on 15/01/2022: Update the Circular magnet AzAngs value
        updated_AzAngs = np.arctan2(magnet.center[1], magnet.center[0]) + np.pi
        #delta_AzAngs = magnet.azAngs - updated_AzAngs
        #print(f"AzAngs was {magnet.azAngs}, AzAngs is now {updated_AzAngs}, delta(AzAngs) is {delta_AzAngs}")
        magnet.azAngs = updated_AzAngs

        #

        # print('Updated orientation_circular = ' + str(magnet.orientation))
        # print('Circular After:'+str(magnet.center))

        # updating plotting view for Robot file centre coordinates
        magnet.view_y = magnet.center[0]
        magnet.view_x = - magnet.center[1]

    
    #print('\n\n\n')

    # recalculate positions of x and y for rectangular magnets as per adjusted circular magnet coordinates
    for i in range(len(all_magnets)):

        if all_magnets[i].__class__.__name__ == 'rectangular_magnet':

            for j in range(len(all_magnets)):

                if all_magnets[j].__class__.__name__ == 'circular_magnet' and all_magnets[i].index == all_magnets[j].index:
                    # print(all_magnets[i].index)
                    # print('BEFORE:'+str(all_magnets[i].center))

                    # circular magnet centre is stored for rectangular magnets as well for calculation of center coordinates
                    all_magnets[i].circular_magnet_center = all_magnets[j].center
                    all_magnets[i].circular_rectangle_magnet_center_distance = circular_rectangle_magnet_center_distance

                    # recalculate rectangular magnet center coordinates as per updated circular magnet center
                    all_magnets[i].center = probe.calculate_rectangular_magnet_center_coordinates(all_magnets[i])

                    #Added by Sam 14/01/22
                    ## UPDATE THE VIEW ATTRIBUTES TOO- otherwise the rectangular magnets aren't using their updated centres!
                    all_magnets[i].view_x = - all_magnets[i].center[1]
                    all_magnets[i].view_y = all_magnets[i].center[0]



    # print("DONEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE")
    # angle = 90 + convert_radians_to_degrees(atan(abs(rectangular_centre[1]-circular_centre[1])/abs(rectangular_centre[0]-circular_centre[0])))
    # print(angle)

    return all_magnets



def calculate_telecentricity_correction(magnet, telecentricity_correction_dictionary, centre=[0, 0], verbose=False):
    """
    Find the required offset in the posotion of the circular magnets based on their telecentricity annulus
    """
    if verbose:
        print("Applying the telecentricity offset...")
    plate_centre_x, plate_centre_y = centre

    centre_x = magnet.center[0]
    centre_y = magnet.center[1]
    telecentricity = magnet.magnet_label

    radial_distance = telecentricity_correction_dictionary[telecentricity]

    if (centre_x == 0.0) & (centre_x == 0.0):
        raise ValueError("Can't have a magnet at precisely the plate centre!")

    norm = np.sqrt((plate_centre_x - centre_x)**2 + (plate_centre_y - centre_y)**2)
    telentricity_offset_x = radial_distance * (plate_centre_x - centre_x) / norm
    telentricity_offset_y = radial_distance * (plate_centre_y - centre_y) / norm

    return telentricity_offset_x, telentricity_offset_y


def magnetPair_radialPositionOffset_circularAnnulus(offset_circularAnnulus, all_magnets):

    """
    Adjust the posotion of a circular magnet based on its telecentrocity annulus. Note- apply this function either here or in the radial offset standalone function but not both!

    Inputs:
        offset_circularAnnulus (dict):
            A dictionary containing the amount each magnet moves depending on its telecentricity annulus. Must have keys 'Blu', 'Yel', 'Gre' and 'Mag'
        all_magnets (list):
            list of all magnet objects
    """

    circular_magnets = [magnet for magnet in all_magnets if magnet.__class__.__name__ == "circular_magnet"]
    rectangular_magnets = [magnet for magnet in all_magnets if magnet.__class__.__name__ == "rectangular_magnet"]

    for circular_magnet, rectangular_magnet in zip(circular_magnets, rectangular_magnets):

        # Apply a telecentricity correction
        telecentricity_correction_x, telecentricity_correction_y = calculate_telecentricity_correction(circular_magnet, offset_circularAnnulus)
        circular_magnet.center[0] += telecentricity_correction_x
        circular_magnet.center[1] += telecentricity_correction_y

        #Update the robot's view of the magnets
        circular_magnet.view_y = circular_magnet.center[0]
        circular_magnet.view_x = -circular_magnet.center[1]

        # Now update the rectangular magnets location absed on the circular magnets
        [x_rect, y_rect] = calculate_rectangular_magnet_center_coordinates(circular_magnet.center[0],circular_magnet.center[1], rectangular_magnet.angs)

        rectangular_magnet.center[0] = x_rect
        rectangular_magnet.center[1] = y_rect

        # updating the plotting view for Robot file coordinates
        rectangular_magnet.view_y = rectangular_magnet.center[0]
        rectangular_magnet.view_x = - rectangular_magnet.center[1]


    return all_magnets


# calculating the rectangular magnet orientation with respect to the circular magnet
def calculate_rectangular_magnet_orientation(angs):

    """
    Transform the angle of the rectangular magnet into the correct frame (by subtracting pi)
    """
    rectangular_magnet_absolute_orientation_degree = angs - np.pi

    return rectangular_magnet_absolute_orientation_degree

# calculating the rectangular magnet center coordinates from its orientation, and categorizing in four quadrants
def calculate_rectangular_magnet_center_coordinates(x,y,angs):

    rm_angle = calculate_rectangular_magnet_orientation(angs)

    circular_rectangle_magnet_centre_distance = 27.2
    rectangular_magnet_centre = [x + circular_rectangle_magnet_centre_distance * np.cos(rm_angle), y +circular_rectangle_magnet_centre_distance * np.sin(rm_angle)]

    return rectangular_magnet_centre
