def intersection_slope_segment_and_vertical_segment(vertical_segment, slope_segment):

    line = slope_segment.create_line_equation()
    intersection_point_y = line.calculate_y_from_x(vertical_segment.start_point[0])
    intersection_point = [vertical_segment.start_point[0],intersection_point_y]

    if vertical_segment.point_is_on_segment(intersection_point):

        if slope_segment.point_is_on_segment(intersection_point):

            return intersection_point

        else:

            return None
    else:

        return None

def segments_intersection(segment1,segment2):

    intersection_point =[]

    if segment1.vertical():

        if segment2.vertical():

            if segment1.start_point[0] == segment2.start_point[0]:
                ### add check if point same segments

                intersection_point = [segment2.start_point[0], None]
        else:

            intersection_point = intersection_slope_segment_and_vertical_segment(segment1,segment2)

    elif segment2.vertical():

        intersection_point = intersection_slope_segment_and_vertical_segment(segment2,segment1)

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
