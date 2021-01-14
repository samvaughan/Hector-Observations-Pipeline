from ...general_operations.geometry_intersections.rectangle_with_rectangle import rectangle_rectangle_intersection
from ...problem_operations.conflicts.blocked_magnet import conflicted_magnet

def check_conflict_rectangle_rectangle_magnets(magnet_pair):

    all_blocked_pickup_areas = []

    for magnet_1, magnet_2 in zip([0, 1], [1, 0]):

        for pickup_area in magnet_pair[magnet_1].pickup_areas:

            points = rectangle_rectangle_intersection(magnet_pair[magnet_2], pickup_area)

            if points:

                # plt.figure()
                # magnet_pair[magnet_1].draw_rectangle('c')
                # plt.text(magnet_pair[magnet_1].center[0], magnet_pair[magnet_1].center[1], str(int(magnet_pair[magnet_1].index)), fontsize=6)
                #
                # magnet_pair[magnet_2].draw_rectangle('r')
                # plt.text(magnet_pair[magnet_2].center[0], magnet_pair[magnet_2].center[1], str(int(magnet_pair[magnet_2].index)), fontsize=6)
                #
                # pickup_area.draw_rectangle('r')

                all_blocked_pickup_areas.append(conflicted_magnet(magnet_pair[magnet_2], magnet_pair[magnet_1], pickup_area))

                # for point in points:
                #    plt.plot(point[0],point[1],'or')

    return all_blocked_pickup_areas



