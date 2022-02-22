import pandas as pd
pd.options.mode.chained_assignment = None  # disabled warning about writes making it back to the original frame
from math import atan,sin,cos,sqrt,degrees,radians,pi
import numpy as np
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt
import warnings

def radialOffset_standaloneFunction(filename, offset=-9999, T_observed=-10000, T_configured=-10000, robot_centre=None,
                                    plate_radius=None, alpha=None, apply_metr_calib=True, robot_shifts_file='./robot_shifts_abs.csv'):
    
    ''' FUNCTION TO ADJUST THE RADIAL OFFSET FOR ALL MAGNETS IN CASE OF THERMAL EXPANSION OF THE PLATE
    INPUTS: 1. 'filename'- Mandatory Input required is the filename with location
                e.g. filename = 'D:/Hector/Hector_project_files/Hector/magnet_position/G15/Allocation/robot_outputs/Robot_GAMA_G15_DC_tile_000.txt'
            2. 'Offset'- value of offset to be adjusted to all magnets in millimetre(mm)
                +ve values will initiate radial outward movement of the magnets
                -ve values will initiate radial inward movement of the magnets
                e.g. offset = 0.02
            3. 'T_observed' and 'T_configured'- 
                # T_observed > T_configured then radial inward movement by magnet, assuming the plate has expanded
                # T_observed < T_configured then radial outward movement by magnet, assuming the plate has contracted
                # ΔT = T_configured - T_observed
                # coeff. of thermal expansion, alpha = 1.2 × 10^−6 K^(−1),[Common grades of Invar, measured between 20 °C and 100 °C]
                # offset = plate_radius x ( alpha ⋅ ΔT)
                e.g. T_observed = 15, T_configured = 20
            4. 'robot_centre', 'plate_radius', 'alpha'- These values are known and hence have their default values, but can 
                also be input to account for changes in future, if any.
                Default values: 'robot_centre = (324.470,297.834)', 'plate_radius=226.0 mm', 'alpha= 1.2 × 10^−6'
                apply_metr_calib - if True, apply the metrology-based calibration, using coefficients fitted to measurments
                    in 'robot_shifts_file'
                'robot_shifts_file' - path and filename for the output file of the metrology measurements
    OUTPUT:  A file with offset value adjusted saved in same directory as input file, with extended name of '_radialOffsetAdjusted' at the end.   
    '''

    if plate_radius == None:
        plate_radius = 226.0

    if robot_centre == None:
        robot_centre = [324.470,297.834]

    if alpha == None:
        alpha = 1.2 * (10**(-6))

    if T_observed != -10000 and T_configured != -10000:
        delta_T = T_configured - T_observed
        offset = plate_radius * ( delta_T * alpha )  
        #print('Offset calculated based on temperature change = '+str(offset)+' mm')

    # getting count of lines to skip at top of file, which contain other information
    with open(filename) as file:
        line = file.readline()
        skipline_count = 0
        description = [line]

        # count and store only the description lines not starting with #
        while (line.startswith('#') == False):
            line = file.readline()
            if line[0] != '#':
                description += [line]
            skipline_count += 1


    df_read = pd.read_csv(filename, sep=',', skiprows=skipline_count)

    # renaming the '#Magnet' column name to remove the '#'
    df = df_read.rename(columns={'#Magnet': 'Magnet'})

    # updating description headers with parameters
    description[2] = description[2].replace('-9999', str(offset))
    description[5] = description[5].replace('1.0', str((round(offset/plate_radius,5) + 1)))

    if T_observed != -10000 and T_configured != -10000:
        description[3] = description[3].replace('9999.0000', str(T_configured))
        description[4] = description[4].replace('9999.0000', str(T_observed))

    magnet_count = len(df)

    print(f"Offset is {offset}")
    for i in range(magnet_count):
        
        if df.loc[i, 'Magnet'] == 'circular_magnet':

            x = df.loc[i, 'Center_x'] - robot_centre[0]
            y = df.loc[i, 'Center_y'] - robot_centre[1]
            theta = atan(y/x)
            

            delta_x = cos(theta) * offset
            delta_y = sin(theta) * offset

            print(f"dX is {delta_x}")
            print(f"dY is {delta_y}")
            if x >= 0:
                delta_x *= 1
                delta_y *= 1
                #df.loc[i, 'Center_x'] = df.loc[i, 'Center_x'] + delta_x
                #df.loc[i, 'Center_y'] = df.loc[i, 'Center_y'] + delta_y
            else:
                delta_x *= -1
                delta_y *= -1
                #df.loc[i, 'Center_x'] = df.loc[i, 'Center_x'] - delta_x
                #df.loc[i, 'Center_y'] = df.loc[i, 'Center_y'] - delta_y

            df.loc[i, 'Center_x'] = df.loc[i, 'Center_x'] + delta_x
            df.loc[i, 'Center_y'] = df.loc[i, 'Center_y'] + delta_y

            for j in range(magnet_count):
        
                if df.loc[j, 'Magnet'] == 'rectangular_magnet' and df.loc[j, 'Index'] == df.loc[i, 'Index']:
                    
                    [x_rect,y_rect] = calculate_rectangular_magnet_center_coordinates(x, y, delta_x, delta_y, df.loc[i, 'rectMag_inputOrientation'])

                    print(f"i is {i}, j is {j}, Circular magnet is at {df.loc[i, 'Center_x']}, old_centre is {df.loc[j, 'Center_x']}, new centre is {x_rect + robot_centre[0]}")
                    df.loc[j, 'Center_x'] = x_rect + robot_centre[0]
                    df.loc[j, 'Center_y'] = y_rect + robot_centre[1]


    # Perform metrology-based calibration
    if apply_metr_calib:
        orig_coords = np.array([df['Center_x'], df['Center_y']]).T
        theta_d = df['rot_platePlacing']
        metr_calibrated_coords, calibd_theta_d = perform_metrology_calibration(orig_coords, theta_d,
                                                           robot_centre=robot_centre, robot_shifts_file=robot_shifts_file)
        df['Center_x'] = metr_calibrated_coords[:, 0]
        df['Center_y'] = metr_calibrated_coords[:, 1]
        df['rot_platePlacing'] = calibd_theta_d

    outputFile = filename[:-4] + '_radialOffsetAdjusted.csv'

    # write the description from input robot file at top of final output robot file
    with open(outputFile, 'w+') as f:
        
        for i in description:
            f.write(i)


    df = df.rename(columns={'Magnet': '#Magnet'})
    df.to_csv(outputFile, index=False, sep=',', mode='a')

    print('Output file produced by adjusting offset values to the input file')
    return df

# calculating circular magnet orientation based on magnet center, categorizing them in four quadrants
def calculate_circular_magnet_orientation(x,y):

    if x >= 0 and y >= 0:
        circular_magnet_orientation = atan(abs(y/x))

    elif x < 0 and y >= 0:
        circular_magnet_orientation = pi - atan(abs(y / x))

    elif x < 0 and y < 0:
        circular_magnet_orientation = pi + atan(abs(y / x))

    elif x >= 0 and y < 0:
        circular_magnet_orientation = 2 * pi - atan(abs(y / x))

    # converting orientation form from radians to degrees
    circular_magnet_orientation = degrees(circular_magnet_orientation)

    return circular_magnet_orientation

# calculating the rectangular magnet orientation with respect to the circular magnet
def calculate_rectangular_magnet_orientation(x,y,rectangular_magnet_input_orientation):

    circular_magnet_orientation = calculate_circular_magnet_orientation(x,y)

    #print(f"Circiular magnet orientation is {circular_magnet_orientation}")
    #print(f"rectangular_magnet_input_orientation is {degrees(rectangular_magnet_input_orientation)}" )
    rectangular_magnet_absolute_orientation_degree = 90 - circular_magnet_orientation - degrees(rectangular_magnet_input_orientation)
    #print(f"rectangular_magnet_absolute_orientation_degree is {rectangular_magnet_absolute_orientation_degree}")

    #rectangular_magnet_absolute_orientation_degree = 270 - np.degrees(self.angs)

    return rectangular_magnet_absolute_orientation_degree

# calculating the rectangular magnet center coordinates from its orientation, and categorizing in four quadrants
def calculate_rectangular_magnet_center_coordinates(x,y, delta_x, delta_y, rectangular_magnet_input_orientation):

    # Calculate the 
    rectangular_magnet_absolute_orientation_degree = calculate_rectangular_magnet_orientation(x,y,rectangular_magnet_input_orientation)

    rectangular_magnet_orientation_modulo_radians = convert_modulus_angle(radians(90 - rectangular_magnet_absolute_orientation_degree))

    circular_rectangle_magnet_center_distance = 27.2

    new_x = x + delta_x
    new_y = y + delta_y
    if 0 < rectangular_magnet_orientation_modulo_radians <= pi / 2:

        rectangular_magnet_center =  [new_x + circular_rectangle_magnet_center_distance * cos(rectangular_magnet_orientation_modulo_radians), \
                                        new_y + circular_rectangle_magnet_center_distance * sin(rectangular_magnet_orientation_modulo_radians)]

    elif pi / 2 < rectangular_magnet_orientation_modulo_radians <= pi:

        rectangular_magnet_center = [new_x - circular_rectangle_magnet_center_distance * cos(pi - rectangular_magnet_orientation_modulo_radians), \
                                      new_y + circular_rectangle_magnet_center_distance * sin(pi - rectangular_magnet_orientation_modulo_radians)]

    elif pi < rectangular_magnet_orientation_modulo_radians <= 3 * pi / 2:

        rectangular_magnet_center = [new_x - circular_rectangle_magnet_center_distance * sin(3 * pi / 2 - rectangular_magnet_orientation_modulo_radians), \
                                      new_y - circular_rectangle_magnet_center_distance * cos(3 * pi / 2 - rectangular_magnet_orientation_modulo_radians)]

    elif 3 * pi / 2 < rectangular_magnet_orientation_modulo_radians <= 2 * pi:

        rectangular_magnet_center = [new_x + circular_rectangle_magnet_center_distance * cos(2 * pi - rectangular_magnet_orientation_modulo_radians),
                                      new_y - circular_rectangle_magnet_center_distance * sin(2 * pi - rectangular_magnet_orientation_modulo_radians)]
    else:
            raise ValueError(f"This rectangular magnet has an orientation of {rectangular_magnet_orientation_modulo_radians} and I can't calculate its centre")

    return rectangular_magnet_center

# function to keep angle in range of 0 to 2pi
def convert_modulus_angle(angle):
    piX2 = 2 * pi

    if angle > piX2:
        angle = angle - piX2
    if angle < 0:
        angle = piX2 - abs(angle)

    return angle


def perform_metrology_calibration(input_coords, input_theta_d, robot_centre, robot_shifts_file):
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
    output_theta_d = input_theta_d + theta_d ### SIGN ISSUE: If rotation direction is incorrect, change this + to -

    print('Applied metrology-based calibration, using the fitted coefficients:')
    print(popt)

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




if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("filename", type=str, help='The object frame number you want to display')
    parser.add_argument("robot_shifts_file")

    args = parser.parse_args()

    radialOffset_standaloneFunction(args.filename, robot_shifts_file=args.robot_shifts_file)

### CALLING THE FUNCTION: provide the filename location and offset value or Temperatures as input to function as shown below

# filename = 'C:/Users/Asus/Desktop/Hector_July/Robot_FinalFormat_G12_tile_000.csv'
# radialOffset_standaloneFunction(filename,None,15,15) # direct value of offset as input

# # Example specifying the metrology measurement file:
# filename = './Robot_FinalFormat_tile_A3391_A3395_commissioning_tile_2_CONFIGURED_correct_header.csv'
# radialOffset_standaloneFunction(filename,None,15,15, robot_shifts_file='./robot_shifts_abs.csv')

# T_observed = 30
# T_configured = 23.0
# radialOffset_standaloneFunction(filename,-10000, T_observed, T_configured) # Observed and configured temperatures as input


