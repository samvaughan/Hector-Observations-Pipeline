from ...general_operations.geometry_shapes.segment import segment
from ...general_operations.geometry_intersections.circle_with_segment import create_circle_segment_equation

# calculating intersection points between circular and rectangular magnet
def circle_rectangle_intersection(circle,rectangle):

    # calculate four corners of the rectangular magnet
    rectangle_corners = rectangle.calculate_4corners()

    all_intersecting_points = []

    for k, l in zip([0, 1, 2, 3], [1, 2, 3, 0]):

        # creating rectangle magnet sides using rectangular magnet corners
        rectangle_side = segment(rectangle_corners[k], rectangle_corners[l])

        # creating circle segment equations and solving for their intersection points
        equation = create_circle_segment_equation(circle, rectangle_side)
        points = equation.solve_segment_intersects_with_circle()

        # adding all intersecting points is list
        if points:
            all_intersecting_points.extend(points)

    return all_intersecting_points
