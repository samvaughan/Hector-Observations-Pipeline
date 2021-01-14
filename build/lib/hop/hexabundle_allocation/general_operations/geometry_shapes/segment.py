import matplotlib.pyplot as plt

class segment():

    def __init__(self,start_point,end_point):
        self.start_point = start_point
        self.end_point = end_point

    def create_line_equation(self):

        slope = (self.end_point[1] - self.start_point[1]) / (self.end_point[0] - self.start_point[0])
        origin = self.end_point[1] - slope * self.end_point[0]
        return line(slope, origin)

    def vertical(self):

        return self.start_point[0] == self.end_point[0]

    def point_is_on_segment(self,point):

        if min(self.start_point[0], self.end_point[0]) <= point[0] <= max(self.start_point[0],self.end_point[0]):
            if min(self.start_point[1], self.end_point[1]) <= point[1] <= max(self.start_point[1],self.end_point[1]):

                return True
            else:
                return False
        else:
            return False

    def vector(self):

        return [self.end_point[0] - self.start_point[0],
                self.end_point[1] - self.start_point[1]]

    def draw_segment(self,colour):

        plt.plot([self.start_point[0], self.end_point[0]], [self.start_point[1], self.end_point[1]], colour)
        plt.axis('scaled')

class line():

    def __init__(self,slope,origin):
        self.slope = slope
        self.origin = origin

    def calculate_y_from_x(self, x):
        return self.slope * x + self.origin

