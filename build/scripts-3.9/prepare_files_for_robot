#!python

import pandas as pd
pd.options.mode.chained_assignment = None  # disabled warning about writes making it back to the original frame
from math import atan,sin,cos,sqrt,degrees,radians,pi
import numpy as np
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt
import warnings
from pathlib import Path

from hop.scripts import robot_corrections as corrections
from hop.scripts import robot_file_input_output as file_functions

def correct_parking_positions_file(filename, robot_shifts_file, T_observed=None, T_configured=None, plate_radius=226.0, alpha=1.2e-6, robot_centre=[324.470,297.834], apply_metrology_calibration=True, apply_roll_correction=True, verbose=True):

    """
    Apply the necessary corrections to the x and y *parking positions* of the Hector magnets. We then write this file out to a new one with the exact same format
    """

    if verbose:
        print(f'\nAdjusting the Parking Positions:')

    df = file_functions.read_parking_positions_file(filename)

    df = apply_corrections(df, offset=0.0, T_observed=T_observed, T_configured=T_configured, plate_radius=plate_radius, alpha=alpha, robot_centre=robot_centre, robot_shifts_file=robot_shifts_file, apply_telecentricity_correction=False, apply_metrology_calibration=apply_metrology_calibration, apply_roll_correction=apply_roll_correction, apply_rotation_correction=False, verbose=verbose)

    # Now write the parking positions file out in the correct format
    # We want to keep the same long string of numbers from the filename of the Robot Shifts file
    fname = Path(filename)

    metrology_date = Path(robot_shifts_file).expanduser().stem.split('_')[-1]
    output_file = fname.parent / (fname.stem + f'_{metrology_date}.csv')
    df = file_functions.write_standard_parking_positions_file(df, output_file)

    if verbose:
        print(f'\tOutput file: {output_file}')

    return df

def correct_robot_file(filename, offset=0.0, T_observed=None, T_configured=None, plate_radius=226.0, alpha=1.2e-6, robot_centre=[324.470,297.834], robot_shifts_file='./robot_shifts_abs.csv', apply_telecentricity_correction=True, apply_metrology_calibration=True, apply_roll_correction=True, apply_rotation_correction=True, verbose=True):

    """
    Apply the necessary corrections to the x and y positions of magnets in a *Hector Robot file*. We also read in and update its header
    """

    if verbose:
        print(f'\n\nAdjusting the Robot File')
    # Read in the file and save the header
    # getting count of lines to skip at top of file, which contain other information
    with open(filename) as file:
        lines = file.readlines()
    header = lines[:6]
    assert lines[6].startswith('#'), "The 7th line of this file doesn't start with #- have the headers changed? If so, update these lines of code!"

    # Updating the header of the robot file with the updated parameters (if any)
    header[2] = f"Radial_Offset_Adjustment, {offset} #Radial offset is in millimetre(mm) with +ve values actioning radial outward movement and -ve values actioning radial inward movement of the magnets.\n"
    header[5] = f"Radial_Scale_factor, {round(offset/plate_radius,5) + 1} #Radial scale factor is thermal coefficient of invar times temperature difference applied radially relative to the plate centre.\n"

    if T_observed is not None and T_configured is not None:
        header[3] = f"RobotTemp, {T_configured:.5f} #temperature (degrees C) the distortion code assumed the field would be observed at\n"
        header[4] = f"ObsTemp, {T_observed:.5f} #actual temperature (degrees C) it is going to be observed at\n"

    # Read in the file again as a data frame
    df = file_functions.read_standard_robot_file(filename)

    # Now apply the corrections
    df = apply_corrections(df, offset=offset, T_observed=T_observed, T_configured=T_configured, plate_radius=plate_radius, alpha=alpha, robot_centre=robot_centre, robot_shifts_file=robot_shifts_file, apply_telecentricity_correction=apply_telecentricity_correction, apply_metrology_calibration=apply_metrology_calibration, apply_roll_correction=apply_roll_correction, apply_rotation_correction=apply_rotation_correction, verbose=verbose)

    df, output_file = file_functions.write_standard_robot_file(df, filename, header)

    if verbose:
        print(f'\tOutput file: {output_file}')

    return df

def apply_corrections(df, robot_shifts_file, offset=0.0, T_observed=None, T_configured=None, plate_radius=226.0, alpha=1.2e-6, robot_centre=[324.470,297.834], apply_telecentricity_correction=True, apply_metrology_calibration=True, apply_roll_correction=True, apply_rotation_correction=True, verbose=True):
    
    """
    Apply a number of corrections to the magnet positions. The corrections are:

        * A radial shift inwards or outwards applied to all circular magnets to account for a difference in temperature between the plate at the time it was configured and the plate at the time it will be observed. Note that this offset can be entered in millimetres, or otherwise two temnperatures and a coefficient of thermal expansion can be given instead.
        * A correction on the position of each circular magnet based on its radial position in one of four telecentricity annuli. The circular magnet is moved inwards to account for the fact the light is not truly plane-parallel across the plate. 
        * A correction on the position of all magnets based on accurate calibration of the position and rotation of the plate using the pre-defined metrology markers. This correction can be decomposed into a shift of the plate centre, a shear term and a rotation. The derived correction is fit to the measurements in the robot_shifts_file.
        * A correction on North/South position of all magnets based on the roll of the robot arm as it places each object. The extension of the robot arm to the far corner of the plate induces a small torque which affects its ability to place a magnet accurately. This torque has been measured and fitted with quadratic function which depends on the y value (in robot coordinates) of the magnet and is applied to its x value.
    
    For corrections which are applied to only the circular magnets, we then calculate the position of the rectangular magnets again to ensure that the distance between them is always 27.2mm. 

    Inputs:
        filename (str):
            A Hector Robot file containing information about 27 circular and 27 rectnagular magnets to be placed on the plate.
        offset (float, optional):
            A radial offset (in mm) to be applied to all circular magnets. Negative values are towards the plate centre, positive values are away from the plate centre. The default is 0.0
        T_observed (float, optional): 
            The temperature (in Celsius or Kelvin) at which the plate will be observed at. Default is None. 
        T_configured (float, optional): 
            The temperature (in Celsius or Kelvin) at which the plate was configured at. Default is None.
        plate_radius (float, optional):
            The radius of the plate in mm, for claculating the thermal expansion. Default is 226.0
        alpha (float, optional):
            The coefficient of thermal expansion of invar. Default is 1.2e-6
        robot_centre (list, optional): 
            A two element list comntaing the x and y coordinates of the plate centre in the coordinates of the robot. You really shouldn't change this unless you have a very good reason! Default is [324.470,297.834]
        robot_shifts_file, (str, optional):
            The location of the metrology measurements file. Default is 'robot_shifts_abs.csv'
        apply_telecentricity_correction (bool, optional):
            Whether or not to apply the telecentricity correction to the magnets. In all reasonable circumstances this should be True! Default is True. 
        apply_metrology_calibration (bool, True):
            Whether or not to apply the metrology-based calibration correction. Should always be True. Default is True
        apply_roll_correction (bool, True):
            Whether or not to apply the robot roll correction. Should always be True. Default is True.
        apply_rotation_correction (bool, True):
            Whether or not to apply the correction for the different alignments of the robot cylinder and pickup arm. Should always be True, default is True. 
        verbose (bool, True):
            Print information about the corrections to the screen. Default is True. 
    Outputs:
        A file with offset value adjusted saved in same directory as input file, with extended name of '_radialOffsetAdjusted.csv' at the end. 
    """

    # Some basic argument checking
    if T_observed is not None and T_configured is None:
        raise ValueError("T_configured cannot be None if T_obsered is not None!")

    if T_observed is None and T_configured is not None:
        raise ValueError("T_observed cannot be None if T_configured is not None!")

    if T_observed is not None and T_configured is not None:
        if offset != 0.0:
            raise ValueError("If T_configured and T_observed are given then offset can't be given as well!")
        delta_T = T_configured - T_observed
        offset = plate_radius * ( delta_T * alpha )  

    
    # Apply the radial offsets
    if (offset !=0) or (apply_telecentricity_correction is True):
        df = corrections.apply_offsets_to_magnets(df, offset, robot_centre, apply_telecentricity_correction=apply_telecentricity_correction, verbose=verbose)

    # Perform metrology-based calibration
    if apply_metrology_calibration:
        orig_coords = np.array([df['Center_x'], df['Center_y']]).T
        theta_d = df['rot_platePlacing']
        metr_calibrated_coords, calibd_theta_d = corrections.perform_metrology_calibration(orig_coords, theta_d,
                                                           robot_centre=robot_centre, robot_shifts_file=robot_shifts_file, verbose=verbose)
        df['Center_x'] = metr_calibrated_coords[:, 0]
        df['Center_y'] = metr_calibrated_coords[:, 1]
        df['rot_platePlacing'] = calibd_theta_d

    # Apply the roll correction to the x values of the magnets _after_ the metrology correction
    if apply_roll_correction:
        if verbose:
            print("\tApplying the Roll Correction")
        for index, row in df.iterrows():
            roll_correction_offset = corrections.roll_correction(row['Center_x'], row['Center_y'], row['Magnet'])
            df.at[index, 'Center_x'] = row['Center_x'] + roll_correction_offset

    #Apply the correction for the unaligned centres of rotation for the pickup arm vs the cylinder
    if apply_rotation_correction:
        if verbose:
            print("\tApplying the offset-rotation-axis correction")
        for index, row in df.iterrows():
            new_x, new_y = corrections.pick_up_arm_rotation_correction(row['Center_x'], row['Center_y'], rot_platePlacing=row['rot_platePlacing'])
            df.at[index, 'Center_x'] = new_x
            df.at[index, 'Center_y'] = new_y

    return df


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("robot_filename", type=str, help='The full path of the robot file to apply corrections to')
    #parser.add_argument("parking_positions_filename", type=str, help='The full path of the parking positions file to apply corrections to')
    parser.add_argument("robot_shifts_file", type=str, help='Filename of the robot metrology measurements')
    parser.add_argument("--offset", default=0, type=float, help='Radial offset to apply in mm. +ve is outward, -ve is inwards')
    parser.add_argument("--T_observed", default=None, type=float, help='Temperature at which the plate will be observed')
    parser.add_argument("--T_configured", default=None, type=float, help='Temperature at which the plate is configured')
    parser.add_argument("--silent", action="store_true", help="Turn off output from the code")

    args = parser.parse_args()
    robot_filename = args.robot_filename
    #parking_positions_filename = args.parking_positions_filename
    robot_shifts_file = args.robot_shifts_file

    #Optional Arguments
    offset = args.offset
    T_observed = args.T_observed
    T_configured = args.T_configured
    verbose = not args.silent

    parking_positions_filename = r"Z:\Robot_tile_files\ParkingPosns_final.csv"

    robot_df = correct_robot_file(robot_filename, robot_shifts_file=robot_shifts_file, offset=offset, T_observed=T_observed, T_configured=T_configured, verbose=verbose)
    parking_positions_df = correct_parking_positions_file(parking_positions_filename, robot_shifts_file=robot_shifts_file, verbose=verbose)