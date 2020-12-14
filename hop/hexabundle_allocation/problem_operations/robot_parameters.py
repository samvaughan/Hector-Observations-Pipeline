import numpy as np
import string
import sys
import re
from general_operations.trigonometry import convert_radians_to_degrees


def assign_magnet_labels(magnet,index1):
    # adding labels for rectangular magnet
    if magnet.__class__.__name__ == 'rectangular_magnet':
        magnet.magnet_label = 'R%02d' % (index1)
        index1 += 1

    # adding labels for circular magnet
    elif magnet.__class__.__name__ == 'circular_magnet':
        if magnet.rads < 0.396:
            magnet.magnet_label = 'Blu'
        elif 0.396 <= magnet.rads < 0.627:
            magnet.magnet_label = 'Gre'
        elif 0.627 <= magnet.rads < 0.823:
            magnet.magnet_label = 'Yel'
        elif magnet.rads >= 0.823:
            magnet.magnet_label = 'Mag'

    return magnet,index1


def check_for_unresolvable_conflict(magnet, conflictFile, max_order, clusterNum, tileNum):
    if magnet.placement_index == None:

        order = 0
        with open(conflictFile, 'a') as fp:
            conflict = magnet.__class__.__name__ + ' ' + str(int(magnet.index)) + ' is an unresolvable conflict,' + \
                       ' detected in Cluster ' + str(clusterNum) + ', tile ' + str(tileNum) + '\n'
            fp.write(conflict)

    else:
        order = 1
        for increment in range(max_order + 1):
            if magnet.placement_index == max_order - increment:
                order += increment

    return order


def add_pickupDirection(magnet, available_pickup):

    # magnets with placement order of 1 are placed at first so can be placed from any pickup direction
    if available_pickup == []:
        if magnet.__class__.__name__ == 'circular_magnet':
            available_pickup = ['TR', 'TL', 'RI', 'RO']
        elif magnet.__class__.__name__ == 'rectangular_magnet':
            available_pickup = ['I', 'O']

    return magnet, available_pickup


def assign_preferable_pickupDirection(available_pickup):

    if 'TR' in available_pickup:
        available_pickup = []
        available_pickup = 'TR'
    elif 'TL' in available_pickup:
        available_pickup = []
        available_pickup = 'TL'
    elif 'RO' in available_pickup:
        available_pickup = []
        available_pickup = 'RO'
    elif 'RI' in available_pickup:
        available_pickup = []
        available_pickup = 'RI'
    elif 'I' in available_pickup:  ### REMEMBER TO CHANGE Inwards to Toward and Outwards to Away ###
        available_pickup = []
        available_pickup = 'I'

    return available_pickup


def calculate_pickup_putdown_angles(magnet, available_pickup):
    
    azAngs = convert_radians_to_degrees(magnet.azAngs)
    # rotation for pickup and putdown
    if magnet.__class__.__name__ == 'circular_magnet':
        if 'TR' in available_pickup:
            magnet.rotation_pickup = 90
            if azAngs < 90:
                magnet.rotation_putdown = 90 - azAngs
            elif azAngs >= 90:
                magnet.rotation_putdown = 360 + (90 - azAngs)
        elif 'TL' in available_pickup:
            magnet.rotation_pickup = 270
            if azAngs < 90:
                magnet.rotation_putdown = 270 - azAngs
            elif azAngs >= 90:
                magnet.rotation_putdown = 270 - azAngs
        elif 'RO' in available_pickup:
            magnet.rotation_pickup = 180
            if azAngs < 90:
                magnet.rotation_putdown = 180 - azAngs
            elif azAngs >= 90:
                magnet.rotation_putdown = 180 - azAngs
        elif 'RI' in available_pickup:
            magnet.rotation_pickup = 0
            if azAngs < 90:
                magnet.rotation_putdown = 360 - azAngs
            elif azAngs >= 90:
                magnet.rotation_putdown = 360 - azAngs


    elif magnet.__class__.__name__ == 'rectangular_magnet':
        magnet.rotation_pickup = 180
        ang_azAngs = convert_radians_to_degrees(magnet.rectangular_magnet_input_orientation) + 270
        if ang_azAngs < 180 and azAngs < 90:
            magnet.rotation_putdown = (90 - azAngs) + (180 - ang_azAngs)
            if 'I' in available_pickup:
                magnet.rotation_putdown = magnet.rotation_putdown + 180
        elif ang_azAngs < 180 and azAngs >= 90:
            magnet.rotation_putdown = (450 - azAngs) - (180 + ang_azAngs)
            if 'I' in available_pickup:
                magnet.rotation_putdown = magnet.rotation_putdown + 180
        elif ang_azAngs > 180 and azAngs < 90:
            magnet.rotation_putdown = (90 - azAngs) - (ang_azAngs - 180)
            if 'I' in available_pickup:
                magnet.rotation_putdown = magnet.rotation_putdown - 180
        elif ang_azAngs > 180 and azAngs >= 90:
            magnet.rotation_putdown = (450 - azAngs) + (180 - ang_azAngs)
            if 'I' in available_pickup:
                magnet.rotation_putdown = magnet.rotation_putdown - 180
    
    return magnet


def check_for_negative_twopi_angle(angle):
    if angle < 0:
        angle = 360 + angle
    elif angle > 360:
        angle = angle - 360
    return angle
