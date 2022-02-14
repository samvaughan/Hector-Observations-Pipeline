from ...general_operations.geometry_shapes.rectangle import rectangle
from ...hector.constants import rectangle_magnet_length,rectangle_magnet_width,robot_arm_width
from math import sin, cos, pi
from ...hector.magnets.pickup_areas import inward, outward
# from 

### SHOULD ALSO INHERIT FROM PROBE CLASS HERE!

class rectangular_magnet(rectangle):

    # initializing a rectangular magnet with all respective parameters
    def __init__(self,center,orientation,index,galaxyORstar,Re,mu_1re,Mstar,magnet_label,hexabundle,rads,rotation_pickup,rotation_putdown,azAngs,rectangular_magnet_input_orientation,IDs, angs, plot_orientation):

        length = rectangle_magnet_length
        width = rectangle_magnet_width
        super().__init__(center,length,width,orientation)
        self.index = index
        self.placement_index = 0
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
        self.rectangular_magnet_input_orientation = rectangular_magnet_input_orientation
        self.IDs = IDs
        self.angs = angs
        self.rectangular_magnet_absolute_orientation_degree = orientation
        self.plot_orientation = plot_orientation

    # calculating the distance between magnet center to pickup area center
    def calculate_center_magnet_to_center_pickup_area_length(self):

        # center_magnet_to_center_pickup_area_length = 0.25 * (self.length + robot_arm_width)  # Tiphaine's version

        # if (robot_arm_width < ((rectangle_magnet_length - robot_arm_width) / 2)):
        #     center_magnet_to_center_pickup_area_length = 0.25 * (rectangle_magnet_length - robot_arm_width)
        # elif (robot_arm_width >= ((rectangle_magnet_length - robot_arm_width) / 2)):
        #     center_magnet_to_center_pickup_area_length = robot_arm_width / 2

        center_magnet_to_center_pickup_area_length = 0.25 * (10 + robot_arm_width)  # generic one for all RAW sizes

        return center_magnet_to_center_pickup_area_length

    # calculating the center coordinate of pickup area- Outward
    def calculate_center_coordinate_outward_pickuparea(self):

        center_coordinates = \
        [self.center[0] + self.calculate_center_magnet_to_center_pickup_area_length() * sin((pi / 180.0) * (self.orientation)),
         self.center[1] + self.calculate_center_magnet_to_center_pickup_area_length() * cos((pi / 180.0) * (self.orientation))]

        return center_coordinates

    # calculating the center coordinate of pickup area- Inwards
    def calculate_center_coordinate_inward_pickuparea(self):

        center_coordinates = \
        [self.center[0] - self.calculate_center_magnet_to_center_pickup_area_length() * sin((pi / 180.0) * (self.orientation)),
         self.center[1] - self.calculate_center_magnet_to_center_pickup_area_length() * cos((pi / 180.0) * (self.orientation))]

        return center_coordinates

    # calculating pickup areas using their respective center coordinates
    def create_pickup_areas(self):

        self.pickup_areas = \
        [inward(self.calculate_center_coordinate_inward_pickuparea(), self.orientation),
         outward(self.calculate_center_coordinate_outward_pickuparea(),self.orientation)]

        return self.pickup_areas

# check for the magnet being rectangular type
def is_rectangular_magnet(magnet):
    return isinstance(magnet, rectangular_magnet)
