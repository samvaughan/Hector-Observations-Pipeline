import numpy as np
import pandas as pd
from io import StringIO
import string
from ..hector.probe import probe
# from ..problem_operations. offsets import radialPositionOffset
from ..problem_operations.robot_parameters import assign_magnet_labels

# derive floats only and if the format is different then return 0 instead of a ValueError
def parse_col(s):
    try:
        return float(s)
    except ValueError:
        return 0

def parse_col_string(s):
    try:
        return float(s)
    except ValueError:
        return s

def remove_apostrophes(s):
    try:
        return float(s)
    except ValueError:
        s.decode('utf-8')
        return s.replace("\"", "")

# extracting the list of probes with all the respective parameters required from the file
def create_list_of_probes_from_file(file,guideFileList):

    df = pd.read_csv(file,sep=',')

    # print list of headers for the file
    print(df.keys())

    count_split = 0
    for i in df['probe']:
        if 0<i<28 :
            count_split += 1

    # Only get the galaxy and standard star hexas
    mask = (df.type == 1) | (df.type == 0)
    df_1 = df[mask]
    df_2 = df[~mask]

    print("Shape of new dataframes - {} , {}".format(df_1.shape, df_2.shape))

    probe_number = list(df_1['probe'])
    probe_number = [round(num) for num in probe_number]
    probe_number += guideFileList[0]

    IDs = list(df_1['ID']) + guideFileList[1]
    circular_magnet_center_x = list(df_1['x']) + guideFileList[2]
    circular_magnet_center_y = list(df_1['y']) + guideFileList[3]
    rads = list(df_1['rads']) + guideFileList[4]
    angs = list(df_1['angs']) + guideFileList[5]
    azAngs = list(df_1['azAngs']) + guideFileList[6]
    rectangle_magnet_input_orientation = list(df_1['angs_azAng']) + guideFileList[7]

    galaxyORstar = list(df_1['type'])
    galaxyORstar = [round(num) for num in galaxyORstar]
    galaxyORstar += guideFileList[8]

    Re = list(df_1['Re']) + guideFileList[9]
    mu_1re = list(df_1['GAL_MU_E_R']) + guideFileList[10]
    Mstar = list(df_1['Mstar']) + guideFileList[11]

    # PROBES list created
    list_of_probes = []

    # creating rectangular magnet label
    magnet_label = [None]*len(probe_number)

    # creating hexabundle array
    hexabundle = 'NA' * len(probe_number)

    # creating rotation for pickup and putdown
    rotation_pickup = [None]*len(probe_number)
    rotation_putdown = [0]*len(probe_number)

    # appending all the probes to the probes list
    i = 0
    for each_probe in probe_number:
        list_of_probes.append(probe(probe_index=probe_number[i],
                                    circular_magnet_center=[circular_magnet_center_x[i],circular_magnet_center_y[i]],
                                     rectangular_magnet_input_orientation=rectangle_magnet_input_orientation[i],
                                     galaxyORstar=galaxyORstar[i],
                                     Re=Re[i],
                                     mu_1re=mu_1re[i],
                                     Mstar=Mstar[i],
                                     magnet_label=magnet_label[i],
                                     hexabundle=hexabundle[i],
                                     rads=rads[i],
                                     rotation_pickup=rotation_pickup[i],
                                     rotation_putdown=rotation_putdown[i],
                                     azAngs=azAngs[i],
                                     IDs=IDs[i],
                                     angs=angs[i]))

        i += 1

    return list_of_probes

# creating a list of circular and rectangular magnets separately from the probes list
def create_list_of_circular_and_rectangular_magnets_from_file(file,guideFileList):  #,magnetPair_offset):

    # creating probes list from file
    list_of_probes = create_list_of_probes_from_file(file,guideFileList)

    ## Created as a standalone function for the robot, so should not be required to implement in this pipeline
    # adjusting the radial position offsets to the magnet pair due to thermal expansion
    # list_of_probes = radialPositionOffset(list_of_probes, magnetPair_offset)

    # circular magnet list created
    list_of_circular_magnet = []

    # extracting the circular magnet parameters respectively from the probes list
    for each_probe in list_of_probes:
        list_of_circular_magnet.append(each_probe.extract_circular_magnet_parameters())

    # rectangular magnet list created
    list_of_rectangular_magnet = []

    # extracting the rectangular magnet parameters respectively from the probes list
    for each_probe in list_of_probes:
        list_of_rectangular_magnet.append(each_probe.extract_rectangular_magnet_parameters())


    return list_of_circular_magnet,list_of_rectangular_magnet

def create_list_of_all_magnets_from_file(file,guideFileList):   #,magnetPair_offset):

    # extracting circular and rectangular magnets list from the list of probes which is first extracted from file
    [circular_magnets, rectangular_magnets] = create_list_of_circular_and_rectangular_magnets_from_file(file,guideFileList)  #,magnetPair_offset)

    all_magnets = np.concatenate([circular_magnets, rectangular_magnets])

    # magnet label index
    index = 1

    for magnet in all_magnets:

        # adding magnet labels of rectangular: R01.. ,and circular: Blu,Mag,Gre,Yel
        magnet,index = assign_magnet_labels(magnet,index)

    return all_magnets#np.concatenate([circular_magnets, rectangular_magnets])

# quick function to read a filename
def get_file(filename):
    return open(filename, "r")