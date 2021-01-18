
# calculating intersection point between a vertical segment and slope segment
def intersection_slope_segment_and_vertical_segment(vertical_segment, slope_segment):

    # calculating intersection point of vertical and slope segments
    line = slope_segment.create_line_equation()
    intersection_point_y = line.calculate_y_from_x(vertical_segment.start_point[0])
    intersection_point = [vertical_segment.start_point[0],intersection_point_y]

    # checking whether intersection point lies on both vertical and slope segments
    if vertical_segment.point_is_on_segment(intersection_point):

        if slope_segment.point_is_on_segment(intersection_point):

            return intersection_point

        else:

            return None
    else:

        return None

# calculating intersection between two segments
def segments_intersection(segment1,segment2):

    intersection_point =[]

    # intersection calculation for either one vertical or both vertical segments
    if segment1.vertical():

        if segment2.vertical():

            if segment1.start_point[0] == segment2.start_point[0]:
                ### add check if point same segments

                intersection_point = [segment2.start_point[0], None]
        else:

            intersection_point = intersection_slope_segment_and_vertical_segment(segment1,segment2)

    # intersection calculation for one vertical and one slope segment
    elif segment2.vertical():

        intersection_point = intersection_slope_segment_and_vertical_segment(segment2,segment1)

    # intersection calculation for both slope segments by creating line equations
    else:
        line1 = segment1.create_line_equation()
        line2 = segment2.create_line_equation()

        if line1.slope != line2.slope:

            intersection_point_x = (line2.origin - line1.origin) / (line1.slope - line2.slope)
            intersection_point_y = line1.slope * intersection_point_x + line1.origin

            if segment1.point_is_on_segment([intersection_point_x,intersection_point_y]):
                if segment2.point_is_on_segment([intersection_point_x,intersection_point_y]):

                    intersection_point = [intersection_point_x,intersection_point_y]

    return intersection_point
