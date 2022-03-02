import matplotlib.pyplot as plt
import pandas as pd
pd.options.mode.chained_assignment = None  # disabled warning about writes making it back to the original frame
import numpy as np
import csv
import datetime
import re
import time

# arranging the guides files in consistent format with hexas files and outputting them separately as well
def arrange_guidesFile(fileNameHexa,fileNameGuides, guide_outputFile):

    # getting count of lines to skip at top of file, which contain other information
    with open(fileNameGuides) as f:
        line = f.readline()
        skipline_count = 0
        description = line
        while line.startswith('#'):
            line = f.readline()
            if line[0] == '#':
                description += line
            skipline_count += 1

    # creating dataframe for guide probes from guides config file
    df_guides = pd.read_csv(fileNameGuides, sep=',', comment='#')

    # crate a probe column to keep count to the guides dataframe
    #df_guides.insert(loc=0, column='probe', value=0)

    # creating data
    # frame for just galaxy probes from hexa config file
    df = pd.read_csv(fileNameHexa, sep=',', comment='#')
    mask = (df['type'] == 1) | (df['type'] == 0)
    df_hexas = df[mask]

    # reading the index value of last hex probe to get count of hexa probes
    hexaCount = df_hexas.probe.max()#int(df_hexas.loc[df_hexas.index[-1], 'probe']) + 1

    # probe numbering for the guide probes, counting onward from hexa count
    df_guides['probe'] = np.arange(hexaCount + 1, hexaCount + 1 + len(df_guides))

    # creating columns for guide file list
    probe_number = list(df_guides['probe'])
    circular_magnet_center_x = list(df_guides['x'])
    circular_magnet_center_y = list(df_guides['y'])
    rads = list(df_guides['rads'])
    angs = list(df_guides['angs'])
    azAngs = list(df_guides['azAngs'])
    rectangle_magnet_input_orientation = list(df_guides['angs_azAng'])
    IDs = list(df_guides['ID'])

    # assigning 'NA' to parameters which guide probes do not have a value for
    galaxyORstar = Re = mu_1re = Mstar = [float('NaN')] * len(probe_number)

    # setting guide file extracted list ready for output
    guideFileList = [probe_number, \
                     IDs, \
                     circular_magnet_center_x, \
                     circular_magnet_center_y, \
                     rads, \
                     angs, \
                     azAngs, \
                     rectangle_magnet_input_orientation, \
                     galaxyORstar, \
                     Re, \
                     mu_1re, \
                     Mstar ]

    # fill out all the empty slots of parameters with NA and save output guide file
    df_guides.fillna('', inplace=True)

    # write the description from config guide file at top of output guide file
    with open(guide_outputFile, 'w') as f:
        f.write(description)

    df_guidesOutput = df_guides.replace('NaN', '', regex=True)
    df_guidesOutput.to_csv(guide_outputFile, index=False, sep=',', mode='a')


    return df_guides, guideFileList


# merging the hexas and guides file to create one plate file with all the magnets
def merge_hexaAndGuides(fileNameHexa, df_guideFile, plate_file):

    # # getting count of lines to skip at top of file, which contain other information
    # with open(fileNameHexa) as f:
    #     line = f.readline()
    #     skipline_count = 0
    #     while line.startswith('#'):
    #         line = f.readline()
    #         skipline_count += 1

    # creating dataframe for just galaxy probes from hexa config file
    df = pd.read_csv(fileNameHexa,sep=',', comment='#')
    mask = (df.type == 1) | (df.type == 0)
    df_hexabundles = df[mask]

    # joining dataframes of hexa and guide probes
    df_combined = pd.concat([df_hexabundles, df_guideFile], sort=False)

    # fill out all the empty slots of parameters with NA
    df_combined.fillna('', inplace=True)

    # write the joined dataframe of hexa and guide probes on plate file
    df_combined.to_csv(plate_file, index=False, sep=',', quoting=csv.QUOTE_NONE, escapechar=' ')

    return df_combined

# creating the robotFile array for
def create_robotFileArray(tile_label, positioning_array,robotFile,newrow,fully_blocked_magnets_dictionary, robot_temp=-9999, obs_temp=-9999):

    # guide probes do not have ID, so they are allocated a large negative integer
    positioning_array[:, 8] = [i if i != 'nan' else -999999 for i in positioning_array[:, 8]]
    for i in range(len(positioning_array[:, 8])):
        positioning_array[:, 8][i] = round(float(positioning_array[:, 8][i]))

    # an array for the robot file is created which is sorted based on the placement order numbering
    robotFilearray = sorted(positioning_array, key=lambda x: x[6])

    # the title row is inserted at the first row of the array
    robotFilearray = np.insert(robotFilearray, 0, np.array(newrow), 0)

    # add the reposition column to robot file by using the fully blocked magnets dictionary
    robotFilearray = add_repositionCol_to_robotFile(positioning_array,robotFilearray,fully_blocked_magnets_dictionary)

    # delete the column which are not to be included in robot file array
    robotFilearray = np.delete(robotFilearray, [13], 1)

    # TEST PRINT
    #print('\n')
    #print(robotFilearray)

    # write the robot file array into the CSV file for the robot
    with open(robotFile, 'w+') as robotFile:

        robotFile.write(f'#Label, {tile_label}\n')
        robotFile.write(f"#Date_and_Time_file_created, {str(datetime.datetime.now().strftime('%d-%B-%y %H:%M:%S'))}\n")

        robotFile.write('#Radial_Offset_Adjustment, -9999 #Radial offset is in millimetre(mm) with +ve values actioning radial outward movement and -ve values actioning radial inward movement of the magnets. \n')


        robotFile.write(f'#RobotTemp, {robot_temp:.4f} #temperature (degrees C) the distortion code assumed the field would be observed at\n')
        robotFile.write(f'#ObsTemp, {obs_temp:.4f} #actual temperature (degrees C) it is going to be observed at\n')

        robotFile.write('#Radial_Scale_factor, 1.0 #Radial scale factor is thermal coefficient of invar times temperature difference applied radially relative to the plate centre.\n')
        robotFile.write('\n\n')

        # robotFile.write('# Radial Offset Adjustment \n \n')

        writer = csv.writer(robotFile, delimiter=',')
        writer.writerows(robotFilearray)

    return positioning_array,robotFilearray

def add_repositionCol_to_robotFile(positioning_array,robotFilearray,fully_blocked_magnets_dictionary):

    # Creates a list containing w lists, each of h item/s, all filled with 0
    w, h = len(positioning_array[:, 8]) + 1, 1
    nameColumn = [['[0]' for x in range(w)] for y in range(h)]

    # creating a magnet name column, which will be used to refer to when repositioning magnets are required
    for i in range(1,len(robotFilearray)):

        # rectangular magnet being named as 'R01,R02...R27'
        if hasNumbers(robotFilearray[i][1]):
            nameColumn[0][i] = robotFilearray[i][1]

        # circular magnet being named as 'Mag01,Gre02...Blu27'
        else:
            x = np.int16(robotFilearray[i][9])
            nameColumn[0][i] = robotFilearray[i][1] + str('%02d' % x)

    # transposing the list to a column with a title assigned
    nameColumn[0][0] = 'Magnet_title'
    nameColumn = np.transpose(nameColumn)

    # add the 'list' column to the robot file array in desired position
    robotFilearray = np.hstack((robotFilearray[:, :8], nameColumn, robotFilearray[:, 8:]))

    # Creates a list containing w lists, each of h item/s, all filled with 0
    w, h = len(positioning_array[:, 9]) + 1, 1
    rePosition_col = [['[0]' for x in range(w)] for y in range(h)]

    # filling out the created list with the blocked magnets dictionary in order with the robot file array
    for each_magnet in fully_blocked_magnets_dictionary:
        for i in range(len(robotFilearray)):

            # checking for match of blocked magnets with robotfile array
            if (robotFilearray[i][0] + ' ' + str(robotFilearray[i][10])) == each_magnet:
                for j in range(len(fully_blocked_magnets_dictionary[each_magnet])):

                    # formatting as required for the rePosition column
                    if j>0:
                        rePosition_col[0][i] += '_'
                    else:
                        rePosition_col[0][i] = '['

                    # checking for match of each blocking magnet with robotfile array to add to rePosition column
                    for k in range(len(robotFilearray)):
                        if robotFilearray[k][0]+' '+str(robotFilearray[k][10]) == fully_blocked_magnets_dictionary[each_magnet][j]:
                            rePosition_col[0][i] += str(robotFilearray[k][8])

                # formatting as required for the rePosition column
                rePosition_col[0][i] += ']'

    # transposing the list to a column with a title assigned
    rePosition_col[0][0] = 'rePosition_magnets'
    rePosition_col = np.transpose(rePosition_col)

    ## TEST PRINTs
    # print(rePosition_col)
    # print(rePosition_col.shape)
    # print(robotFilearray.shape)

    # add the 'list' column to the robot file array in desired position
    robotFilearray = np.hstack((robotFilearray[:, :9], rePosition_col, robotFilearray[:, 9:]))

    return robotFilearray

def hasNumbers(inputString):
    return any(char.isdigit() for char in inputString)

def positioningArray_adjust_and_mergetoFile(positioning_array, plate_file, outputFile, newrow, newrow_circular):

    # splitting positioning array into circular
    positioning_array_circular = np.vsplit(positioning_array, 2)[0]

    # adding the title row and getting rid of some columns not required
    positioning_array_circular = np.insert(positioning_array_circular, 0, np.array(newrow_circular), 0)
    positioning_array_circular = np.delete(positioning_array_circular, [2, 3, 4, 5, 8, 9, 10, 11, 12], 1)

    # splitting positioning array to keep only rectangular ones
    positioning_array = np.vsplit(positioning_array, 2)[1]

    # adding the title row and getting rid of some columns not required
    positioning_array = np.insert(positioning_array, 0, np.array(newrow), 0)
    positioning_array = np.delete(positioning_array, [2, 3, 4, 5, 8], 1)

    # index for keeping count
    index = 0

    # Open the input_file in read mode and output_file in write mode
    with open(plate_file, 'r') as read_obj, \
            open(outputFile, 'w+', newline='') as write_obj:

        # Create a csv.reader object from the input file object
        csv_reader = csv.reader(read_obj)

        # Create a csv.writer object from the output file object
        csv_writer = csv.writer(write_obj)

        # Read each row of the input csv file as list
        for row in csv_reader:
            
            # roww_circular = np.array2string(positioning_array_circular[index], separator=',',formatter={'str_kind': lambda x: x})
            # roww = np.array2string(positioning_array[index], separator=',', formatter={'str_kind': lambda x: x})

            # # Append the default text in the row / list
            # row.append(str(roww).replace('[', '').rstrip(']'))
            # row.append(str(roww_circular).replace('[', '').rstrip(']'))
            row.extend(positioning_array[index])
            row.extend(positioning_array_circular[index])

            # Add the updated row / list to the output file
            csv_writer.writerow(row)

            # updating index count
            index += 1

    return positioning_array,positioning_array_circular

# final files being formatted to maintain consistency
def finalFiles(all_magnets, outputFile, fileNameHexa):

    # read output file to create probes dataframe
    df_probes = pd.read_csv(outputFile, sep=',',header=0)

    df_probes['offset_P'] = 0.0
    df_probes['offset_Q'] = 0.0

    # update all the center coordinates of circular magnets in probes df with 'all_magnets' list
    # and add the offsets P and Q to be printed out in final tile output file
    for i in all_magnets:

        for j in df_probes['probe']:

            if i.__class__.__name__ == 'circular_magnet' and i.index == int(j):
                df_probes['x'][j-1] = i.center[0]
                df_probes['y'][j-1] = i.center[1]
                df_probes['offset_P'][j-1] = i.offset_P
                df_probes['offset_Q'][j-1] = i.offset_Q

    # getting count of lines to read at top of file, and storing descriptive comments
    with open(fileNameHexa) as f:
        line = f.readline()
        skipline_count = 0
        description = line

        # count and store only lines starting with #
        while line.startswith('#'):
            line = f.readline()
            if line[0] == '#':
                description += line
            skipline_count += 1

    # creating dataframe for just the sky fibres from config hexa files
    df = pd.read_csv(fileNameHexa,sep=',', skiprows=skipline_count)
    hexabundle_mask = df['fibre_type'] == 'P'
    skyfibre_mask = df['fibre_type'] == 'S'
    assert hexabundle_mask.sum() + skyfibre_mask.sum() == len(df)
    df_skyfibre = df.loc[skyfibre_mask]

    # joining the dataframes of hexa nd guide probes and skyfibres
    df_tileOutput = pd.concat([df_probes, df_skyfibre], sort=False)

    # fill out all the empty slots of parameters with ''
    df_tileOutput.fillna('', inplace=True)

    # remove commas due to joining of positioning arrays of circular and rectangular magnets
    #df_tileOutput = df_tileOutput.replace(',', '', regex=True)
    #df_tileOutput.columns = df_tileOutput.columns.str.replace(',', '')

    # write the description from config file at top of final tile output file
    with open(outputFile, 'w') as f:
        f.write(description)

    # write the joined dataframe of probes and skyfibres on final tile output file after the description
    df_tileOutput.to_csv(outputFile, index=False, sep=',', quoting=csv.QUOTE_NONE, escapechar=' ', mode='a')

