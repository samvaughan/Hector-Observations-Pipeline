import matplotlib.pyplot as plt

class segment():

    # initializing a segment with the respective parameters
    def __init__(self,start_point,end_point):
        self.start_point = start_point
        self.end_point = end_point

    # creating a line equation by calculating slope and origin first
    def create_line_equation(self):

        slope = (self.end_point[1] - self.start_point[1]) / (self.end_point[0] - self.start_point[0])
        origin = self.end_point[1] - slope * self.end_point[0]
        return line(slope, origin)

    # checking whether line is vertical by the x=coordinate of start and end point
    def vertical(self):

        return self.start_point[0] == self.end_point[0]

    # checking if a point lies within segment by the range of start and end points
    def point_is_on_segment(self,point):

        if min(self.start_point[0], self.end_point[0]) <= point[0] <= max(self.start_point[0],self.end_point[0]):
            if min(self.start_point[1], self.end_point[1]) <= point[1] <= max(self.start_point[1],self.end_point[1]):

                return True
            else:
                return False
        else:
            return False

    # function for difference between end and start point
    def vector(self):

        return [self.end_point[0] - self.start_point[0],
                self.end_point[1] - self.start_point[1]]

    # plotting the segment
    def draw_segment(self,colour):

        plt.plot([self.start_point[0], self.end_point[0]], [self.start_point[1], self.end_point[1]], colour)
        plt.axis('scaled')

class line():

    # initializing a line with the respective parameters
    def __init__(self,slope,origin):
        self.slope = slope
        self.origin = origin

    # calculating y from x using straight line equation: y = mx + c
    def calculate_y_from_x(self, x):
        return self.slope * x + self.origin

