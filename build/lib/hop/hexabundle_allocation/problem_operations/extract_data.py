import numpy as np
from ..hector.probe import probe
from ..problem_operations. offsets import radialPositionOffset


def parse_col(s):
    try:
        return float(s)
    except ValueError:
        return 0

def create_list_of_probes_from_file(file):

    probe_number, \
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
    Mstar = np.loadtxt(file, skiprows=1, unpack=True, converters={12:parse_col,13:parse_col,24:parse_col}, usecols =[1,9,3,4,5,6,7,8,43,12,24,13])

    list_of_probes = []

    # creating rectangular magnet label
    magnet_label = [None]*len(probe_number)

    # creating hexabundle array
    hexabundle= 'NA' * len(probe_number)

    # creating rotation for pickup and putdown
    rotation_pickup = [None]*len(probe_number)
    rotation_putdown =[0]*len(probe_number)


    i = 0
    for each_probe in probe_number:
        list_of_probes.append(probe(probe_number[i],
                                    [circular_magnet_center_x[i],circular_magnet_center_y[i]],
                                     rectangle_magnet_input_orientation[i],
                                     galaxyORstar[i],
                                     Re[i],
                                     mu_1re[i],
                                     Mstar[i],
                                     magnet_label[i],
                                     hexabundle[i],
                                     rads[i],
                                     rotation_pickup[i],
                                     rotation_putdown[i],
                                     azAngs[i],
                                     IDs[i]))

        i += 1

    return list_of_probes

def create_list_of_circular_and_rectangular_magnets_from_file(file,magnetPair_offset):

    list_of_probes = create_list_of_probes_from_file(file)

    list_of_probes = radialPositionOffset(list_of_probes, magnetPair_offset)
    ## ***** offset adjustments for magnet pair
    # for item in magnetPair_offset:
    #     for each_probe in list_of_probes:
    #         if item[0] == each_probe.index:
    #             # print(each_probe.circular_magnet_center)
    #             # each_probe.circular_magnet_center[0] += item[1]
    #             # each_probe.circular_magnet_center[1] += item[2]
    #             # print(each_probe.circular_magnet_center)
    #
    #             theta = atan(each_probe.circular_magnet_center[1] / each_probe.circular_magnet_center[0])
    #             each_probe.circular_magnet_center = (each_probe.circular_magnet_center[0] + (cos(theta) * item[1]), \
    #                                                  each_probe.circular_magnet_center[1] + (sin(theta) * item[1]))
    ## *****

    list_of_circular_magnet = []

    for each_probe in list_of_probes:
        list_of_circular_magnet.append(each_probe.extract_circular_magnet_parameters())

    list_of_rectangular_magnet = []

    for each_probe in list_of_probes:
        list_of_rectangular_magnet.append(each_probe.extract_rectangular_magnet_parameters())

    return list_of_circular_magnet,list_of_rectangular_magnet

def create_list_of_all_magnets_from_file(file,magnetPair_offset):

    [circular_magnets, rectangular_magnets] = create_list_of_circular_and_rectangular_magnets_from_file(file,magnetPair_offset)

    # # ***** working function for adjusting magnet pair position, needs to be moved to offsets and be controlled from main
    # for magnet in circular_magnets:
    #     if magnet.index == 13:
    #         magnet.center[0] += 20
    #
    # for magnet in rectangular_magnets:
    #     if magnet.index == 13:
    #         magnet.center[0] += 20

    return np.concatenate([circular_magnets, rectangular_magnets])


def get_file(filename):
    return open(filename, "r")