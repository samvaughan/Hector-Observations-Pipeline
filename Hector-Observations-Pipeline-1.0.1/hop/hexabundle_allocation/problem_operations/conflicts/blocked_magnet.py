import sys

from ...hector.magnets.pickup_areas import tangential_right, tangential_left, radial_inward,radial_outward,inward,outward

class conflicted_magnet:

    # initializing class of magnets with conflicts
    def __init__(self,blocking_magnet,blocked_magnet,blocked_pickup_area):

        self.blocking_magnet     = blocking_magnet
        self.blocked_magnet      = blocked_magnet
        self.blocked_pickup_area = blocked_pickup_area

    # removing the pickup areas which are blocked from the list of pickup areas
    def remove_blocked_pickup_areas_from_list_available_pickup(self):

        if self.blocked_pickup_area in self.blocked_magnet.pickup_areas:
            self.blocked_magnet.pickup_areas.remove(self.blocked_pickup_area)

# checking if circular magnet is fully blocked by its pickup directions
def circular_magnet_is_fully_blocked(list_of_blocked_pickup_areas):

    # initializing all pickup directions being blocked as false
    [blocked_tangential_right,\
     blocked_tangential_left,\
     blocked_radial_inward,\
     blocked_radial_outward] = [False, False, False, False]

    # checking for each pickup direction to see if it's blocked
    for blocked_pickup_area in list_of_blocked_pickup_areas:

        if isinstance(blocked_pickup_area,tangential_right):
            blocked_tangential_right = True

        if isinstance(blocked_pickup_area,tangential_left):
            blocked_tangential_left = True

        if isinstance(blocked_pickup_area,radial_inward):
            blocked_radial_inward = True

        if isinstance(blocked_pickup_area,radial_outward):
            blocked_radial_outward = True

    # if all pickup directions are blocked, function returns True for magnet being fully blocked or else False
    if blocked_tangential_right and blocked_tangential_left and blocked_radial_inward and blocked_radial_outward:
        return True

    else:
        return False

# checking if rectangular magnet is fully blocked by its pickup directions
def rectangular_magnet_is_fully_blocked(list_of_blocked_pickup_areas):

    # initializing all pickup directions being blocked as false
    [blocked_inward, blocked_outward] = [False, False]

    # checking for each pickup direction to see if it's blocked
    for pickup_area in list_of_blocked_pickup_areas:

        if isinstance(pickup_area,inward):
            blocked_inward = True

        if isinstance(pickup_area,outward):
            blocked_outward = True

    # if all pickup directions are blocked, function returns True for magnet being fully blocked or else False
    if blocked_inward and blocked_outward:
        return True

    else:
        return False

# print out the fully blocked magnets list on terminal, and write them in the conflicts record file
def print_fully_blocked_magnets(fully_blocked_magnets, conflictsRecord, fileNameHexa):

    # Check if fully_blocked_magnets has any elements
    if fully_blocked_magnets:
        print('\n\nSOME MAGNETS ARE FULLY BLOCKED\n\nFully blocked magnets:')

        for magnet in fully_blocked_magnets:

            # print fully blocked magnets on terminal
            print(magnet.__class__.__name__,int(magnet.index))

            # write fully blocked magnets on file
            with open(conflictsRecord,'a') as fp:
                conflict = magnet.__class__.__name__ + ' ' + str(int(magnet.index)) + ' ' + fileNameHexa.as_posix() + '\n'
                fp.write(conflict)
        raise ValueError("Some Magnets are blocked! Go back and change the configuration")

    # if none of the magnets are fully blocked then print 'None'
    else: 
        print('\nNo magnets are fully blocked!')
