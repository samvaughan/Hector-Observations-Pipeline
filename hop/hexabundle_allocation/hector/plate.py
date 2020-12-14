import matplotlib.pyplot as plt
from ..general_operations.geometry_shapes.circle import circle
from ..hector.constants import HECTOR_plate_center_coordinate,HECTOR_plate_radius,robot_center_x,robot_center_y

class HECTOR_plate(circle):

    def __init__(self):
        self.center = HECTOR_plate_center_coordinate
        self.radius = HECTOR_plate_radius
        self.orientation = 0

def plot_plate(HECTOR_plate):

    # # setting aspect ratio
    # plt.figure()
    # plt.plot(np.sin(np.linspace(0, 20, 100)))
    # plt.figure().canvas.layout.width = '800px'
    # plt.figure().canvas.draw()

    plt.plot(HECTOR_plate[0], HECTOR_plate[1], 'r')
    plt.plot(HECTOR_plate[0], HECTOR_plate[2], 'r')
    #plt.plot(0, 0, 'x')

    plt.axis('scaled')
    # plt.set_axis(xlim=(-350, 350), ylim=(-350, 350))
    # plt.gca().set_xlim([-400, 400])
    # plt.gca().set_ylim([-400, 400])


    plt.xlabel('x (mm)')
    plt.ylabel('y (mm)')
    #plt.show()

###############################################################
    #2nd figure
    plt.figure()

    plt.plot(HECTOR_plate[0], HECTOR_plate[1], 'r')
    plt.plot(HECTOR_plate[0], HECTOR_plate[2], 'r')
    # plt.plot(0, 0, 'x')

    plt.axis('scaled')
    plt.axis([robot_center_x - 260, robot_center_x + 260, robot_center_y + 260, robot_center_y - 260])
    plt.xlabel('x (mm)')
    plt.ylabel('y (mm)')
    #plt.show()
###############################################################
