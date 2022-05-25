from math import pi, cos, sin
import numpy as np

# function for rotational matrix required for rotating at the specified rotation angle
def rotational_matrix(rotation_angle):
    return [[cos(rotation_angle), - sin(rotation_angle)],
            [sin(rotation_angle),   cos(rotation_angle)]]

# function  for converting degrees to radians
def convert_degrees_to_radians(angle_degrees):
    return angle_degrees * np.pi/180

# function  for converting radians to degrees
def convert_radians_to_degrees(angle_radians):
    return angle_radians * 180/np.pi

# function to keep angle in range of 0 to 2pi
def convert_modulus_angle(angle):
    # piX2 = 2 * np.pi
    # # if angle > 2 * np.pi:
    # #     raise ValueError("Angle is bigger than 2pi!")
    # if angle > piX2:
    #     angle = angle - piX2
    # if angle < 0:
    #     angle = piX2 - abs(angle)

    return angle % (2 * np.pi)