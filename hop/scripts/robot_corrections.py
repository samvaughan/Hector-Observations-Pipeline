import pandas as pd
pd.options.mode.chained_assignment = None  # disabled warning about writes making it back to the original frame
import numpy as np
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt
import warnings
from pathlib import Path

"""
Various functions which correct the magnet x and y positions on the Hector Plate
"""


def pick_up_arm_rotation_correction(robot_centre_x, robot_centre_y, rot_platePlacing):
    """
    Correct for the errors in magnet position introduced by the different centres of rotation of the robot cylinder and the pickup arm. 

    First written by Stefania Barsanti, May 2022. Edited by Sam Vaughan, May/June 2022
    """
    
    d = 24*0.001 # distance from center of pick up arm to center of rotation in [mm]
    ang0 = 20 # when the robot is at 0deg, it is actually 20 deg from the +x axis. The rotation direction is clockwise
    theta = np.radians(rot_platePlacing - ang0)

    robot_centre_x_new = 1 * (robot_centre_x - d * np.cos(theta))
    robot_centre_y_new = 1 * (robot_centre_y + d * np.sin(theta))

    return robot_centre_x_new, robot_centre_y_new

def apply_offsets_to_magnets(df, offset, robot_centre, apply_telecentricity_correction=True, verbose=True):
    """
    Apply the radial and telecentricity offsets to the circular magnets, then calculate the corresponding positions of the rectangular magnets.
    """

    if verbose:
        print(f"\tApplying an overall radial offset of {offset}mm to all circular magnets")

    # Loop through the hexabundles
    all_hexabundles = df.loc[df['Magnet'] == 'circular_magnet', 'Hexabundle'].values
    for hexabundle in all_hexabundles:

        circular_magnet = df.loc[(df['Hexabundle'] == hexabundle) & (df['Magnet'] == 'circular_magnet')]
        rectangular_magnet = df.loc[(df['Hexabundle'] == hexabundle) & (df['Magnet'] == 'rectangular_magnet')]
        
        new_x, new_y = calculate_radial_offset(circular_magnet, offset, robot_centre, apply_telecentricity_correction=apply_telecentricity_correction, verbose=verbose)

        if verbose:
            print(f"\tHexabundle {hexabundle}: \n\tCircular magnet:")
            print(f"\t\toriginal centre is ({circular_magnet['Center_x'].values[0]:.3f}, {circular_magnet['Center_y'].values[0]:.3f})")
            print(f"\t\tnew centre is ({new_x + robot_centre[0]:.3f}, {new_y + robot_centre[1]:.3f})")

        # Update the new centres of the circular magnets
        df.at[circular_magnet.index[0], 'Center_x'] = new_x + robot_centre[0]
        df.at[circular_magnet.index[0], 'Center_y'] = new_y + robot_centre[1]

        # Now find the centres of the rectangular magnets
        angle_for_rectangular_magnet= np.radians(270 - rectangular_magnet['rot_holdingPosition'] - rectangular_magnet['rot_platePlacing']).values[0]
        [x_rect,y_rect] = calculate_rectangular_magnet_centre_coordinates(new_x, new_y, angle_for_rectangular_magnet)

        if verbose:
            print("\tRectangular magnet:")
            print(f"\t\toriginal centre is ({rectangular_magnet['Center_x'].values[0]:.3f}, {rectangular_magnet['Center_y'].values[0]:.3f})")
            print(f"\t\tnew centre is ({x_rect + robot_centre[0]:.3f}, {y_rect + robot_centre[1]:.3f})")

        # Update the new centres of the rectangular magnets
        df.at[rectangular_magnet.index[0], 'Center_x'] = x_rect + robot_centre[0]
        df.at[rectangular_magnet.index[0], 'Center_y'] = y_rect + robot_centre[1]

    return df


def calculate_telecentricity_correction(magnet, robot_centre, verbose=True):
    """
    Find the required offset in the position of the circular magnets based on their telecentricity annulus
    """
    if verbose:
        print("\tApplying the telecentricity offset")
    robot_centre_x, robot_centre_y = robot_centre

    centre_x = magnet.Center_x.values
    centre_y = magnet.Center_y.values
    telecentricity = magnet.Label.values

    if telecentricity == 'Blu':
        radial_d = 20.7*0.001 #mm      
    elif telecentricity == 'Gre':
        radial_d = 42.0*0.001
    elif telecentricity == 'Yel':
        radial_d = 61.2*0.001  
    elif telecentricity == 'Mag':
        radial_d = 79.9*0.001

    if ((robot_centre_x - centre_x) == 0.0) & ((robot_centre_y - centre_y) == 0.0):
        return 0.0, 0.0

    norm = np.sqrt((robot_centre_x - centre_x)**2 + (robot_centre_y - centre_y)**2)
    telentricity_offset_x = radial_d * (robot_centre_x - centre_x) / norm
    telentricity_offset_y = radial_d * (robot_centre_y - centre_y) / norm

    return telentricity_offset_x[0], telentricity_offset_y[0]

def calculate_radial_offset(circular_magnet, radial_offset, robot_centre, apply_telecentricity_correction=True, verbose=True):

    """
    Given an x and y coordinate, calculate the total radial offsets to apply based on a temperature change and a fixed telecentricity correction 
    """
    x = (circular_magnet['Center_x'] - robot_centre[0]).values[0]
    y = (circular_magnet['Center_y'] - robot_centre[1]).values[0]

    # If x and y are already at the plate centre, return them as they are since we don't know the direction to move them in (their radial offset direction isn't known)
    # Change this to be less than 1e-100 since values like 1e-162 were failing this check
    if (np.abs(x) <= 1e-150) & (np.abs(y) <=1e-150):
        return 0, 0

    if apply_telecentricity_correction:
        telecentricity_x_corr, telecentricity_y_corr = calculate_telecentricity_correction(circular_magnet, robot_centre, verbose=verbose)
    else:
        telecentricity_x_corr = 0 
        telecentricity_y_corr = 0

    # Make a unit vector which points towards the origin
    # The origin here is (0, 0) since we've already removed the plate centre offset
    unit_vector = 1./(np.sqrt(x**2 + y**2)) * np.array([x, y]) - np.array([0, 0]) 
    offset_vector = unit_vector * radial_offset

    delta_x = offset_vector[0]
    delta_y = offset_vector[1]

    new_x = x + delta_x + telecentricity_x_corr
    new_y = y + delta_y + telecentricity_y_corr

    return new_x, new_y


def calculate_rectangular_magnet_centre_coordinates(x, y, rm_angle):
    """
    The rectangular magnets always have to be 27.2 mm from their circular magnets, and at the appropriate angle
    """
    circular_rectangle_magnet_centre_distance = 27.2

    rectangular_magnet_centre = [x + circular_rectangle_magnet_centre_distance * np.cos(rm_angle), y +circular_rectangle_magnet_centre_distance * np.sin(rm_angle)]
    return rectangular_magnet_centre


def perform_metrology_calibration(input_coords, input_theta_d, robot_centre, robot_shifts_file, verbose=True, permagnet_theta_corr=True):
    """
    Apply a correction based on the measured metrology of the robot. Written by Barnaby Norris. 
    """

    cent_wrt_origin = np.array([-270.5, 179.0])  # Manufactured offset between metrology origin and plate centre

    # Calculate correction coefficients from metrology
    robot_shifts_abs = np.loadtxt(robot_shifts_file, delimiter=',')
    [x0, y0] = robot_shifts_abs[0, 0:2]
    metr_in_coords = robot_shifts_abs[:, 0:2]
    metr_wanted_coords = np.array([x0, y0]) + robot_shifts_abs[:, 2:4]

    # Solve for transformation coefficients.
    # popt contains the optimised coefficients
    p0 = np.array([0, 0, 0, 1, 1, 0, 0])
    warnings.filterwarnings("ignore", message="Covariance")
    popt, pcov = curve_fit(fitting_fun, metr_wanted_coords.reshape((8)), metr_in_coords.reshape((8)), p0=p0)

    # # For DEBUG: Measure residuals for the marker measurements
    corrected_data = apply_cal(metr_wanted_coords, popt)
    all_resids = corrected_data - metr_in_coords

    # For backwards compatability reasons, the input coordinates are supplied based on best-known plate centre
    # (given by the 'robot_centre' parameter). Here, we instead want to use the actual measured value of
    # the metrology origin, along with true value (specified to manufacturer) of the offset between the
    # metrology origin and plate centre.
    true_centre = np.array([x0, y0]) + cent_wrt_origin
    input_coords_centred = input_coords - robot_centre + true_centre

    # Apply metrology-based correction
    metr_calibrated_coords = apply_cal(input_coords_centred, popt)

    # Now correct theta, the angle of robot rotation stage. Needs to be rotated by the same amount as global coordinate
    # rotation.
    if permagnet_theta_corr:
        npts = input_coords_centred.shape[0]
        all_theta_ds = np.zeros(npts)
        for k in range(npts):
            dx1 = x0 - input_coords_centred[k, 0]
            dy1 = y0 - input_coords_centred[k, 1]
            phi_old = np.arctan(dy1 / dx1)
            dx2 = x0 - metr_calibrated_coords[k, 0]
            dy2 = y0 - metr_calibrated_coords[k, 1]
            phi_new = np.arctan(dy2 / dx2)
            cur_theta_d = (phi_new - phi_old) / np.pi * 180
            all_theta_ds[k] = cur_theta_d
        theta_d = all_theta_ds
    else:
        theta_d = popt[2]
    output_theta_d = (input_theta_d - theta_d) % 360 ### SIGN ISSUE: If rotation direction is incorrect, change this + to -

    if verbose:
        print(f'\tApplied metrology-based calibration, using the following fitted coefficients: {popt}')

    return metr_calibrated_coords, output_theta_d


def apply_cal(inputs, p):
    # Applies the offset and transformation matrix
    thr = p[2]/180*np.pi
    Rmat = np.array([[np.cos(thr), -np.sin(thr)],
                     [np.sin(thr), np.cos(thr)]])
    p[6]=0 # With 2 shear terms matrix is underconstrained
    shmat = np.array([[1, p[5]],
                     [p[6], 1]])
    sclmat = np.array([[p[3], 0], [0, p[4]]])

    out_coords = np.zeros_like(inputs)
    for k in range(inputs.shape[0]):
        invec = inputs[k,:]
        outvec = invec + np.array([p[0], p[1]])
        outvec = Rmat @ outvec
        outvec = shmat @ outvec
        outvec = sclmat @ outvec
        out_coords[k,:] = outvec
    return out_coords


def fitting_fun(Xs, offsX, offsY, theta, sclX, sclY, sh1, sh2):
    params = np.array([offsX, offsY, theta, sclX, sclY, sh1, sh2])
    in_coords = Xs.reshape((4,2))
    out_coords = apply_cal(in_coords, params)
    out = out_coords.reshape((8))
    return out


def roll_correction(centre_x, centre_y, magnet):
    """
    Calculate the offset which should be applied to the robot x coordinate to account for the roll of the robot arm. 
    """
    if magnet == 'circular_magnet':
        a = 3.313E-07
        b = -4.507E-05
        c = 1.819E-02
    elif magnet == 'rectangular_magnet':  
        a = 4.133E-07
        b = -4.151E-05
        c = 1.767E-02

    roll_offset_centre_x = a * centre_y**2 + b * centre_y + c  

    return roll_offset_centre_x
