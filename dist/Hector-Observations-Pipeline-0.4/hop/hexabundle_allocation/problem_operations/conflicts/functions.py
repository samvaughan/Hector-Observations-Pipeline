
from ...hector.magnets.magnet_pair import circle_and_circle_magnets, circle_rectangle_magnets, rectangle_rectangle_magnets
from .circular_magnet_with_circular_magnet import check_conflict_circle_circle_magnets
from .circular_magnet_with_rectangular_magnet import check_conflict_circle_rectangle_magnets
from .rectangular_magnet_and_rectangular_magnet import check_conflict_rectangle_rectangle_magnets
from .blocked_magnet import circular_magnet_is_fully_blocked,rectangular_magnet_is_fully_blocked
from ...hector.constants import rectangle_magnet_length, robot_arm_width
import numpy as np
from ...hector.magnets.rectangular import rectangular_magnet, is_rectangular_magnet
from ...hector.magnets.circular import circular_magnet, is_circular_magnet

## The function names with no comments have self-defining names

def minimum_magnet_proximity():
    return rectangle_magnet_length + robot_arm_width

def calculate_magnet_to_magnet_distance(magnet1, magnet2):
    return np.sqrt((magnet2.center[0] - magnet1.center[0]) ** 2 + (magnet2.center[1] - magnet1.center[1]) ** 2)

def create_list_magnets_in_close_proximity(list_magnets):

    magnets_in_close_proximity = []

    # checking each magnet with all the other ones iteratively through the magnets list
    for i in range(len(list_magnets)):
        magnet_i = list_magnets[i]

        for j in range(i+1, len(list_magnets)):
            magnet_j = list_magnets[j]

            # adding to list if distance between selected magnets are less than the specified minimum proximity
            if calculate_magnet_to_magnet_distance(magnet_i, magnet_j) < minimum_magnet_proximity():
                magnets_in_close_proximity.append([magnet_i, magnet_j])

    return np.array(magnets_in_close_proximity)

def highlight_closed_magnets_on_plot(closed_magnets, colour):

    # iterating through magnet pairs identified as closed magnets
    for magnet_pair in closed_magnets:

        # iterating through each magnet of the selected pair
        for magnet in magnet_pair:

            # highlighting the respective rectangular magnet with provided input colour
            if isinstance(magnet, rectangular_magnet):
                magnet.draw_rectangle(colour)

            # highlighting the respective circular magnet with provided input colour
            elif isinstance(magnet, circular_magnet):
                magnet.draw_circle(colour)

def find_conflicts_between_magnets(pair):

    all_blocked_pickup_areas = []

    # checking if the magnet pair consists of both circular ones
    if circle_and_circle_magnets(pair):

        # check for conflict between circular magnets
        magnet_conflict = check_conflict_circle_circle_magnets(pair)

        # record the identified conflicts as blocked pickup areas
        if magnet_conflict:
            all_blocked_pickup_areas.extend(magnet_conflict)

    # checking if the magnet pair consists of one circular and one rectangular
    elif circle_rectangle_magnets(pair):

        # check for conflict between circular and rectangular magnets
        magnet_conflict = check_conflict_circle_rectangle_magnets(pair)

        # record the identified conflicts as blocked pickup areas
        if magnet_conflict:
            all_blocked_pickup_areas.extend(magnet_conflict)

    # checking if the magnet pair consists of both rectangular ones
    elif rectangle_rectangle_magnets(pair):

        # check for conflict between rectangular magnets
        magnet_conflict = check_conflict_rectangle_rectangle_magnets(pair)

        # record the identified conflicts as blocked pickup areas
        if magnet_conflict:
            all_blocked_pickup_areas.extend(magnet_conflict)

    return all_blocked_pickup_areas

def find_all_blocked_magnets(all_magnets):

    # create magnet pairs from magnets list which have specified close proximity
    magnet_pairs = create_list_magnets_in_close_proximity(all_magnets)

    blocked_areas = []

    for pair in magnet_pairs:

        # check for conflict between the magnet pairs in specified close proximity
        magnet_conflict = find_conflicts_between_magnets(pair)

        # check if magnet conflicts exist
        if len(magnet_conflict) > 0:

            # add the magnet conflicted area to the blocked areas
            blocked_areas.extend(magnet_conflict)

            # remove the blocked pickup areas from list of available pickup by iterating through conflicts
            for conflict in magnet_conflict:
                conflict.remove_blocked_pickup_areas_from_list_available_pickup()

    return blocked_areas

def is_magnet_fully_blocked(magnet,blocked_pickup_areas):

    # check for circular magnet being fully blocked
    if is_circular_magnet(magnet):
        magnet_is_fully_blocked = circular_magnet_is_fully_blocked(blocked_pickup_areas)

    # check for rectangular magnet being fully blocked
    elif is_rectangular_magnet(magnet):
        magnet_is_fully_blocked = rectangular_magnet_is_fully_blocked(blocked_pickup_areas)

    return magnet_is_fully_blocked

def create_list_of_blocked_pickup_areas(magnet, conflicted_magnet_list):

    blocked_pickup_areas = []

    for conflict in conflicted_magnet_list:

        # identify and add the blocked pickup areas from the conflicts by checking against the blocked magnets
        if magnet == conflict.blocked_magnet:
            blocked_pickup_areas.append(conflict.blocked_pickup_area)

    return blocked_pickup_areas

def remove_multiple_occurrences_in_list(object_list):
    return list(set(object_list))

def create_list_of_fully_blocked_magnets(list_of_blocked_magnets):

    fully_blocked_magnets = []

    for conflict in list_of_blocked_magnets:

        # creating list of blocked pickup areas by checking each blocked magnet against blocked magnets list
        blocked_pickup_areas = create_list_of_blocked_pickup_areas(conflict.blocked_magnet, list_of_blocked_magnets)

        # identify and add fully blocked magnets by checking them against blocked pickup areas
        if is_magnet_fully_blocked(conflict.blocked_magnet,blocked_pickup_areas):
            fully_blocked_magnets.append(conflict.blocked_magnet)

    return remove_multiple_occurrences_in_list(fully_blocked_magnets)

# identifying the magnets which are responsible for blocking each of the fully blocked magnets
def blocking_magnets_for_fully_blocked_magnets(list_of_blocked_magnets):

    fully_blocked_magnets_dictionary = {}

    for conflict in list_of_blocked_magnets:

        blocked_pickup_areas = create_list_of_blocked_pickup_areas(conflict.blocked_magnet, list_of_blocked_magnets)

        if is_magnet_fully_blocked(conflict.blocked_magnet, blocked_pickup_areas):

            # magnet conflicts stated out with each fully blocked magnet and its list of blocking magnets
            conflict_magnet = 'For ' + conflict.blocked_magnet.__class__.__name__ + ' ' + str(int(conflict.blocked_magnet.index)) + ': ' + \
                            conflict.blocking_magnet.__class__.__name__ + ' ' + str(int(conflict.blocking_magnet.index)) + '\n'

            ## TEST PRINT
            print(conflict_magnet)

            # blocked magnet stored as key and the blocking magnets as values in the fully blocked magnets dictionary
            key = conflict.blocked_magnet.__class__.__name__ + ' ' + str(int(conflict.blocked_magnet.index))
            value = conflict.blocking_magnet.__class__.__name__ + ' ' + str(int(conflict.blocking_magnet.index))

            # store the values by creating the keys if necessary and adding multiple values to same key if required
            fully_blocked_magnets_dictionary.setdefault(key, [])
            if value not in fully_blocked_magnets_dictionary[key]:
                fully_blocked_magnets_dictionary[key].append(value)

    ## TEST PRINT
    print(fully_blocked_magnets_dictionary)

    return fully_blocked_magnets_dictionary

def create_list_of_blocking_magnets(list_of_conflicts, blocked_magnet):

    blocking_magnets = []

    for conflict in list_of_conflicts:

        # check for the blocked magnet in list of conflicts and add the respective blocking magnets
        if conflict.blocked_magnet == blocked_magnet:
            blocking_magnets.append(conflict.blocking_magnet)

    return remove_multiple_occurrences_in_list(blocking_magnets)

def blocking_magnet_is_fully_blocked(blocking_magnet, list_of_fully_blocked_magnets):
    return blocking_magnet in list_of_fully_blocked_magnets

def all_blocking_magnets_are_fully_blocked(list_of_blocking_magnets,list_of_fully_blocked_magnets):

    all_blocking_magnets_are_fully_blocked = True

    # check against each blocking magnet whether all the blocking magnets are fully blocked
    for blocking_magnet in list_of_blocking_magnets:
        all_blocking_magnets_are_fully_blocked *= blocking_magnet_is_fully_blocked(blocking_magnet, list_of_fully_blocked_magnets)

    return all_blocking_magnets_are_fully_blocked
