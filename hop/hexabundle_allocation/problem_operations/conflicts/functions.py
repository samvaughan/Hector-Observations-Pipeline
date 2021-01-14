
from ...hector.magnets.magnet_pair import circle_and_circle_magnets, circle_rectangle_magnets, rectangle_rectangle_magnets
from .circular_magnet_with_circular_magnet import check_conflict_circle_circle_magnets
from .circular_magnet_with_rectangular_magnet import check_conflict_circle_rectangle_magnets
from .rectangular_magnet_and_rectangular_magnet import check_conflict_rectangle_rectangle_magnets
from .blocked_magnet import circular_magnet_is_fully_blocked,rectangular_magnet_is_fully_blocked
from ...hector.constants import rectangle_magnet_length, robot_arm_width
import numpy as np
from ...hector.magnets.rectangular import rectangular_magnet, is_rectangular_magnet
from ...hector.magnets.circular import circular_magnet, is_circular_magnet

def minimum_magnet_proximity():
    return rectangle_magnet_length + robot_arm_width

def calculate_magnet_to_magnet_distance(magnet1, magnet2):
    return np.sqrt((magnet2.center[0] - magnet1.center[0]) ** 2 + (magnet2.center[1] - magnet1.center[1]) ** 2)

def create_list_magnets_in_close_proximity(list_magnets):

    magnets_in_close_proximity = []

    for i in range(len(list_magnets)):
        magnet_i = list_magnets[i]

        for j in range(i+1, len(list_magnets)):
            magnet_j = list_magnets[j]

            if calculate_magnet_to_magnet_distance(magnet_i, magnet_j) < minimum_magnet_proximity():
                magnets_in_close_proximity.append([magnet_i, magnet_j])

    return np.array(magnets_in_close_proximity)

def highlight_closed_magnets_on_plot(closed_magnets, colour):

    for magnet_pair in closed_magnets:

        for magnet in magnet_pair:

            if isinstance(magnet, rectangular_magnet):
                magnet.draw_rectangle(colour)

            elif isinstance(magnet, circular_magnet):
                magnet.draw_circle(colour)

def find_conflicts_between_magnets(pair):

    all_blocked_pickup_areas = []

    if circle_and_circle_magnets(pair):

        magnet_conflict = check_conflict_circle_circle_magnets(pair)

        if magnet_conflict:

            all_blocked_pickup_areas.extend(magnet_conflict)

    elif circle_rectangle_magnets(pair):

        magnet_conflict = check_conflict_circle_rectangle_magnets(pair)

        if magnet_conflict:
            all_blocked_pickup_areas.extend(magnet_conflict)

    elif rectangle_rectangle_magnets(pair):

        magnet_conflict = check_conflict_rectangle_rectangle_magnets(pair)

        if magnet_conflict:

            all_blocked_pickup_areas.extend(magnet_conflict)

    return all_blocked_pickup_areas

def find_all_blocked_magnets(all_magnets):

    magnet_pairs = create_list_magnets_in_close_proximity(all_magnets)

    blocked_areas = []

    for pair in magnet_pairs:

        magnet_conflict = find_conflicts_between_magnets(pair)

        if len(magnet_conflict) > 0:
            blocked_areas.extend(magnet_conflict)

            for conflict in magnet_conflict:
                conflict.remove_blocked_pickup_areas_from_list_available_pickup()

    return blocked_areas

def is_magnet_fully_blocked(magnet,blocked_pickup_areas):

    if is_circular_magnet(magnet):
        magnet_is_fuly_blocked = circular_magnet_is_fully_blocked(blocked_pickup_areas)

    elif is_rectangular_magnet(magnet):
        magnet_is_fuly_blocked = rectangular_magnet_is_fully_blocked(blocked_pickup_areas)

    return magnet_is_fuly_blocked

def create_list_of_blocked_pickup_areas(magnet, conflicted_magnet_list):

    blocked_pickup_areas = []

    for conflict in conflicted_magnet_list:

        if magnet == conflict.blocked_magnet:
            blocked_pickup_areas.append(conflict.blocked_pickup_area)

    return blocked_pickup_areas

def remove_multiple_occurrences_in_list(object_list):
    return list(set(object_list))

def create_list_of_fully_blocked_magnets(list_of_blocked_magnets):

    fully_blocked_magnets = []

    for conflict in list_of_blocked_magnets:

        blocked_pickup_areas = create_list_of_blocked_pickup_areas(conflict.blocked_magnet, list_of_blocked_magnets)

        if is_magnet_fully_blocked(conflict.blocked_magnet,blocked_pickup_areas):

            fully_blocked_magnets.append(conflict.blocked_magnet)

    return remove_multiple_occurrences_in_list(fully_blocked_magnets)

def blocking_magnets_for_fully_blocked_magnets(list_of_blocked_magnets):

    fully_blocked_magnets_dictionary = {}

    for conflict in list_of_blocked_magnets:

        blocked_pickup_areas = create_list_of_blocked_pickup_areas(conflict.blocked_magnet, list_of_blocked_magnets)

        if is_magnet_fully_blocked(conflict.blocked_magnet, blocked_pickup_areas):
            conflict_magnet = 'For ' + conflict.blocked_magnet.__class__.__name__ + ' ' + str(int(conflict.blocked_magnet.index)) + ': ' + \
                              conflict.blocking_magnet.__class__.__name__ + ' ' + str(int(conflict.blocking_magnet.index)) + '\n'
            print(conflict_magnet)

            key = conflict.blocked_magnet.__class__.__name__ + ' ' + str(int(conflict.blocked_magnet.index))
            value = conflict.blocking_magnet.__class__.__name__[0].upper() + str('%02d' % int(conflict.blocking_magnet.index))

            fully_blocked_magnets_dictionary.setdefault(key, [])
            if value not in fully_blocked_magnets_dictionary[key]:
                if fully_blocked_magnets_dictionary[key] == []:
                    fully_blocked_magnets_dictionary[key] = value
                else:
                    fully_blocked_magnets_dictionary[key] += ',' + value

            # fully_blocked_magnets_dictionary[conflict.blocked_magnet.__class__.__name__ +' '+ str(int(conflict.blocked_magnet.index))] = \
            #                 conflict.blocking_magnet.__class__.__name__ + ' ' + str(int(conflict.blocking_magnet.index))

    print(fully_blocked_magnets_dictionary)
    return fully_blocked_magnets_dictionary

def create_list_of_blocking_magnets(list_of_conflicts, blocked_magnet):

    blocking_magnets = []

    for conflict in list_of_conflicts:

        if conflict.blocked_magnet == blocked_magnet:
            blocking_magnets.append(conflict.blocking_magnet)

    return remove_multiple_occurrences_in_list(blocking_magnets)

def blocking_magnet_is_fully_blocked(blocking_magnet, list_of_fully_blocked_magnets):
    return blocking_magnet in list_of_fully_blocked_magnets

def all_blocking_magnets_are_fully_blocked(list_of_blocking_magnets,list_of_fully_blocked_magnets):

    all_blocking_magnets_are_fully_blocked = True

    for blocking_magnet in list_of_blocking_magnets:
        all_blocking_magnets_are_fully_blocked *= blocking_magnet_is_fully_blocked(blocking_magnet, list_of_fully_blocked_magnets)

    return all_blocking_magnets_are_fully_blocked
