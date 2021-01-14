import sys

from ...hector.magnets.pickup_areas import tangential_right, tangential_left, radial_inward,radial_outward,inward,outward

class conflicted_magnet:

    def __init__(self,blocking_magnet,blocked_magnet,blocked_pickup_area):

        self.blocking_magnet     = blocking_magnet
        self.blocked_magnet      = blocked_magnet
        self.blocked_pickup_area = blocked_pickup_area

    def remove_blocked_pickup_areas_from_list_available_pickup(self):

        if self.blocked_pickup_area in self.blocked_magnet.pickup_areas:
            self.blocked_magnet.pickup_areas.remove(self.blocked_pickup_area)

def circular_magnet_is_fully_blocked(list_of_blocked_pickup_areas):

    [blocked_tangential_right,\
     blocked_tangential_left,\
     blocked_radial_inward,\
     blocked_radial_outward] = [False, False, False, False]

    for blocked_pickup_area in list_of_blocked_pickup_areas:

        if isinstance(blocked_pickup_area,tangential_right):
            blocked_tangential_right = True

        if isinstance(blocked_pickup_area,tangential_left):
            blocked_tangential_left = True

        if isinstance(blocked_pickup_area,radial_inward):
            blocked_radial_inward = True

        if isinstance(blocked_pickup_area,radial_outward):
            blocked_radial_outward = True

    if blocked_tangential_right and blocked_tangential_left and blocked_radial_inward and blocked_radial_outward:

        return True

    else:

        return False

def rectangular_magnet_is_fully_blocked(list_of_blocked_pickup_areas):

    [blocked_inward, blocked_outward] = [False, False]

    for pickup_area in list_of_blocked_pickup_areas:

        if isinstance(pickup_area,inward):
            blocked_inward = True

        if isinstance(pickup_area,outward):
            blocked_outward = True

    if blocked_inward and blocked_outward:

        return True

    else:

        return False

def print_fully_blocked_magnets(fully_blocked_magnets, conflictsRecord, fileNameHexa):

    print('Fully blocked magnets:')

    for magnet in fully_blocked_magnets:
        print(magnet.__class__.__name__,int(magnet.index))
        #sys.exit('CONFLICTING MAGNETS FOUND!')

        with open(conflictsRecord,'a') as fp:
            conflict = magnet.__class__.__name__ + ' ' + str(int(magnet.index)) + ' ' + fileNameHexa + '\n'
            fp.write(conflict)

    if fully_blocked_magnets == []:
        print('None.')
