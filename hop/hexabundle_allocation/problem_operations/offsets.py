from ..general_operations.trigonometry import rotational_matrix, convert_degrees_to_radians, convert_radians_to_degrees
from ..hector.constants import robot_center_x, robot_center_y, circular_rectangle_magnet_center_distance
from ..hector.probe import probe
from math import atan,sin,cos,sqrt,degrees,radians,pi
import numpy as np
import pandas as pd
import csv
import string
import sys

"""
TODO:

- One function here uses probe.calculate_rectangular_magnet_center_coordinates, the other uses the calculate_rectangular_magnet_center_coordinates function here. 
- Change hexaPositionOffset to use the .csv Offset file rather than the excel version
- Get rid of unused imports
"""

def hexaPositionOffset(all_magnets, offsetFile):

    """
    Apply the P and Q offsets to the circular magnets and then move the rectangular magnets accordingly. 
    """

    circular_magnets = [magnet for magnet in all_magnets if magnet.__class__.__name__ == "circular_magnet"]
    rectangular_magnets = [magnet for magnet in all_magnets if magnet.__class__.__name__ == "rectangular_magnet"]

    # Get the P and Q offsets from the file
    df = pd.read_excel(offsetFile, usecols=['Name', 'P', 'Q'], engine='openpyxl')

    # drop the rows with NA values in Q column
    df.dropna(subset=['Q'], inplace=True)

    # omit the information row via string length check
    df_adjusted = df[df['Name'].str.len() <= 5]
    # Set the index to be the hexabundle name
    P_Q_offsets_df = df_adjusted.set_index("Name")

    # Add in the orientation of the rectangular magnets to the dataframe
    ang = {r.hexabundle : r.orientation for r in rectangular_magnets}
    P_Q_offsets_df['rectangular_magnet_orientation'] = P_Q_offsets_df.index.map(ang)

    # calculate and make the changes to x and y positions of circular magnets as per offset values
    for magnet in circular_magnets:

        # adjusting the angle to ensure movement is about the rectangular magnet's centre axis
        angle_adjusted = (90 + P_Q_offsets_df.loc[magnet.hexabundle, 'rectangular_magnet_orientation']) % 360

        P_offset =  P_Q_offsets_df.loc[magnet.hexabundle, 'P'] / 1000.0
        Q_offset =  P_Q_offsets_df.loc[magnet.hexabundle, 'Q'] / 1000.0

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

        # recalculating the circular magnet orientation with offset adjusted centre coordinates
        magnet.circular_magnet_center = magnet.center
        magnet.orientation = probe.calculate_circular_magnet_orientation(magnet)

        # Added by Sam on 15/01/2022: Update the Circular magnet AzAngs value
        updated_AzAngs = np.arctan2(magnet.center[1], magnet.center[0]) + np.pi
        magnet.azAngs = updated_AzAngs

        # updating plotting view for Robot file centre coordinates
        magnet.view_y = magnet.center[0]
        magnet.view_x = - magnet.center[1]


    # Now recalculate the x/y positions of the rectangular magnets using the adjusted circular magnet coordinates
    for c, r in zip(circular_magnets, rectangular_magnets):

        assert c.hexabundle == r.hexabundle, "Our magnets are out of order!"
        r.circular_magnet_center = c.center
        r.circular_rectangle_magnet_center_distance = circular_rectangle_magnet_center_distance

        r.center = probe.calculate_rectangular_magnet_center_coordinates(r)
        r.view_x = -r.center[1]
        r.view_y = r.center[0]

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
