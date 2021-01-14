from ...general_operations.geometry_intersections.segment_with_segment import segments_intersection
from ...general_operations.geometry_shapes.segment import segment

def rectangle_rectangle_intersection(rectangle1, rectangle2):

    rectangle1_corners = rectangle1.calculate_4corners()
    rectangle2_corners = rectangle2.calculate_4corners()

    all_intersection_points = []

    for k,l in zip([0, 1, 2, 3],[1, 2, 3, 0]):
        for m,n in zip([0, 1, 2, 3],[1, 2, 3, 0]):

            rectangle1_side = segment(rectangle1_corners[k], rectangle1_corners[l])
            rectangle2_side = segment(rectangle2_corners[m], rectangle2_corners[n])

            point = segments_intersection(rectangle1_side,rectangle2_side)

            if point:

                all_intersection_points.append(point)

    return all_intersection_points
