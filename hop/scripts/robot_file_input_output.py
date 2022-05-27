import pandas as pd
pd.options.mode.chained_assignment = None  # disabled warning about writes making it back to the original frame
import numpy as np
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt
import warnings
from pathlib import Path

"""
Various functions to read in and write out the files we need
"""

def read_parking_positions_file(filename):
    """
    Read in the oddly formatted parking positions file. Be very sure to get the right lines here, or error if things aren't as expected. Don't just guess line numbers!
    """

    # The file is small so read the whole thing
    with open(filename, 'r') as f:
        all_lines = f.readlines()
    # Now find the lines where the each magnet type starts
    try:
        rectangular_magnets_start_index = all_lines.index('Rectangular Magnets, , , , \n')
        blue_magnets_start_index = all_lines.index('Round Blue Magnets, , , , \n')
        green_magnets_start_index = all_lines.index('Round Green Magnets, , , , \n')
        yellow_magnets_start_index = all_lines.index('Round Yellow Magnets, , , , \n')
        magneta_magnets_start_index = all_lines.index('Round Magneta Magnets, , , , \n')
    except ValueError:
        raise ValueError("This file is not in the expected format- can't find where each magnet type starts. Exiting")


    rectangular_magnets = all_lines[rectangular_magnets_start_index+1:blue_magnets_start_index]
    blue_magnets = all_lines[blue_magnets_start_index+1:green_magnets_start_index]
    green_magnets = all_lines[green_magnets_start_index+1:yellow_magnets_start_index]
    yellow_magnets = all_lines[yellow_magnets_start_index+1:magneta_magnets_start_index]
    magneta_magnets = all_lines[magneta_magnets_start_index+1:]

    df = pd.DataFrame()
    for data, name in zip([rectangular_magnets, blue_magnets, green_magnets, yellow_magnets, magneta_magnets], ['RectangularMagnet', 'BlueCircularMagnet', 'GreenCircularMagnet', 'YellowCircularMagnet', 'MagnetaCircularMagnet']):
        magnet_df = pd.DataFrame([l.rstrip().split(',') for l in data], columns=['MagnetNumber', 'Center_x', 'Center_y', 'z', 'zero'])
        # Make all the values floats/integers
        magnet_df = magnet_df.apply(pd.to_numeric)
        
        magnet_df['MagnetName'] = name

        #Now add the magnet type for each- either rectangular or circular
        if name == 'RectangularMagnet':
            magnet_df['Magnet'] = 'rectangular_magnet'
        else:
            magnet_df['Magnet'] = 'circular_magnet'
        # Now add the rot_plate_placing value for each magnet.
        # CHECK WITH JULIA- is this 0.0?
        magnet_df['rot_platePlacing'] = 0.0

        df = pd.concat((df, magnet_df))

    #This line is very important! Otherwise in the for loop in the main file gets the rows all wrong
    df = df.reset_index(drop=True)
    
    return df

def write_standard_parking_positions_file(df, output_file):
    """
    Write out the parking positions file in the silly format it needs
    """

    with open(output_file, 'w') as f:
        f.write("Rectangular Magnets, , , , \n")
        df.loc[df.MagnetName == "RectangularMagnet"].to_csv(f, index=False, sep=',', mode='a', columns=['MagnetNumber','Center_x','Center_y','z','zero'], header=False, line_terminator='\n')

        # Now the Blue circular magnets
        f.write('Round Blue Magnets, , , , \n')
        df.loc[df.MagnetName == "BlueCircularMagnet"].to_csv(f, index=False, sep=',', mode='a', columns=['MagnetNumber','Center_x','Center_y','z','zero'], header=False, line_terminator='\n')

        # Round Green
        f.write("Round Green Magnets, , , , \n")
        df.loc[df.MagnetName == "GreenCircularMagnet"].to_csv(f, index=False, sep=',', mode='a', columns=['MagnetNumber','Center_x','Center_y','z','zero'], header=False, line_terminator='\n')

        # Round Yellow
        f.write("Round Yellow Magnets, , , , \n")
        df.loc[df.MagnetName == "YellowCircularMagnet"].to_csv(f, index=False, sep=',', mode='a', columns=['MagnetNumber','Center_x','Center_y','z','zero'], header=False, line_terminator='\n')

        # Round Magenta
        f.write("Round Magneta Magnets, , , , \n")
        df.loc[df.MagnetName == "MagnetaCircularMagnet"].to_csv(f, index=False, sep=',', mode='a', columns=['MagnetNumber','Center_x','Center_y','z','zero'], header=False, line_terminator='\n')

    return df

def read_standard_robot_file(filename):
    """
    Read in a standard robot file and return a pandas dataframe
    """

    df_read = pd.read_csv(filename, sep=',', skiprows=6)
    # renaming the '#Magnet' column name to remove the '#'
    df = df_read.rename(columns={'#Magnet': 'Magnet'})
    return df

def write_standard_robot_file(df, filename, header):
    """
    Write out the robot file in the expected format (with the header properly formatted)
    """

    # With that done, write the output file with the expected file ending
    fname = Path(filename)
    output_file = fname.parent / (fname.stem + '_CorrectionsApplied.csv')

    # Replace the # at the start of the line with column names
    df = df.rename(columns={'Magnet': '#Magnet'})

    # write the description from input robot file at top of final output robot file
    with open(output_file, 'w') as f:
        for line in header:
            f.write(line)
    df.to_csv(output_file, index=False, sep=',', mode='a')

    return df, output_file