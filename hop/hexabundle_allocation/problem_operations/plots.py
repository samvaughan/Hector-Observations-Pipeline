import matplotlib.pyplot as plt
import matplotlib.patches as patches
import matplotlib.cm as cm
from ..hector.magnets.circular import is_circular_magnet
from ..hector.magnets.rectangular import is_rectangular_magnet
from ..hector.constants import robot_center_x,robot_center_y
import re
import numpy as np
from math import pi, cos, sin
import pandas as pd
from ..general_operations.trigonometry import rotational_matrix,convert_degrees_to_radians, convert_radians_to_degrees


plt.rc('font', size=30)          # controls default text sizes
plt.rc('axes', titlesize=30)     # fontsize of the axes title
plt.rc('axes', labelsize=30)     # fontsize of the x and y labels
plt.rc('xtick', labelsize=30)    # fontsize of the tick labels
plt.rc('ytick', labelsize=30)    # fontsize of the tick labels
plt.rc('legend', fontsize=30)    # legend fontsize
plt.rc('figure', titlesize=30)   # fontsize of the figure title

# creating annotated text on the magnets to differentiate the type adn size of hexabundles used and for labels
def annotate_text_in_plot(magnet):
    rotation_matrix = rotational_matrix(convert_degrees_to_radians(magnet.orientation))

    if magnet.hexabundle == 'A' or magnet.hexabundle == 'B':
        corner_offset = 9
        width = 22.4
        length = 54.4
        radius = 14
        z_order = 2
    elif magnet.hexabundle == 'C':
        corner_offset = 6
        width = 19.4
        length = 51.4
        radius = 12
        z_order = 2
    elif magnet.hexabundle == 'H' or magnet.hexabundle == 'U':
        corner_offset = 2
        width = 13.4
        length = 45.4
        radius = 9
        z_order = 3
    else:
        corner_offset = 3.5
        width = 16.4
        length = 48.4
        radius = 10
        z_order = 3

    magnet.corner = (magnet.center[0] + rotation_matrix[0][0] * (magnet.width + corner_offset) / 2 + rotation_matrix[1][0] * (magnet.length + 3) / 2, \
                    magnet.center[1] + rotation_matrix[0][1] * (magnet.width + corner_offset) / 2 + rotation_matrix[1][1] * (magnet.length + 3) / 2)
    magnet.circular_center = (magnet.center[0] - rotation_matrix[1][0] * (magnet.length + 27) / 2, \
                             magnet.center[1] - rotation_matrix[1][1] * (magnet.length + 27) / 2)
    annotated_text = re.split('(\d+)', magnet.magnet_label)[1]
    annotated_text = str(magnet.hexabundle + '  ' + str(annotated_text))

    if (annotated_text[1] == 'S'):
        annotated_text = annotated_text.replace('GS', '')
        label_color = 'yellow'# change back to 'yellow'
        patch_color = 'black'
    elif ('I'> magnet.hexabundle >='A'):
        label_color = 'black'
        patch_color = 'white'
    elif (magnet.hexabundle >'H'):
        label_color = 'white'# change back to 'white'
        patch_color = 'black'

    # Increase zorder of the smaller hexabundles over A and B and C- the largest ones
    # ****** previousOne draw_fill = patches.Rectangle((magnet.corner[0], magnet.corner[1]), width, length, angle=abs(180 - magnet.orientation),facecolor=patch_color,alpha=0.8,zorder=3)#,edgecolor='black',linewidth=0.5
    draw_fill2 = patches.Rectangle((magnet.corner[0], magnet.corner[1]), width, (length-12), angle=abs(180 - magnet.orientation), facecolor=patch_color,edgecolor='black',linewidth=0.5, alpha=0.8,zorder=z_order)
    draw_fill3 = patches.Circle((magnet.circular_center[0], magnet.circular_center[1]), radius=radius, facecolor=patch_color, edgecolor='black',linewidth=0.5, alpha=0.8,zorder=z_order)
    draw_fill4 = patches.Rectangle((magnet.corner[0], magnet.corner[1]), width, (length - 11), angle=abs(180 - magnet.orientation),
                                   facecolor=patch_color, linewidth=0.5, alpha=0.9,zorder=3)
    # plt.gcf().gca().add_artist(draw_fill) ## previous one
    plt.gcf().gca().add_artist(draw_fill2)
    plt.gcf().gca().add_artist(draw_fill3)
    plt.gcf().gca().add_artist(draw_fill4)

    magnet.text_center = (magnet.center[0] - rotation_matrix[1][0] * (magnet.length ) / 2, \
                    magnet.center[1] - rotation_matrix[1][1] * (magnet.length ) / 2)

    if (magnet.hexabundle == 'H') or (magnet.hexabundle == 'U'):
        annotated_star = str('      ★')
        plt.annotate(annotated_star, (magnet.text_center[0], magnet.text_center[1]), color=label_color,rotation=abs(90 - magnet.orientation), \
                     weight='bold', fontsize=11, ha='center', va='center', zorder=3)

    plt.annotate(annotated_text, (magnet.text_center[0], magnet.text_center[1]), color=label_color, rotation=abs(90 - magnet.orientation),\
                 weight='bold', fontsize=6, ha='center', va='center',zorder=4)

def coordinates_and_angle_of_skyFibres(angle,radii):
    x = y = rotation = 0
    if angle>180:
        angle = -(360-angle)

    if 0<=angle<=90:
        x = -radii*cos(convert_degrees_to_radians(90-angle))
        y = radii*sin(convert_degrees_to_radians(90-angle))
        rotation = angle
    elif 90<angle<=180:
        x = -radii*cos(convert_degrees_to_radians(angle-90))
        y = -radii*sin(convert_degrees_to_radians(angle-90))
        rotation = angle
    elif -90<=angle<0:
        x = radii*cos(convert_degrees_to_radians(angle+90))
        y = radii*sin(convert_degrees_to_radians(angle+90))
        rotation = angle
    elif -180<=angle<-90:
        x = radii*sin(convert_degrees_to_radians(angle+180))
        y = -radii*cos(convert_degrees_to_radians(angle+180))
        rotation = angle

    return x,y,rotation

def read_sky_fibre_file(filename):

    # dataframe = pd.read_csv(filename,skiprows=lambda x: x in [0,1,2,3,4,5])
    # # print(dataframe)
    #
    # subplate_name = dataframe['ID']
    # position = dataframe['Position']
    #
    # skyfibreDict = {}
    # j = 0
    # for i in subplate_name:
    #     if i[0] == 'S':
    #
    #         if i[4] == 'A':
    #             fib_num = 7
    #         elif i[4] == 'H':
    #             fib_num = 8
    #
    #         num = [int(s) for s in i.split('-') if s.isdigit()]
    #         num = int(num[0])
    #
    #         k = 1
    #         while num > fib_num:
    #             num = num - fib_num
    #             k += 1
    #
    #         if (i[4]+str(k)) not in skyfibreDict:
    #             skyfibreDict[i[4]+str(k)] = []
    #         skyfibreDict[i[4] + str(k)].append({int(num): int(position[j])})
    #
    #     j += 1

    # print(skyfibreDict)

    # getting count of lines to skip at top of file, which contain other information
    with open(filename) as f:
        line = f.readline()
        skipline_count = 0
        while line.startswith('#'):
            line = f.readline()
            skipline_count += 1

    df_skyfibre = pd.read_csv(filename, sep = ' ', skiprows=skipline_count)

    mask = df_skyfibre['probe'] < 22
    df_skyfibre = df_skyfibre[~mask]

    skyfibreDict = {}
    subplate_info = df_skyfibre['IDs']
    position = df_skyfibre['Position']
    position.reset_index(drop=True, inplace=True)

    j = 0
    for i in subplate_info:
        # print(i[4:6])
        # print(i[7])
        # print(int(position[0]))

        # skyfibreDict[i[4:5]] = {int(i[7]): int(position[j])}
        if (str(i[4:6])) not in skyfibreDict:
            skyfibreDict[str(i[4:6])] = []
        skyfibreDict[str(i[4:6])].append({int(i[7]): int(position[j])})

        j += 1

    return skyfibreDict

def sky_fibre_annotations(skyfibre_file):
    angle_subplate = [7,5,3,1,-1,-3,-5,-7]
    radii = 270

    # filename = 'Sky_fibre_position_example.csv'
    skyfibreDict = read_sky_fibre_file(skyfibre_file)
    print(skyfibreDict)
    string = str(skyfibreDict['H3'][5].keys())
    print(re.sub('[^0-9]', '', string))

    #sky fibres top batch
    angle = 30
    skyfibreTitles_top = ['H3','A3','H4','A4']
    for i in range(0,4):
        x, y, rotation = coordinates_and_angle_of_skyFibres(angle,radii)
        plt.annotate(skyfibreTitles_top[i], (x,y), color='black',rotation=rotation, fontsize=11, ha='center', va='center')
        for j in range(len(skyfibreDict[skyfibreTitles_top[i]])):
            angle_pos = angle + angle_subplate[j]
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos,342)
            fibre_num = re.sub('[^0-9]', '', str(skyfibreDict[skyfibreTitles_top[i]][j].keys()))
            plt.annotate(fibre_num, (x, y), color='black', rotation=rotation, fontsize=5,ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 308)
            plt.annotate('▮', (x, y), color='black', rotation=rotation, fontsize=7,ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 322)
            plt.annotate(str(skyfibreDict[skyfibreTitles_top[i]][j][j+1]), (x, y), color='black', rotation=rotation, fontsize=6, weight='bold', ha='center',va='center')
        if skyfibreTitles_top[i][0] == 'H':
            alpha = 0.4
        elif skyfibreTitles_top[i][0] == 'A':
            alpha = 0.7
        draw_wedge = patches.Wedge((0, 0), r=333, theta1=angle-9+90, theta2=angle+9+90, width=90, facecolor='gray', edgecolor='black',alpha=alpha)
        plt.gcf().gca().add_artist(draw_wedge)
        angle = angle - 20

    # sky fibres left batch
    angle = 150
    skyfibreTitles_left = ['A1', 'H1', 'H2', 'A2']
    for i in range(0, 4):
        x, y, rotation = coordinates_and_angle_of_skyFibres(angle,radii)
        plt.annotate(skyfibreTitles_left[i], (x, y), color='black', rotation=rotation, fontsize=11,ha='center', va='center')
        for j in range(len(skyfibreDict[skyfibreTitles_left[i]])):
            angle_pos = angle + angle_subplate[j]
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos,342)
            fibre_num = re.sub('[^0-9]', '', str(skyfibreDict[skyfibreTitles_left[i]][j].keys()))
            plt.annotate(fibre_num, (x, y), color='black', rotation=rotation, fontsize=5, ha='center',va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 308)
            plt.annotate('▮', (x, y), color='black', rotation=rotation, fontsize=7, ha='center',va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 322)
            plt.annotate(str(skyfibreDict[skyfibreTitles_left[i]][j][j + 1]), (x, y), color='black', rotation=rotation, fontsize=6, weight='bold', ha='center', va='center')
        if skyfibreTitles_left[i][0] == 'H':
            alpha = 0.4
        elif skyfibreTitles_left[i][0] == 'A':
            alpha = 0.7
        draw_wedge = patches.Wedge((0, 0), r=333, theta1=angle-9+90, theta2=angle+9+90, width=90, facecolor='gray', edgecolor='black',alpha=alpha)
        plt.gcf().gca().add_artist(draw_wedge)
        angle = angle - 20

    # sky fibres right batch
    angle = -160
    skyfibreTitles_right = ['H7', 'A5', 'H6', 'H5']
    for i in range(0, 4):
        x, y, rotation = coordinates_and_angle_of_skyFibres(angle,radii)
        plt.annotate(skyfibreTitles_right[i], (x, y), color='black', rotation=rotation, fontsize=11, ha='center', va='center')
        for j in range(len(skyfibreDict[skyfibreTitles_right[i]])):
            angle_pos = angle + angle_subplate[j]
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 342)
            fibre_num = re.sub('[^0-9]', '', str(skyfibreDict[skyfibreTitles_right[i]][j].keys()))
            plt.annotate(fibre_num, (x, y), color='black', rotation=rotation, fontsize=5, ha='center',va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 308)
            plt.annotate('▮', (x, y), color='black', rotation=rotation, fontsize=7, ha='center',va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 322)
            plt.annotate(str(skyfibreDict[skyfibreTitles_right[i]][j][j + 1]), (x, y), color='black', rotation=rotation, fontsize=6, weight='bold', ha='center', va='center')
        if skyfibreTitles_right[i][0] == 'H':
            alpha = 0.4
        elif skyfibreTitles_right[i][0] == 'A':
            alpha = 0.7
        draw_wedge = patches.Wedge((0, 0), r=333, theta1=angle-9+90, theta2=angle+9+90, width=90, facecolor='gray', edgecolor='black',alpha=alpha)
        plt.gcf().gca().add_artist(draw_wedge)
        angle = angle + 20

def draw_circularSegments():

    draw_circle = plt.Circle((0, 0), 226, fill=True, color='#E78BE7', alpha=0.4)
    draw_circle1 = plt.Circle((0, 0), (196.05124), fill=True, color='#f6f93b')
    draw_circle2 = plt.Circle((0, 0), (147.91658), fill=True, color='#60fb3d')
    draw_circle3 = plt.Circle((0, 0), (92.71721), fill=True, color='#add8e6')

    plt.gcf().gca().add_artist(draw_circle)
    plt.gcf().gca().add_artist(draw_circle1)
    plt.gcf().gca().add_artist(draw_circle2)
    plt.gcf().gca().add_artist(draw_circle3)


def draw_all_magnets(magnets, clusterNum, tileNum, fileNameHexa, robot_figureFile, hexabundle_figureFile):
    # plt.figure(2)
    # draw_magnet_pickup_areas(magnets, '--c')

    plt.figure(1)

    draw_circularSegments()

    sky_fibre_annotations(fileNameHexa)

    # plt.figure(2)
    # draw_magnet_pickup_areas(magnets, '--c')

    for magnet in magnets:

        if is_circular_magnet(magnet):
            plt.figure(1)
            magnet.draw_circle('r') ## The magnets being drawn
###################################################
            plt.figure(2)
            for pickup_area in magnet.pickup_areas:
                pickup_area.draw_rectangle('--c',2)
            magnet.draw_circle('r')

            #plt.text(magnet.center[0], magnet.center[1], str(int(magnet.index)), fontsize=12, rotation=abs(135-magnet.orientation))
###################################################
        if is_rectangular_magnet(magnet):
            plt.figure(1)
            magnet.draw_rectangle('r',1) ## The magnets being drawn
            plt.figure(1)
            annotate_text_in_plot(magnet)
            # plt.text(magnet.center[0], magnet.center[1], str(magnet.magnet_label), fontsize=9, rotation=abs(180-magnet.orientation))  #+'\n'+str(magnet.hexabundle)

            plt.xlabel('mm')
            plt.ylabel('mm')
################################################################
            plt.figure(2)
            for pickup_area in magnet.pickup_areas:
                pickup_area.draw_rectangle('--c',2)
            magnet.draw_rectangle('r',2)
            plt.text(robot_center_x + magnet.center[0], robot_center_y + magnet.center[1], str(magnet.magnet_label), fontsize=8, \
                     rotation=abs(180 - magnet.orientation),weight='bold')  # +'\n'+str(magnet.hexabundle)


            plt.xlabel('mm')
            plt.ylabel('mm')
################################################################

    plt.figure(1)
    plt.axis('off')
    axes = plt.gca()
    axes.set_xlim([-350, 350])
    axes.set_ylim([-350, 350])

    # plt.show()                ## for showing the figure of magnets with pickup area

    plt.savefig(hexabundle_figureFile)
    # plt.savefig("image.png", bbox_inches='tight', dpi=100)

    plt.figure(2)
    plt.savefig(robot_figureFile)

    f = plt.figure(1)
    f1 = plt.figure(2)
    f.clear()
    f1.clear()
    plt.close(f)
    plt.close(f1)

def create_magnet_pickup_areas(magnets):

    for magnet in magnets:
        magnet.create_pickup_areas()

def draw_magnet_pickup_areas(magnets, colour):

    for magnet in magnets:
        for pickup_area in magnet.pickup_areas:
            pickup_area.draw_rectangle(colour)

