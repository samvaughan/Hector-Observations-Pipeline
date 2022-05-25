from ...general_operations.geometry_intersections.rectangle_with_rectangle import rectangle_rectangle_intersection
from ...problem_operations.conflicts.blocked_magnet import conflicted_magnet

# check for conflicts between two rectangular magnets by checking each of the pickup direction of one magnet with the other
def check_conflict_rectangle_rectangle_magnets(magnet_pair):

    all_blocked_pickup_areas = []

    # for checking the magnets against each other
    for magnet_1, magnet_2 in zip([0, 1], [1, 0]):

        # for checking through each of the selected magnet's pickup areas
        for pickup_area in magnet_pair[magnet_1].pickup_areas:

            # creating points if there is intersection between the respective pickup area of each magnet with the other
            points = rectangle_rectangle_intersection(magnet_pair[magnet_2], pickup_area)

            # if points of intersection exist, the respective pickup area of the respective magnet is blocked
            if points:

                # adding the identified blocked pickup area
                all_blocked_pickup_areas.append(conflicted_magnet(magnet_pair[magnet_2], magnet_pair[magnet_1], pickup_area))

                ## PLOTS TO VISUALIZE THE INTERSECTIONS FOR CLOSE INSPECTION
                # plt.figure()
                # magnet_pair[magnet_1].draw_rectangle('c')
                # plt.text(magnet_pair[magnet_1].center[0], magnet_pair[magnet_1].center[1], str(int(magnet_pair[magnet_1].index)), fontsize=6)
                #
                # magnet_pair[magnet_2].draw_rectangle('r')
                # plt.text(magnet_pair[magnet_2].center[0], magnet_pair[magnet_2].center[1], str(int(magnet_pair[magnet_2].index)), fontsize=6)
                #
                # pickup_area.draw_rectangle('r')

                # for point in points:
                #    plt.plot(point[0],point[1],'or')

    return all_blocked_pickup_areas



