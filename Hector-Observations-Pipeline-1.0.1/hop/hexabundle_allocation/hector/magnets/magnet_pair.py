from ...hector.magnets.circular import is_circular_magnet
from ...hector.magnets.rectangular import is_rectangular_magnet

# check for the pair of magnets if both are circular
def circle_and_circle_magnets(pair):

    return is_circular_magnet(pair[0]) and is_circular_magnet(pair[1])

# check for the pair of magnets if both are rectangular
def circle_rectangle_magnets(pair):

    if is_circular_magnet(pair[0]) and is_rectangular_magnet(pair[1]):
        return True

    elif is_rectangular_magnet(pair[0]) and is_circular_magnet(pair[1]):
        return True

    else:
        return False

# check for the pair of magnets if one is circular and the other rectangular
def rectangle_rectangle_magnets(pair):

    return is_rectangular_magnet(pair[0]) and is_rectangular_magnet(pair[1])