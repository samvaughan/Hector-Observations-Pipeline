import numpy as np
import matplotlib.pyplot as plt
from ...hector.constants import robot_center_x,robot_center_y

class circle:

    def __init__(self, center,radius,orientation):

        self.center = center
        self.radius = radius
        self.orientation = orientation

        # fixing plotting view for Robot file coordinates
        self.view_y = self.center[0]
        self.view_x = - self.center[1]
        # self.view_orientation = self.orientation# - 90

    def calculate_circle_all_coordinates(self):

        self.x_coordinates = np.arange(self.center[0] - self.radius, self.center[0] + self.radius, self.radius / 100000)
        self.y_coordinates_negative = -np.sqrt(np.abs(self.radius ** 2 - (self.x_coordinates - self.center[0]) ** 2)) + self.center[1]
        self.y_coordinates_positive = np.sqrt(np.abs(self.radius ** 2 - (self.x_coordinates - self.center[0]) ** 2)) + self.center[1]


    def draw_circle(self,colour):

        circle.calculate_circle_all_coordinates(self)

        plt.figure(1)
        plt.plot(self.x_coordinates, self.y_coordinates_negative, colour, alpha = 0.5)
        plt.plot(self.x_coordinates, self.y_coordinates_positive, colour, alpha = 0.5)
        plt.axis('scaled')

################################################################
        plt.figure(2)
        plt.plot(self.x_coordinates + robot_center_x, (self.y_coordinates_negative + robot_center_y), colour)
        plt.plot(self.x_coordinates + robot_center_x, (self.y_coordinates_positive + robot_center_y), colour)
        plt.axis('scaled')
################################################################
