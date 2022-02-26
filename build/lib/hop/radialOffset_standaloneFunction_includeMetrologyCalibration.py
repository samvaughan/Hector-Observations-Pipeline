import pandas as pd
pd.options.mode.chained_assignment = None  # disabled warning about writes making it back to the original frame
from math import atan,sin,cos,sqrt,degrees,radians,pi
import numpy as np
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt
import warnings
from pathlib import Path

def radialOffset_standaloneFunction(filename, offset=0.0, T_observed=None, T_configured=None, plate_radius=226.0, alpha=1.2e-6, robot_centre=[324.470,297.834], robot_shifts_file='./robot_shifts_abs.csv', apply_telecentricity_correction=True, apply_metrology_calibration=True, apply_roll_correction=True, verbose=True):
    
    """
    Apply a number of corrections to the magnet positions in a Hector Robot file. The corrections are:

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
    
    # Read in the file and save the header
    # getting count of lines to skip at top of file, which contain other information
    with open(filename) as file:
        lines = file.readlines()
    header = lines[:6]
    assert lines[6].startswith('#'), "The 7th line of this file doesn't start with #- have the headers changed? If so, update these lines of code!"

    df_read = pd.read_csv(filename, sep=',', skiprows=6)
    # renaming the '#Magnet' column name to remove the '#'
    df = df_read.rename(columns={'#Magnet': 'Magnet'})


    # Updating the header of the robot file with the updated parameters (if any)
    header[2] = f"Radial_Offset_Adjustment, {offset} #Radial offset is in millimetre(mm) with +ve values actioning radial outward movement and -ve values actioning radial inward movement of the magnets.\n"
    header[5] = f"Radial_Scale_factor, {round(offset/plate_radius,5) + 1} #Radial scale factor is thermal coefficient of invar times temperature difference applied radially relative to the plate centre.\n"

    if T_observed is not None and T_configured is not None:
        header[3] = f"RobotTemp, {T_configured:.5f} #temperature (degrees C) the distortion code assumed the field would be observed at\n"
        header[4] = f"ObsTemp, {T_observed:.5f} #actual temperature (degrees C) it is going to be observed at\n"


    # Apply the radial offsets
    df = apply_offsets_to_magnets(df, offset, robot_centre, apply_telecentricity_correction=apply_telecentricity_correction, verbose=verbose)

    # Perform metrology-based calibration
    if apply_metrology_calibration:
        orig_coords = np.array([df['Center_x'], df['Center_y']]).T
        theta_d = df['rot_platePlacing']
        metr_calibrated_coords, calibd_theta_d = perform_metrology_calibration(orig_coords, theta_d,
                                                           robot_centre=robot_centre, robot_shifts_file=robot_shifts_file, verbose=verbose)
        df['Center_x'] = metr_calibrated_coords[:, 0]
        df['Center_y'] = metr_calibrated_coords[:, 1]
        df['rot_platePlacing'] = calibd_theta_d

    # Apply the roll correction to the x values of the magnets _after_ the metrology correction
    if apply_roll_correction:
        if verbose:
            print("\nApplying the Roll Correction")
        for index, row in df.iterrows():
            roll_correction_offset = roll_correction(row['Center_x'], row['Center_y'], row['Magnet'])
            df.at[index, 'Center_x'] = row['Center_x'] + roll_correction_offset

    # Write the output file with the expected file ending
    fname = Path(filename)
    outputFile = fname.parent / (fname.stem + '_radialOffsetAdjusted.csv')

    # Replace the # at the start of the line with column names
    df = df.rename(columns={'Magnet': '#Magnet'})

    # write the description from input robot file at top of final output robot file
    with open(outputFile, 'w') as f:
        for line in header:
            f.write(line)
    df.to_csv(outputFile, index=False, sep=',', mode='a')

    if verbose:
        print(f'Output file: {outputFile}')
    return df


def apply_offsets_to_magnets(df, offset, robot_centre, apply_telecentricity_correction=True, verbose=True):
    """
    Apply the radial and telecentricity offsets to the circular magnets, then calculate the corresponding positions of the rectangular magnets.
    """

    if verbose:
        print(f"Applying an overall radial offset of {offset}mm to all circular magnets")

    # Loop through the hexabundles
    all_hexabundles = df.loc[df['Magnet'] == 'circular_magnet', 'Hexabundle'].values
    for hexabundle in all_hexabundles:

        circular_magnet = df.loc[(df['Hexabundle'] == hexabundle) & (df['Magnet'] == 'circular_magnet')]
        rectangular_magnet = df.loc[(df['Hexabundle'] == hexabundle) & (df['Magnet'] == 'rectangular_magnet')]
        
        new_x, new_y = calculate_radial_offset(circular_magnet, offset, robot_centre, apply_telecentricity_correction=apply_telecentricity_correction, verbose=verbose)

        if verbose:
            print(f"Hexabundle {hexabundle}: \n\tCircular magnet:")
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
    Find the required offset in the posotion of the circular magnets based on their telecentricity annulus
    """
    if verbose:
        print("Applying the telecentricity offset...")
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
    Given an x and y calculate the total radial offsets to apply based on a temperature change and a fixed telecentricity correction 
    """
    x = (circular_magnet['Center_x'] - robot_centre[0]).values[0]
    y = (circular_magnet['Center_y'] - robot_centre[1]).values[0]

    # If x and y are already at the plate centre, return them as they are since we don't know the direction to move them in (their radial offset direction isn't known)
    if (x == 0.0) & (y == 0.0):
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


def perform_metrology_calibration(input_coords, input_theta_d, robot_centre, robot_shifts_file, verbose=True):

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
    popt, pcov = curve_fit(fitting_fun, metr_in_coords.reshape((8)), metr_wanted_coords.reshape((8)), p0=p0)

    # # For DEBUG: Measure residuals for the marker measurements
    # corrected_data = apply_cal(metr_in_coords, popt)
    # all_resids = corrected_data - metr_wanted_coords

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
    theta_d = popt[2]
    output_theta_d = (input_theta_d - theta_d) % 360 ### SIGN ISSUE: If rotation direction is incorrect, change this + to -

    if verbose:
        print(f'\nApplied metrology-based calibration, using the following fitted coefficients: {popt}')

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

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("filename", type=str, help='The object frame number you want to display')
    parser.add_argument("robot_shifts_file", type=str, help='Filename of the robot metrology measurements')
    parser.add_argument("--offset", default=0, type=float, help='Radial offset to apply in mm. +ve is outward, -ve is inwards')
    parser.add_argument("--T_observed", default=None, type=float, help='Temperature at which the plate will be observed')
    parser.add_argument("--T_configured", default=None, type=float, help='Temperature at which the plate is configured')
    parser.add_argument("--silent", action="store_true", help="Turn off output from the code")

    args = parser.parse_args()
    filename = args.filename
    robot_shifts_file = args.robot_shifts_file

    #Optional Arguments
    offset = args.offset
    T_observed = args.T_observed
    T_configured = args.T_configured
    verbose = not args.silent


    df = radialOffset_standaloneFunction(filename, robot_shifts_file=robot_shifts_file, offset=offset, T_observed=T_observed, T_configured=T_configured, verbose=verbose)