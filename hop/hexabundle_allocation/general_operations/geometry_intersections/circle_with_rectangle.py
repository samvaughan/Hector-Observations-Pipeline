from ...general_operations.geometry_shapes.segment import segment
from ...general_operations.geometry_intersections.circle_with_segment import create_circle_segment_equation

def circle_rectangle_intersection(circle,rectangle):

    rectangle_corners = rectangle.calculate_4corners()

    all_intersecting_points = []

    for k, l in zip([0, 1, 2, 3], [1, 2, 3, 0]):

            rectangle_side = segment(rectangle_corners[k], rectangle_corners[l])

            equation = create_circle_segment_equation(circle, rectangle_side)
            points = equation.solve_segment_intersects_with_circle()

            if points:
                all_intersecting_points.extend(points)

    return all_intersecting_points
