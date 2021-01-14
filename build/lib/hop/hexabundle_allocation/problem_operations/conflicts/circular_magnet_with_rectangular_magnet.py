from ...problem_operations.conflicts.blocked_magnet import conflicted_magnet
from ...general_operations.geometry_intersections.circle_with_rectangle import circle_rectangle_intersection
from ...general_operations.geometry_intersections.rectangle_with_rectangle import rectangle_rectangle_intersection
from ...hector.magnets.circular import is_circular_magnet
from ...hector.magnets.rectangular import is_rectangular_magnet


def circle_blocking_rectangle(circle,rectangle):

    all_blocked_pickup_areas = []

    for pickup_area in rectangle.pickup_areas:

        points = circle_rectangle_intersection(circle, pickup_area)

        if points:

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

            all_blocked_pickup_areas.append(conflicted_magnet(circle, rectangle, pickup_area))


    return all_blocked_pickup_areas

def rectangle_blocking_circle(circle,rectangle):

    all_blocked_pickup_areas = []

    for pickup_area in circle.pickup_areas:

        points = rectangle_rectangle_intersection(rectangle, pickup_area)

        if points:

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

            all_blocked_pickup_areas.append(conflicted_magnet(rectangle, circle, pickup_area))

    return all_blocked_pickup_areas

def check_conflict_circle_rectangle_magnets(magnet_pair):

    all_blocked_pickup_areas = []

    for magnet_1, magnet_2 in zip([0, 1], [1, 0]):

        if is_circular_magnet(magnet_pair[magnet_1]):

            blocked_pickup_area = circle_blocking_rectangle(magnet_pair[magnet_1], magnet_pair[magnet_2])

            if blocked_pickup_area:

                all_blocked_pickup_areas.extend(blocked_pickup_area)


        elif is_rectangular_magnet(magnet_pair[magnet_1]):

            blocked_pickup_area = rectangle_blocking_circle(magnet_pair[magnet_2], magnet_pair[magnet_1])

            if blocked_pickup_area:

                all_blocked_pickup_areas.extend(blocked_pickup_area)

    return all_blocked_pickup_areas
