from ...problem_operations.conflicts.blocked_magnet import conflicted_magnet
from ...general_operations.geometry_intersections.circle_with_rectangle import circle_rectangle_intersection
from ...general_operations.geometry_intersections.rectangle_with_rectangle import rectangle_rectangle_intersection
from ...hector.magnets.circular import is_circular_magnet
from ...hector.magnets.rectangular import is_rectangular_magnet

# check for the circular magnet blocking the rectangular magnet
def circle_blocking_rectangle(circle,rectangle):

    all_blocked_pickup_areas = []

    # for checking through each of rectangular magnet's pickup areas
    for pickup_area in rectangle.pickup_areas:

        # creating points if there is intersection between the respective pickup area of rectangular magnet with circular one
        points = circle_rectangle_intersection(circle, pickup_area)

        # if points of intersection exist, the respective pickup area of the respective magnet is blocked
        if points:

            # adding the identified blocked pickup area
            all_blocked_pickup_areas.append(conflicted_magnet(circle, rectangle, pickup_area))

            ## PLOTS TO VISUALIZE THE INTERSECTIONS FOR CLOSE INSPECTION
            # plt.figure()
            # circle.draw_circle('r')
            # plt.text(circle.center[0], circle.center[1],str(int(circle.index)), fontsize=6)
            #
            # rectangle.draw_rectangle('c')
            # plt.text(rectangle.center[0], rectangle.center[1],str(int(rectangle.index)), fontsize=6)
            #
            # pickup_area.draw_rectangle('r')

            # for point in points:
            #    plt.plot(point[0],point[1],'or')

    return all_blocked_pickup_areas

# check for the rectangular magnet blocking the circular magnet
def rectangle_blocking_circle(circle,rectangle):

    all_blocked_pickup_areas = []

    # for checking through each of circular magnet's pickup areas
    for pickup_area in circle.pickup_areas:

        # creating points if there is intersection between the respective pickup area of circular magnet with rectangular one
        points = rectangle_rectangle_intersection(rectangle, pickup_area)

        # if points of intersection exist, the respective pickup area of the respective magnet is blocked
        if points:

            # adding the identified blocked pickup area
            all_blocked_pickup_areas.append(conflicted_magnet(rectangle, circle, pickup_area))

            ## PLOTS TO VISUALIZE THE INTERSECTIONS FOR CLOSE INSPECTION
            # plt.figure()
            # circle.draw_circle('r')
            # plt.text(circle.center[0], circle.center[1],str(int(circle.index)), fontsize=6)
            #
            # rectangle.draw_rectangle('c')
            # plt.text(rectangle.center[0], rectangle.center[1],str(int(rectangle.index)), fontsize=6)
            #
            # pickup_area.draw_rectangle('r')

            # for point in points:
            #   plt.plot(point[0],point[1],'or')

    return all_blocked_pickup_areas

# check for conflicts between circular and rectangular magnets by checking each of the pickup
# direction of one magnet with the other
def check_conflict_circle_rectangle_magnets(magnet_pair):

    all_blocked_pickup_areas = []

    # for checking the magnets against each other
    for magnet_1, magnet_2 in zip([0, 1], [1, 0]):

        # for checking through each of the circular magnet's pickup areas
        if is_circular_magnet(magnet_pair[magnet_1]):

            blocked_pickup_area = circle_blocking_rectangle(magnet_pair[magnet_1], magnet_pair[magnet_2])

            # adding the identified blocked pickup area, if any
            if blocked_pickup_area:

                all_blocked_pickup_areas.extend(blocked_pickup_area)

        # for checking through each of the rectangular magnet's pickup areas
        elif is_rectangular_magnet(magnet_pair[magnet_1]):

            blocked_pickup_area = rectangle_blocking_circle(magnet_pair[magnet_2], magnet_pair[magnet_1])

            # adding the identified blocked pickup area, if any
            if blocked_pickup_area:

                all_blocked_pickup_areas.extend(blocked_pickup_area)

    return all_blocked_pickup_areas
