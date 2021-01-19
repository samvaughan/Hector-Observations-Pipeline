import numpy as np

class circle_line_intersection_equation():

    # initializing the circle line intersection equation with the respective parameters
    def __init__(self,A,B,C,line_segment):
        self.parameter_A = A
        self.parameter_B = B
        self.parameter_C = C

        self.segment = line_segment

    # calculating determinant via: b^2 - 4ac
    def calculate_determinant(self):

        self.determinant = self.parameter_B ** 2 - 4 * self.parameter_A * self.parameter_C
        return self.determinant

    # determining intersection by checking whether it's greater than or equal to zero
    def intersection(self):

        if self.calculate_determinant() >= 0:
            return True
        else:
            return False

    # solving the intersection for roots using the quadratic formula
    def solve_intersection_times(self):

        if self.intersection():
            t1 = (- self.parameter_B + np.sqrt(self.determinant)) / (2 * self.parameter_A)
            t2 = (- self.parameter_B - np.sqrt(self.determinant)) / (2 * self.parameter_A)

            return t1,t2

    # solving for intersection points by multiply roots with the vector then adding with start point
    def solve_intersecting_points(self):

        if self.intersection():
            return [[self.segment.start_point[0] + self.solve_intersection_times()[0] * self.segment.vector()[0],
                     self.segment.start_point[1] + self.solve_intersection_times()[0] * self.segment.vector()[1]],
                    [self.segment.start_point[0] + self.solve_intersection_times()[1] * self.segment.vector()[0],
                     self.segment.start_point[1] + self.solve_intersection_times()[1] * self.segment.vector()[1]]]

    # solving for all the intersection points of segment with circle
    def solve_segment_intersects_with_circle(self):

        circle_line_intersection_points = self.solve_intersecting_points()

        circle_segment_intersection_points = []

        # adding all the circle segment intersection points in list
        if circle_line_intersection_points:
            for point in circle_line_intersection_points:

                if point_is_on_segment(point, self.segment):

                    circle_segment_intersection_points.append(point)

        return circle_segment_intersection_points

# creating a circle segment equation by calculating the required parameters
def create_circle_segment_equation(circle,segment):

    # changes in x and y calculated
    dx = segment.start_point[0] - circle.center[0]
    dy = segment.start_point[1] - circle.center[1]

    parameter_A = segment.vector()[0] ** 2 + segment.vector()[1] ** 2
    parameter_B = 2 * (segment.vector()[0] * dx + segment.vector()[1] * dy)
    parameter_C = dx ** 2 + dy ** 2 - circle.radius ** 2

    return circle_line_intersection_equation(parameter_A,parameter_B,parameter_C,segment)

# checking whether point is on segment by the range of start point to end point coordinates
def point_is_on_segment(point, segment):

    if min(segment.start_point[0],segment.end_point[0]) <= point[0] <= max(segment.start_point[0],segment.end_point[0]):
        if min(segment.start_point[1],segment.end_point[1]) <= point[1] <= max(segment.start_point[1],segment.end_point[1]):
            return True
        else:
            return False
    else:
        return False

