import pandas as pd
pd.options.mode.chained_assignment = None  # disabled warning about writes making it back to the original frame
from math import atan,sin,cos,sqrt,degrees,radians,pi


def radialOffset_standaloneFunction(filename, offset=-9999, T_observed=-10000, T_configured=-10000, robot_centre=None, plate_radius=None, alpha=None):
    
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
                Default values: 'robot_centre = (329.5,300)', 'plate_radius=226.0 mm', 'alpha= 1.2 × 10^−6'
    OUTPUT:  A file with offset value adjusted saved in same directory as input file, with extended name of '_radialOffsetAdjusted' at the end.   
    '''

    if plate_radius == None:
        plate_radius = 226.0

    if robot_centre == None:
        #robot_centre = [329.5,300.0]
        robot_centre = [324.47, 297.834]

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
        
        if df['Magnet'][i] == 'circular_magnet':

            x = df['Center_x'][i]-robot_centre[0]
            y = df['Center_y'][i]-robot_centre[1]
            theta = atan(y/x)
            

            delta_x = cos(theta) * offset
            delta_y = sin(theta) * offset

            #print(f"dX is {delta_x}")
            #print(f"dY is {delta_y}")

            if x >= 0:
                df['Center_x'][i] = df['Center_x'][i] + delta_x
                df['Center_y'][i] = df['Center_y'][i] + delta_y
            else:
                df['Center_x'][i] = df['Center_x'][i] - delta_x
                df['Center_y'][i] = df['Center_y'][i] - delta_y

            for j in range(magnet_count):
        
                if df['Magnet'][j] == 'rectangular_magnet' and df['Index'][j] == df['Index'][i]:
                    
                    [x_rect,y_rect] = calculate_rectangular_magnet_center_coordinates(x + delta_x, y + delta_y, df['rectMag_inputOrientation'][i])


                    df['Center_x'][j] = x_rect + robot_centre[0]
                    df['Center_y'][j] = y_rect + robot_centre[1]
                    print(f"i is {i}, j is {j}, Circular magnet is at {df['Center_x'][i]}, old_centre is {df['Center_x'][j]}, new centre is {x_rect + robot_centre[0]}")


    outputFile = filename[:-4] + '_radialOffsetAdjusted.csv'


    # write the description from input robot file at top of final output robot file
    with open(outputFile, 'w+') as f:
        
        for i in description:
            f.write(i)


    df = df.rename(columns={'Magnet': '#Magnet'})
    df.to_csv(outputFile, index=False, sep=',', mode='a')

    print('Output file produced by adjusting offset values to the input file')


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
    olf_rectangular_magnet_absolute_orientation_degree = 90 - circular_magnet_orientation - degrees(rectangular_magnet_input_orientation)
    #print(f"rectangular_magnet_absolute_orientation_degree is {rectangular_magnet_absolute_orientation_degree}")

    rectangular_magnet_absolute_orientation_degree = 270 - np.degrees(self.angs)

    return rectangular_magnet_absolute_orientation_degree

# calculating the rectangular magnet center coordinates from its orientation, and categorizing in four quadrants
def calculate_rectangular_magnet_center_coordinates(x,y,rectangular_magnet_input_orientation):

    rectangular_magnet_absolute_orientation_degree = calculate_rectangular_magnet_orientation(x,y,rectangular_magnet_input_orientation)

    rectangular_magnet_orientation_modulo_radians = convert_modulus_angle(radians(90 - rectangular_magnet_absolute_orientation_degree))

    circular_rectangle_magnet_center_distance = 27.2

    if 0 < rectangular_magnet_orientation_modulo_radians <= pi / 2:

        rectangular_magnet_center =  [x + circular_rectangle_magnet_center_distance * cos(rectangular_magnet_orientation_modulo_radians), \
                                        y + circular_rectangle_magnet_center_distance * sin(rectangular_magnet_orientation_modulo_radians)]

    elif pi / 2 < rectangular_magnet_orientation_modulo_radians <= pi:

        rectangular_magnet_center = [x - circular_rectangle_magnet_center_distance * cos(pi - rectangular_magnet_orientation_modulo_radians), \
                                      y + circular_rectangle_magnet_center_distance * sin(pi - rectangular_magnet_orientation_modulo_radians)]

    elif pi < rectangular_magnet_orientation_modulo_radians <= 3 * pi / 2:

        rectangular_magnet_center = [x - circular_rectangle_magnet_center_distance * sin(3 * pi / 2 - rectangular_magnet_orientation_modulo_radians), \
                                      y - circular_rectangle_magnet_center_distance * cos(3 * pi / 2 - rectangular_magnet_orientation_modulo_radians)]

    elif 3 * pi / 2 < rectangular_magnet_orientation_modulo_radians <= 2 * pi:

        rectangular_magnet_center = [x + circular_rectangle_magnet_center_distance * cos(2 * pi - rectangular_magnet_orientation_modulo_radians),
                                      y - circular_rectangle_magnet_center_distance * sin(2 * pi - rectangular_magnet_orientation_modulo_radians)]
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



# ### CALLING THE FUNCTION: provide the filename location and offset value or Temperatures as input to function as shown below 

# filename = 'C:/Users/Asus/Desktop/Hector_July/Robot_FinalFormat_G12_tile_000.csv'

# radialOffset_standaloneFunction(filename,None,15,35) # direct value of offset as input

# # T_observed = 30
# # T_configured = 23.0
# # radialOffset_standaloneFunction(filename,-10000, T_observed, T_configured) # Observed and configured temperatures as input



