from .constants import circular_rectangle_magnet_center_distance,HECTOR_plate_radius
from math import pi, cos, sin, atan
import numpy as np
from ..general_operations.trigonometry import convert_radians_to_degrees, convert_modulus_angle, convert_degrees_to_radians
from .magnets.circular import circular_magnet
from .magnets.rectangular import rectangular_magnet

class probe:

    # intializing a probe with all the respective parameters
    def __init__(self, probe_index, circular_magnet_center, rectangular_magnet_input_orientation, galaxyORstar, Re, mu_1re, Mstar, magnet_label, hexabundle, rads, rotation_pickup, rotation_putdown, azAngs, IDs, angs):

        self.index = probe_index
        self.circular_magnet_center = np.array(circular_magnet_center) #* HECTOR_plate_radius
        self.rectangular_magnet_input_orientation = rectangular_magnet_input_orientation
        self.circular_rectangle_magnet_center_distance = circular_rectangle_magnet_center_distance
        self.galaxyORstar = galaxyORstar
        self.Re = Re
        self.mu_1re = mu_1re
        self.Mstar = Mstar
        self.magnet_label = magnet_label
        self.hexabundle = hexabundle
        self.rads = rads
        self.rotation_pickup = rotation_pickup
        self.rotation_putdown = rotation_putdown
        self.azAngs = azAngs
        self.IDs = IDs
        self.angs = angs


        self.rectangular_magnet_absolute_orientation_degree = self.calculate_rectangular_magnet_orientation()

    # calculating circular magnet orientation based on magnet center, categorizing them in four quadrants
    def calculate_circular_magnet_orientation(self):

        # Do some basic checking for magnets at 0.0
        if self.circular_magnet_center[0] == 0:
            if self.circular_magnet_center[1] == 0:
                self.circular_magnet_orientation = 0.0
            elif self.circular_magnet_center[1] > 0:
                self.circular_magnet_orientation = np.radians(90)
            else:
                self.circular_magnet_orientation = np.radians(270)
        else:
            self.circular_magnet_orientation = np.arctan2(self.circular_magnet_center[1], self.circular_magnet_center[0])
        # converting orientation form from radians to degrees
        self.circular_magnet_orientation = np.degrees(self.circular_magnet_orientation)

        return self.circular_magnet_orientation

    def calculate_rectangular_magnet_orientation_for_plots(self):
        """
        The old calculation of the rectangular magnet orientation seems to work well with the labels on the plots and the correct ones below don't. Probably a matplotlib rectangle orientation thing? I don't get the difference as the old ones are always factors of 360 from the new ones...
        Until it's debugged, just use the old values
        """
        probe.calculate_circular_magnet_orientation(self)
        old_orientation = (90 - self.circular_magnet_orientation - np.degrees(self.rectangular_magnet_input_orientation))# % 360

        return old_orientation

    # calculating the rectangular magnet orientation with respect to the circular magnet
    def calculate_rectangular_magnet_orientation(self):

       probe.calculate_circular_magnet_orientation(self)
       old_orientation = (90 - self.circular_magnet_orientation - np.degrees(self.rectangular_magnet_input_orientation))# % 360
       #a = 270 - np.degrees(self.angs)
       

       rectangular_magnet_absolute_orientation_degree = 270 - np.degrees(self.angs)
       # print(f"Rmag_orientation calculated = {b:.7f}, angs={a:.7f}")
       #import ipdb; ipdb.set_trace()

       #print(f"{self.hexabundle}: old={old_orientation:.3f}, new={rectangular_magnet_absolute_orientation_degree:.3f}")

       return rectangular_magnet_absolute_orientation_degree

    # calculating the rectangular magnet center coordinates from its orientation, and categorizing in four quadrants
    def calculate_rectangular_magnet_center_coordinates(self):


        self.rectangular_magnet_orientation_modulo_radians = \
        convert_modulus_angle(convert_degrees_to_radians(90 - self.rectangular_magnet_absolute_orientation_degree))

        if 0 <= self.rectangular_magnet_orientation_modulo_radians <= pi / 2:

            self.rectangular_magnet_center = \
            [self.circular_magnet_center[0] \
             + self.circular_rectangle_magnet_center_distance * cos(self.rectangular_magnet_orientation_modulo_radians),
             self.circular_magnet_center[1] \
             + self.circular_rectangle_magnet_center_distance * sin(self.rectangular_magnet_orientation_modulo_radians)]

        elif pi / 2 < self.rectangular_magnet_orientation_modulo_radians <= pi:

            self.rectangular_magnet_center = \
            [self.circular_magnet_center[0] \
             - self.circular_rectangle_magnet_center_distance * cos(pi - self.rectangular_magnet_orientation_modulo_radians),
             self.circular_magnet_center[1] \
             + self.circular_rectangle_magnet_center_distance * sin(pi - self.rectangular_magnet_orientation_modulo_radians)]

        elif pi < self.rectangular_magnet_orientation_modulo_radians <= 3 * pi / 2:

            self.rectangular_magnet_center = \
            [self.circular_magnet_center[0] \
             - self.circular_rectangle_magnet_center_distance * sin(3 * pi / 2 - self.rectangular_magnet_orientation_modulo_radians),
             self.circular_magnet_center[1] \
             - self.circular_rectangle_magnet_center_distance * cos(3 * pi / 2 - self.rectangular_magnet_orientation_modulo_radians)]

        elif 3 * pi / 2 < self.rectangular_magnet_orientation_modulo_radians <= 2 * pi:

            self.rectangular_magnet_center = \
            [self.circular_magnet_center[0] \
             + self.circular_rectangle_magnet_center_distance * cos(2 * pi - self.rectangular_magnet_orientation_modulo_radians),
             self.circular_magnet_center[1] \
             - self.circular_rectangle_magnet_center_distance * sin(2 * pi - self.rectangular_magnet_orientation_modulo_radians)]
        else:
            raise ValueError(f"This rectangular magnet has an orientation of {self.rectangular_magnet_orientation_modulo_radians} and I can't calculate its centre")

        
        # # Print the distance between the circular magnet and the rectangular magnet
        # radial_distance = np.sqrt((self.rectangular_magnet_center[0] - self.circular_magnet_center[0])**2 + (self.rectangular_magnet_center[1] - self.circular_magnet_center[1])**2)
        # print(f"Radial dist between cm and rm: {radial_distance}")

        return np.array(self.rectangular_magnet_center)

    def extract_circular_magnet_parameters(self):

        c_magnet = circular_magnet(self.circular_magnet_center, probe.calculate_circular_magnet_orientation(self), self.index, self.galaxyORstar, self.Re,
                    self.mu_1re, self.Mstar, self.magnet_label, self.hexabundle, self.rads, self.rotation_pickup, self.rotation_putdown, self.azAngs, self.rectangular_magnet_input_orientation, self.IDs)
        # if c_magnet.hexabundle == "A":
        #     import ipdb; ipdb.set_trace()
        return c_magnet

    def extract_rectangular_magnet_parameters(self):
        #import ipdb; ipdb.set_trace()

        r_magnet = rectangular_magnet(center=self.calculate_rectangular_magnet_center_coordinates(),
                                  orientation=self.calculate_rectangular_magnet_orientation(),
                                  index=self.index, 
                                  galaxyORstar=self.galaxyORstar,
                                  Re=self.Re,
                                  mu_1re=self.mu_1re,
                                  Mstar=self.Mstar,
                                  magnet_label=self.magnet_label,
                                  hexabundle=self.hexabundle,
                                  rads=self.rads,
                                  rotation_pickup=self.rotation_pickup,
                                  rotation_putdown=self.rotation_putdown,
                                  azAngs=self.azAngs,
                                  rectangular_magnet_input_orientation=self.rectangular_magnet_input_orientation,
                                  IDs=self.IDs,
                                  angs=self.angs,
                                  plot_orientation=self.calculate_rectangular_magnet_orientation_for_plots())

        # if r_magnet.hexabundle == "A":
        #     import ipdb; ipdb.set_trace()
        return r_magnet
