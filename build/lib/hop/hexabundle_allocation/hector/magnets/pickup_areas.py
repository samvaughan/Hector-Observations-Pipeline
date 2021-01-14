from ...hector.constants import rectangle_magnet_length, robot_arm_width,robot_arm_length,circular_magnet_radius
from ...general_operations.geometry_shapes.rectangle import rectangle

class rectangular_magnet_pickup_area(rectangle):

    def __init__(self,center,orientation):
        #length = 0.5 * (rectangle_magnet_length + 3* robot_arm_width) # Tiphaine's calculation
        if (robot_arm_width < ((rectangle_magnet_length - robot_arm_width) / 2)):
            length = (robot_arm_width + rectangle_magnet_length) / 2
        elif (robot_arm_width >= ((rectangle_magnet_length - robot_arm_width) / 2)):
            length = 2 * robot_arm_width

        width  = robot_arm_length
        super().__init__(center,length,width,orientation)


class circular_magnet_pickup_area(rectangle):

    def __init__(self,center,orientation):
        length = (3.0/2) * robot_arm_width + circular_magnet_radius
        width  = robot_arm_length
        super().__init__(center,length,width,orientation)


class inward(rectangular_magnet_pickup_area):
    kind = 'inward (toward circular magnet) pickup area'
    code = 'I'
    pass

class outward(rectangular_magnet_pickup_area):
    kind = 'outward (toward circular magnet) pickup area'
    code = 'O'
    pass

class tangential_right(circular_magnet_pickup_area):
    kind = 'tangential from right pickup area'
    code = 'TR'
    pass

class tangential_left(circular_magnet_pickup_area):
    kind = 'tangential from left pickup area'
    code = 'TL'
    pass

class radial_inward(circular_magnet_pickup_area):
    kind = 'inward radial pickup area'
    code ='RI'
    pass

class radial_outward(circular_magnet_pickup_area):
    kind = 'outward radial pickup area'
    code = 'RO'
    pass



