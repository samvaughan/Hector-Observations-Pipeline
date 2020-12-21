from .conflicts.functions import create_list_of_blocking_magnets,\
                                all_blocking_magnets_are_fully_blocked
from ..general_operations.trigonometry import convert_radians_to_degrees
from .hexabundle import create_Magnet_Dictionary, get_probe_count, star_hexabundle_allocation, \
                                cores91_hexabundles_sortedByMstar, resolve_hexabundle_allocation_repeats, \
                                resolve_unallocated_hexabundle, check_for_hexabundle_allocation_repeats, \
                                check_for_unallocated_hexabundle, update_galaxyIDrecord_withHexabundle, \
                                hexabundle_allocation_fromMagnetDict, overall_hexabundle_size_allocation_operation_version1, \
                                overall_hexabundle_size_allocation_operation_version2_median, \
                                overall_hexabundle_size_allocation_operation_version3_largerBundlePriority
from ..problem_operations.robot_parameters import assign_magnet_labels, add_pickupDirection, \
                                                assign_preferable_pickupDirection, calculate_pickup_putdown_angles, \
                                                check_for_negative_twopi_angle, check_for_unresolvable_conflict
from ..hector.constants import robot_center_x,robot_center_y
import numpy as np
import string
import sys
import re

def calculate_placement_ordering_of_blocked_magnet(blocked_magnet,list_of_conflicts,list_of_fully_blocked_magnets):

    blocking_magnets = create_list_of_blocking_magnets(list_of_conflicts, blocked_magnet)

    for i in range(50):

        if not all_blocking_magnets_are_fully_blocked(blocking_magnets,list_of_fully_blocked_magnets):

            blocked_magnet.placement_index += 1

            break

        else:

            blocking_magnets = []

            for magnet in blocking_magnets:
                blocking_magnets.append(create_list_of_blocking_magnets(list_of_conflicts, magnet))


    if i == 49:
        blocked_magnet.placement_index = None
        print('Error ! ',blocked_magnet.__class__.__name__,int(blocked_magnet.index),' cannot be placed')



def calculate_placement_ordering_of_all_blocked_magnets(list_of_fully_blocked_magnets,list_of_conflicts):

    for blocked_magnet in list_of_fully_blocked_magnets:
        calculate_placement_ordering_of_blocked_magnet(blocked_magnet,list_of_conflicts,list_of_fully_blocked_magnets)


def create_position_ordering_array(all_magnets, fully_blocked_magnets, conflicted_magnets, galaxyIDrecord, \
                                   clusterNum, tileNum, conflictFile, flagsFile):

    calculate_placement_ordering_of_all_blocked_magnets(fully_blocked_magnets, conflicted_magnets)

    max_order = max(magnet.placement_index for magnet in all_magnets if magnet.placement_index is not None)

    position_ordering_array = []

    # magnet label and guide hexabundle index
    index1 = 1
    guideIndex = 1

    # carrying out the whole hexabundle allocation algorithm from hexabundles.py script
    galaxyIDrecord, MagnetDict = overall_hexabundle_size_allocation_operation_version3_largerBundlePriority(all_magnets, \
                                                                        galaxyIDrecord, clusterNum, tileNum, flagsFile)

    for magnet in all_magnets:

        # identifying possible pickup directions available for magnet
        available_pickup = [area.code for area in magnet.pickup_areas]

        # checking for magnets with unresolvable conflicts, which are flagged and assigned order of '0'
        order = check_for_unresolvable_conflict(magnet,conflictFile,max_order,clusterNum,tileNum)

        # adding magnet labels of rectangular: R01.. ,and circular: Blu,Mag,Gre,Yel
        magnet,index1 = assign_magnet_labels(magnet,index1)

        # adding pickup direction to placement orders of 1 in the case of more than 1 placement orders
        magnet, available_pickup = add_pickupDirection(magnet, available_pickup)

        # selecting only 1 pickup direction based on preference for circular: TR,TL,RO,RI and rectangular: I,O
        available_pickup = assign_preferable_pickupDirection(available_pickup)

        # assigning allocated hexabundles to respective magnet as stored in Magnet Dictionary
        magnet, guideIndex = hexabundle_allocation_fromMagnetDict (MagnetDict,magnet,guideIndex)

        # calculating rotation angles for pickup and putdown based on available pickup direction
        magnet = calculate_pickup_putdown_angles(magnet,available_pickup)

        # checking rotation angle to ensure it is within range of 0 to 360 degrees
        magnet.rotation_putdown = check_for_negative_twopi_angle(magnet.rotation_putdown)

        ## ** Might need to change center coordinates of magnets to different scale for considering optical and other type of distortion **

        # storing all the parameters in positioning array
        f = np.append([magnet.__class__.__name__, str(magnet.magnet_label), str(robot_center_x+magnet.view_x), \
                       str(robot_center_y+magnet.view_y), str(magnet.rotation_pickup), str(round(magnet.rotation_putdown,2)), \
                       str(order), available_pickup, str(float(magnet.IDs)), str(int(magnet.index))], str(magnet.hexabundle))
        position_ordering_array.append(np.array(f))


    return np.array(position_ordering_array),galaxyIDrecord


