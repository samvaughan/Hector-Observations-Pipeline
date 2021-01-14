from math import pi, cos, sin

def rotational_matrix(rotation_angle):
    return [[cos(rotation_angle), - sin(rotation_angle)],
            [sin(rotation_angle),   cos(rotation_angle)]]

def convert_degrees_to_radians(angle_degrees):
    return angle_degrees * pi/180

def convert_radians_to_degrees(angle_radians):
    return angle_radians * 180/pi

def convert_modulus_angle(angle):
    piX2 = 2 * pi

    if angle > piX2:
        angle = angle - piX2
    if angle < 0:
        angle = piX2 - abs(angle)

    return angle