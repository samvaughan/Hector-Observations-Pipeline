import matplotlib.pyplot as plt
import matplotlib.patches as patches
from collections import Counter
import matplotlib.cm as cm
import pandas as pd
import numpy as np
import datetime
import string as STRING # Added by Sam to workaround assignemnt to variable string
import csv
import re
import random
from pathlib import Path
from ..problem_operations import extract_data, file_arranging, plots
from ..problem_operations.plots import draw_circularSegments,sky_fibre_annotations,read_sky_fibre_file,coordinates_and_angle_of_skyFibres, plot_skyfibre_section


def create_fibreSlit_info_file(fileNameHexa,fibre_file,output_fibreSlitInfo):

    skyfibreDict = plots.read_sky_fibre_file(fileNameHexa)

    pd.set_option('display.max_columns', 12)

    fibre_data = pd.read_csv(fibre_file)
    # print(skyfibreDict)
    # print(fibre_data)

    nullPosition_skyFibre = {}
    index_1 = 0
    for i in skyfibreDict:
        for j in skyfibreDict[i]:
            for k in j:
                if int(j[k]) == 0:
                    # if (str(i)) not in nullPosition_skyFibre:
                    #     nullPosition_skyFibre[str(i)] = []
                    # nullPosition_skyFibre[str(i)].append(k)

                    index2 = 0
                    # read through fibre data to match null position skyfibres and update the 'hexabundle or skyfibre' column
                    for l in fibre_data['Spectrograph']:

                        if (fibre_data['Bundle/plate'][index2] == str(i)) and (int(fibre_data['Fibre_number'][index2]) == int(k)):
                            fibre_data['Hexabundle or sky fibre'][index2] = 'BLOCK-sky'

                        index2 += 1

            index_1 += 1

    fibre_data.to_csv(output_fibreSlitInfo, index=False, sep=',')

    # print(nullPosition_skyFibre)



def extract_fibreInfo(fibre_file,output_fibreAAOmega,output_fibreSpector):

    fibre_data = pd.read_csv(fibre_file)

    hexabundle_names = list(STRING.ascii_uppercase)[:21]

    # Creates a list containing w lists, each of h item/s, all filled with 0
    w, h1, h2 = 10, 820, 856
    new_arrayAAOmega = [['0' for x in range(w)] for y in range(h1)]
    new_arraySpector = [['0' for x in range(w)] for y in range(h2)]


    # headers for the spectrograph fibres file
    new_arrayAAOmega[0] = new_arraySpector[0] = ['slit_pos', 'type(S/P)', 'probe_num', 'probe_fibre', \
                                'Good/bad', 'label', 'slitlet', 'cores_count', 'ring', 'position_on_slit']

    j = 1
    for i in fibre_data['Hexabundle or sky fibre']:
        if fibre_data['Spectrograph'][0] == 'AAOmega' and j < 820:

            new_arrayAAOmega[j][0] = j
            if str(i) == 'Sky':
                new_arrayAAOmega[j][1] = 'S'
                new_arrayAAOmega[j][4] = 'nan'
            elif str(i) == 'H':
                new_arrayAAOmega[j][1] = 'P'
                new_arrayAAOmega[j][4] = 'Good'
            elif str(i) == 'BLOCK':
                new_arrayAAOmega[j][4] = 'nan'
            elif str(i) == 'BROKEN':
                # Added by Sam. We need to know if a broken fibre was a sky fibre or not
                # If it's a hexabundle fibre, its bundle/plate value with be a capital letter from A to U
                # So check for this and mark it as 'P'
                if fibre_data['Bundle/plate'][j-1] in hexabundle_names:
                    new_arrayAAOmega[j][1] = 'P'
                else:
                    new_arrayAAOmega[j][1] = 'S'
                new_arrayAAOmega[j][4] = 'nan'
            new_arrayAAOmega[j][2] = new_arrayAAOmega[j][5] = fibre_data['Bundle/plate'][j-1]
            if fibre_data['Fibre_number'][j-1] > 0:
                new_arrayAAOmega[j][3] = round(fibre_data['Fibre_number'][j-1])
            else:
                new_arrayAAOmega[j][3] = 'nan'
            if fibre_data['no.cores'][j-1] > 0:
                new_arrayAAOmega[j][7] = round(fibre_data['no.cores'][j-1])
            else:
                new_arrayAAOmega[j][7] = 'nan'
            new_arrayAAOmega[j][6] = fibre_data['slitlet'][j-1]
            new_arrayAAOmega[j][8] = fibre_data['ring'][j-1]
            new_arrayAAOmega[j][9] = fibre_data['position on slit '][j-1]  #*** Might need to swap places with fibre number **

        elif fibre_data['Spectrograph'][819] == 'Spector' and j >= 820:

            new_arraySpector[j-819][0] = j-819
            if str(i) == 'Sky':
                new_arraySpector[j-819][1] = 'S'
                new_arraySpector[j-819][4] = 'nan'
            elif str(i) == 'H':
                new_arraySpector[j-819][1] = 'P'
                new_arraySpector[j-819][4] = 'Good'
            elif str(i) == 'BLOCK':
                new_arraySpector[j-819][4] = 'nan'
            elif str(i) == 'BROKEN':
                # Added by Sam. We need to know if a broken fibre was a sky fibre or not
                # If it's a hexabundle fibre, its bundle/plate value with be a capital letter from A to U
                # So check for this and mark it as 'P'
                if fibre_data['Bundle/plate'][j-1] in hexabundle_names:
                    new_arraySpector[j-819][1] = 'P'
                else:
                    new_arraySpector[j-819][1] = 'S'
                new_arraySpector[j-819][4] = 'nan'
            new_arraySpector[j-819][2] = new_arraySpector[j-819][5] = fibre_data['Bundle/plate'][j - 1]
            if fibre_data['Fibre_number'][j - 1] > 0:
                new_arraySpector[j-819][3] = round(fibre_data['Fibre_number'][j - 1])
            else:
                new_arraySpector[j-819][3] = 'nan'
            if fibre_data['no.cores'][j - 1] > 0:
                new_arraySpector[j-819][7] = round(fibre_data['no.cores'][j - 1])
            else:
                new_arraySpector[j-819][7] = 'nan'
            new_arraySpector[j-819][6] = fibre_data['slitlet'][j - 1]
            new_arraySpector[j-819][8] = fibre_data['ring'][j - 1]
            new_arraySpector[j-819][9] = fibre_data['position on slit '][j-1]  # *** Might need to swap places with fibre number **

        j += 1

    # print(new_arrayAAOmega)
    # print(new_arraySpector)

    for i in range(2):
        if i == 0:
            output_fibreFile = output_fibreAAOmega
            fibre_array = new_arrayAAOmega
            string_to_save = 'AAOmega'
        elif i == 1:
            output_fibreFile = output_fibreSpector
            fibre_array = new_arraySpector
            string_to_save = 'Spector'

        my_file = Path(output_fibreFile)
        if my_file.is_file():
            print('Fibre Spectrograph file exists.')
        else:
            # write the fibre file array into the CSV file for the fibre spectrograph
            with open(output_fibreFile, 'w+') as output_fibre:
                output_fibre.write('# Experimental implementation of Hector_fibres_')
                output_fibre.write(string_to_save+'.txt\n')
                output_fibre.write('# \n')
                output_fibre.write('#   Ayoan Sadman\n')
                output_fibre.write('#   ')
                output_fibre.write(str(datetime.datetime.now().strftime('%B %d %y'))+'\n')
                output_fibre.write('#   Generated by "The Hector Galaxy Survey Observation Pipeline"\n')
                output_fibre.write('# \n')
                output_fibre.write('# ')
                writer = csv.writer(output_fibre, delimiter=' ', lineterminator='\n')
                writer.writerows(fibre_array)

    return new_arrayAAOmega,new_arraySpector


def create_hexabundleFibre_coordData(output_hexabundle_coordData):

    hexabundles = 'ABCDEFGHIJKLMNOPQRSTU'

    for i in hexabundles:
        if i < 'C':
            cores = 169
        elif i == 'C':
            cores = 127
        elif (i == 'D') or (i == 'I'):
            cores = 91
        elif ('D' < i < 'H') or ('I' < i < 'U'):
            cores = 61
        elif (i == 'H') or (i == 'U'):
            cores = 37

        # Creates a list containing w lists, each of h item/s, all filled with 0
        w, h = 4, cores
        file_array = [['0' for x in range(w)] for y in range(h)]

        for j in range(1,cores+1):
            file_array[j-1][0] = j
            if j == 1:
                ring = 0
            elif 1 < j < 8:
                ring = 1
            elif 7 < j < 20:
                ring = 2
            elif 19 < j < 38:
                ring = 3
            elif 37 < j < 62:
                ring = 4
            elif 61 < j < 92:
                ring = 5
            elif 91 < j < 128:
                ring = 6
            elif 127 < j < 170:
                ring = 7
            file_array[j - 1][1] = ring

        # print(file_array)

        output_file = output_hexabundle_coordData + 'hexabundle_' + i + '.txt'

        my_file = Path(output_file)
        if my_file.is_file():
            print('Hexabundle coordinate data of '+i+ ' file exists.')
        else:
            # write the hexabundle file array into the CSV file for the fibre coordinate data
            with open(output_file, 'w+') as output_fibre:
                output_fibre.write('# Hexabundle "')
                output_fibre.write(str(i) + '" Coordinates Definition - _transformed\n')
                output_fibre.write('# Date: ')
                output_fibre.write(str(datetime.datetime.now().strftime('%d-%B-%y %H:%M:%S')) + '\n')
                output_fibre.write('# Probe: Probe ' + str(i) + '\n')
                output_fibre.write('# Hexabundle: ' + str(i) +  '\n')
                output_fibre.write('# Manufacture:  \n')
                output_fibre.write('# col 1: ID  \n')
                output_fibre.write('# col 2: Radial Ring Number \n')
                output_fibre.write('# col 3: Fiber Core X [sky-east] (microns) \n')
                output_fibre.write('# col 4: Fiber Core Y [sky-north] (microns) \n')
                writer = csv.writer(output_fibre, delimiter=' ', lineterminator='\n')
                writer.writerows(file_array)


def convert_rectangularMagnetOrientation(magnet):

    #   Probe orientation is the rotation of the hexabundle on sky between -180 to +180 degrees
    #   where 0 degrees is the orientation in which the cable end of the hexabundle ferule is
    #   pointing north on the field plate and the hexabundle end of the ferule is pointing South,
    #   and the table defining the centre positions of the fibre cores is oriented such that North
    #   is at the top, as imaged at input face of the prism. The image is flipped N-S through the
    #   prism and hence the north direction maps to the field-plate side of the hexabundle face. The
    #   positive angle rotation rotates the ferule tail towards the astronomical west on the field
    #   plate, which rotates the image on the hexabundle towards from north towards east.

    if magnet.__class__.__name__ == 'rectangular_magnet':

        probe_orientation = magnet.orientation
        while probe_orientation > 180:
            probe_orientation = probe_orientation - 360
        while probe_orientation < -180:
            probe_orientation = probe_orientation + 360

    elif magnet.__class__.__name__ == 'circular_magnet':
        probe_orientation = 0

    return probe_orientation

def create_slitletFigure(new_arrayAAOmega,new_arraySpector,fibreFigure_AAOmega, fibreFigure_Spector):

    # print((new_arrayAAOmega))
    print('figure being created')
    # fibre_data = pd.read_excel(fibre_file)

    plt.figure(3)
    plt.clf()
    j = 1
    for slitlet_count in range(13):

        y_start = 858 - (66 * (slitlet_count+1))
        plt.gcf().gca().add_patch(patches.Rectangle((20, y_start), 20, 63, edgecolor='black',facecolor='none',lw=2, zorder=3))

        colours = ['#C7980A', '#F4651F', '#82D8A7', '#CC3A05', '#575E76', '#156943', '#0BD055', '#ACD338']

        y = y_start + 62
        x_1 = 20
        x_2 = 40
        for i in range(63):

            if len(str(new_arrayAAOmega[j][2])) == 2:
                color = 'skyblue'
            elif len(str(new_arrayAAOmega[j][2])) == 3:
                color = 'red'
            elif str(new_arrayAAOmega[j][2]) == 'A':
                color = 'navy'
            elif str(new_arrayAAOmega[j][2]) == 'B':
                color = 'purple'
            elif str(new_arrayAAOmega[j][2]) == 'C':
                color = 'peru'
            elif str(new_arrayAAOmega[j][2]) == 'D':
                color = 'green'
            elif str(new_arrayAAOmega[j][2]) == 'E':
                color = 'blue'
            elif str(new_arrayAAOmega[j][2]) == 'F':
                color = 'brown'
            elif str(new_arrayAAOmega[j][2]) == 'G':
                color = 'grey'
            elif str(new_arrayAAOmega[j][2]) == 'H':
                color = 'gold'

            plt.gcf().gca().add_patch(plt.Rectangle((x_1, y), x_2 - x_1, 1,facecolor=color))
            y = y - 1
            j += 1

        plt.gcf().gca().add_artist(plt.annotate(str(slitlet_count + 1), xy=(30, y + 30), xytext=(30, y + 30), xycoords='data', \
                         fontsize=10, ha='right', va='center', rotation=0, color='white'))

    # Adjust the fontsizes on tick labels for this plot
    fs = 9.0

    # Here is the label and arrow code of interest
    plt.gcf().gca().add_artist(plt.annotate('A', xy=(19, 763), xytext=(16, 763), xycoords='data',\
                fontsize=fs , ha='right',va='center', rotation=0, color='navy',  \
                bbox=dict(boxstyle='square', fc='white'),\
                arrowprops=dict(arrowstyle='-[, widthB=3.0, lengthB=0.5', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=51', xy=(20, 672), xytext=(16, 672), xycoords='data',\
                fontsize=fs * 0.7 , ha='right',va='center', rotation=0, color='navy', \
                arrowprops=dict(arrowstyle='-', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('C', xy=(41, 601), xytext=(44, 601), xycoords='data',\
                fontsize=fs , ha='right',va='center', rotation=0, color='peru',  \
                bbox=dict(boxstyle='square', fc='white'),\
                arrowprops=dict(arrowstyle='-[, widthB=2.45, lengthB=0.5', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=52', xy=(40, 672), xytext=(44, 672), xycoords='data',\
                fontsize=fs * 0.7 , ha='left',va='center', rotation=0, color='peru', \
                arrowprops=dict(arrowstyle='-', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('F', xy=(19, 493.5), xytext=(16, 493.5), xycoords='data', \
                fontsize=fs, ha='right', va='center', rotation=0, color='brown', \
                bbox=dict(boxstyle='square', fc='white'), \
                arrowprops=dict(arrowstyle='-[, widthB=1.0, lengthB=0.5', lw=1.0)))

    # plt.gcf().gca().add_artist(plt.annotate('fibre_num=58', xy=(19, 530), xytext=(16, 530), xycoords='data', \
    #             fontsize=fs * 0.7, ha='right', va='center', rotation=0, color='brown', \
    #             arrowprops=dict(arrowstyle='-', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('F', xy=(19, 493.5), xytext=(16, 493.5), xycoords='data', \
                fontsize=fs, ha='right', va='center', rotation=0, color='brown', \
                bbox=dict(boxstyle='square', fc='white'), \
                arrowprops=dict(arrowstyle='-[, widthB=1.0, lengthB=0.5', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('G', xy=(19, 427.5), xytext=(16, 427.5), xycoords='data', \
                fontsize=fs, ha='right', va='center', rotation=0, color='grey', \
                bbox=dict(boxstyle='square', fc='white'), \
                arrowprops=dict(arrowstyle='-[, widthB=1.0, lengthB=0.5', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('E', xy=(19, 361.5), xytext=(16, 361.5), xycoords='data', \
                fontsize=fs, ha='right', va='center', rotation=0, color='blue', \
                bbox=dict(boxstyle='square', fc='white'), \
                arrowprops=dict(arrowstyle='-[, widthB=1.0, lengthB=0.5', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('D', xy=(41, 285.5), xytext=(44, 285.5), xycoords='data', \
                fontsize=fs, ha='left', va='center', rotation=0, color='green', \
                bbox=dict(boxstyle='square', fc='white'), \
                arrowprops=dict(arrowstyle='-[, widthB=1.1, lengthB=0.5', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=16', xy=(40, 245), xytext=(44, 245), xycoords='data',\
                fontsize=fs * 0.7 , ha='left',va='center', rotation=0, color='green', \
                arrowprops=dict(arrowstyle='-', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('D', xy=(41, 197), xytext=(44, 197), xycoords='data', \
                fontsize=fs, ha='left', va='center', rotation=0, color='green', \
                bbox=dict(boxstyle='square', fc='white'), \
                arrowprops=dict(arrowstyle='-[, widthB=0.3, lengthB=0.5, angleB=1.0', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=54', xy=(40, 208), xytext=(44, 136), xycoords='data',\
                fontsize=fs * 0.7 , ha='left',va='center', rotation=0, color='green', \
                arrowprops=dict(arrowstyle='-', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=9', xy=(40, 186), xytext=(43, 96), xycoords='data',\
                fontsize=fs * 0.7 , ha='left',va='center', rotation=0, color='green', \
                arrowprops=dict(arrowstyle='-', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('fibre_num=17\n ★ H ★\n fibre_num=53', xy=(19, 226.5), xytext=(16, 226.5), xycoords='data', \
                fontsize=fs*0.8, ha='right', va='center', rotation=0, color='black', \
                bbox=dict(boxstyle='square', fc='white'), \
                arrowprops=dict(arrowstyle='-[, widthB=0.7, lengthB=0.5', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('B', xy=(19, 94), xytext=(16, 94), xycoords='data', \
                fontsize=fs, ha='right', va='center', rotation=0, color='purple', \
                bbox=dict(boxstyle='square', fc='white'), \
                arrowprops=dict(arrowstyle='-[, widthB=3.0, lengthB=0.5', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=10', xy=(20, 186), xytext=(16, 146), xycoords='data', \
                fontsize=fs * 0.7, ha='right', va='center', rotation=0, color='purple', \
                arrowprops=dict(arrowstyle='-', lw=1.0)))

    for i in [793,727,661,595,199,133,67,1]:
        plt.gcf().gca().add_artist(plt.annotate('▮ BLOCK', xy=(40, i), xytext=(50, 400), xycoords='data', \
                                                fontsize=fs * 0.7, ha='right', va='center', rotation=0, color='red', \
                                                arrowprops=dict(arrowstyle='-', color='red',lw=0.7)))
    # plt.gcf().gca().add_artist(plt.annotate('▮ BLOCK', xy=(40, 1), xytext=(48, 400), xycoords='data', \
    #                                             fontsize=fs * 0.7, ha='right', va='center', rotation=0, color='red', \
    #                                             arrowprops=dict(arrowstyle='-', color='red',lw=0.8)))

    plt.xlim(-10, 70)
    plt.ylim(-10, 870)
    plt.axis('off')
    # plt.show()
    figure = plt.gcf()

    figure.set_size_inches(6,8)
    plt.savefig(fibreFigure_AAOmega, dpi=500)

    ## Spector slitelets figure

    plt.figure(4)
    plt.clf()
    j = 1
    for slitlet_count in range(19):

        y_start = 915 - (48 * (slitlet_count + 1))
        plt.gcf().gca().add_patch(patches.Rectangle((20, y_start), 20, 45, edgecolor='black', facecolor='none', lw=2, zorder=3))

        y = y_start + 44
        x_1 = 20
        x_2 = 40
        for i in range(45):

            if len(str(new_arraySpector[j][2])) == 2:
                color = 'skyblue'
            elif len(str(new_arraySpector[j][2])) == 3:
                color = 'red'
            elif str(new_arraySpector[j][2]) == 'I':
                color = 'navy'
            elif str(new_arraySpector[j][2]) == 'J':
                color = 'peru'
            elif str(new_arraySpector[j][2]) == 'K':
                color = 'brown'
            elif str(new_arraySpector[j][2]) == 'L':
                color = 'purple'
            elif str(new_arraySpector[j][2]) == 'M':
                color = 'orange'
            elif str(new_arraySpector[j][2]) == 'N':
                color = 'green'
            elif str(new_arraySpector[j][2]) == 'O':
                color = 'chocolate'
            elif str(new_arraySpector[j][2]) == 'P':
                color = 'navy'
            elif str(new_arraySpector[j][2]) == 'Q':
                color = 'brown'
            elif str(new_arraySpector[j][2]) == 'R':
                color = 'peru'
            elif str(new_arraySpector[j][2]) == 'S':
                color = 'blue'
            elif str(new_arraySpector[j][2]) == 'T':
                color = 'crimson'
            elif str(new_arraySpector[j][2]) == 'U':
                color = 'gold'

            plt.gcf().gca().add_patch(plt.Rectangle((x_1, y), x_2 - x_1, 1, facecolor=color))
            y = y - 1
            j += 1

        plt.gcf().gca().add_artist(
            plt.annotate(str(slitlet_count + 1), xy=(30, y + 22), xytext=(30, y + 22), xycoords='data', \
                         fontsize=10, ha='right', va='center', rotation=0, color='white'))

    # Adjust the fontsizes on tick labels for this plot
    fs = 9.0

    # Here is the label and arrow code of interest
    plt.gcf().gca().add_artist(plt.annotate('I', xy=(19, 859.5), xytext=(16, 859.5), xycoords='data', \
                                            fontsize=fs, ha='right', va='center', rotation=0, color='navy', \
                                            bbox=dict(boxstyle='square', fc='white'), \
                                            arrowprops=dict(arrowstyle='-[, widthB=2.0, lengthB=0.5', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=8', xy=(20, 808), xytext=(16, 808), xycoords='data', \
                                            fontsize=fs * 0.7, ha='right', va='center', rotation=0, color='navy', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('J', xy=(41, 774), xytext=(44, 774), xycoords='data', \
                                            fontsize=fs, ha='right', va='center', rotation=0, color='peru', \
                                            bbox=dict(boxstyle='square', fc='white'), \
                                            arrowprops=dict(arrowstyle='-[, widthB=1.0, lengthB=0.5', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=9', xy=(40, 807), xytext=(44, 807), xycoords='data', \
                                            fontsize=fs * 0.7, ha='left', va='center', rotation=0, color='peru', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=27', xy=(40, 741), xytext=(44, 741), xycoords='data', \
                                            fontsize=fs * 0.7, ha='left', va='center', rotation=0, color='peru', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('K', xy=(19, 704), xytext=(16, 704), xycoords='data', \
                                            fontsize=fs, ha='right', va='center', rotation=0, color='brown', \
                                            bbox=dict(boxstyle='square', fc='white'), \
                                            arrowprops=dict(arrowstyle='-[, widthB=1.0, lengthB=0.5', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=28', xy=(20, 740), xytext=(16, 740), xycoords='data', \
                                            fontsize=fs * 0.7, ha='right', va='center', rotation=0, color='brown', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=4', xy=(20, 668), xytext=(16, 668), xycoords='data', \
                                            fontsize=fs * 0.7, ha='right', va='center', rotation=0, color='brown', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('L', xy=(41, 634), xytext=(44, 634), xycoords='data', \
                                            fontsize=fs, ha='left', va='center', rotation=0, color='purple', \
                                            bbox=dict(boxstyle='square', fc='white'), \
                                            arrowprops=dict(arrowstyle='-[, widthB=1.0, lengthB=0.5', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=5', xy=(40, 667), xytext=(44, 667), xycoords='data', \
                                            fontsize=fs * 0.7, ha='left', va='center', rotation=0, color='purple', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=23', xy=(40, 601), xytext=(44, 601), xycoords='data', \
                                            fontsize=fs * 0.7, ha='left', va='center', rotation=0, color='purple', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('M', xy=(19, 567), xytext=(16, 567), xycoords='data', \
                                            fontsize=fs, ha='right', va='center', rotation=0, color='orange', \
                                            bbox=dict(boxstyle='square', fc='white'), \
                                            arrowprops=dict(arrowstyle='-[, widthB=1.0, lengthB=0.5', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=24', xy=(20, 600), xytext=(16, 600), xycoords='data', \
                                            fontsize=fs * 0.7, ha='right', va='center', rotation=0, color='orange', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=42', xy=(20,534), xytext=(16,534), xycoords='data', \
                                            fontsize=fs * 0.7, ha='right', va='center', rotation=0, color='orange', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('N', xy=(41, 497), xytext=(44, 497), xycoords='data', \
                                            fontsize=fs, ha='left', va='center', rotation=0, color='green', \
                                            bbox=dict(boxstyle='square', fc='white'), \
                                            arrowprops=dict(arrowstyle='-[, widthB=1.0, lengthB=0.5', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=43', xy=(40, 533), xytext=(44, 533), xycoords='data', \
                                            fontsize=fs * 0.7, ha='left', va='center', rotation=0, color='green', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=19', xy=(40, 461), xytext=(44, 461), xycoords='data', \
                                            fontsize=fs * 0.7, ha='left', va='center', rotation=0, color='green', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('O', xy=(19, 427), xytext=(16, 427), xycoords='data', \
                                            fontsize=fs, ha='right', va='center', rotation=0, color='chocolate', \
                                            bbox=dict(boxstyle='square', fc='white'), \
                                            arrowprops=dict(arrowstyle='-[, widthB=1.0, lengthB=0.5', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=20', xy=(20, 460), xytext=(16, 460), xycoords='data', \
                                            fontsize=fs * 0.7, ha='right', va='center', rotation=0, color='chocolate', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=38', xy=(20,394), xytext=(16,394), xycoords='data', \
                                            fontsize=fs * 0.7, ha='right', va='center', rotation=0, color='chocolate', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('P', xy=(41, 357), xytext=(44, 357), xycoords='data', \
                                            fontsize=fs, ha='left', va='center', rotation=0, color='navy', \
                                            bbox=dict(boxstyle='square', fc='white'), \
                                            arrowprops=dict(arrowstyle='-[, widthB=1.0, lengthB=0.5', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=39', xy=(40, 393), xytext=(44, 393), xycoords='data', \
                                            fontsize=fs * 0.7, ha='left', va='center', rotation=0, color='navy', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=15', xy=(40, 321), xytext=(44, 321), xycoords='data', \
                                            fontsize=fs * 0.7, ha='left', va='center', rotation=0, color='navy', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('Q', xy=(19, 287), xytext=(16, 287), xycoords='data', \
                                            fontsize=fs, ha='right', va='center', rotation=0, color='brown', \
                                            bbox=dict(boxstyle='square', fc='white'), \
                                            arrowprops=dict(arrowstyle='-[, widthB=1.0, lengthB=0.5', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=16', xy=(20, 320), xytext=(16, 320), xycoords='data', \
                                            fontsize=fs * 0.7, ha='right', va='center', rotation=0, color='brown', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=34', xy=(20,254), xytext=(16,254), xycoords='data', \
                                            fontsize=fs * 0.7, ha='right', va='center', rotation=0, color='brown', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('R', xy=(41, 249), xytext=(44, 249), xycoords='data', \
                                            fontsize=fs, ha='left', va='center', rotation=0, color='peru', \
                                            bbox=dict(boxstyle='square', fc='white'), \
                                            arrowprops=dict(arrowstyle='-[, widthB=0.3, lengthB=0.5', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=35', xy=(40, 254), xytext=(44, 284), xycoords='data', \
                                            fontsize=fs * 0.7, ha='left', va='center', rotation=0, color='peru', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('R', xy=(41, 170), xytext=(44, 170), xycoords='data', \
                                            fontsize=fs, ha='left', va='center', rotation=0, color='peru', \
                                            bbox=dict(boxstyle='square', fc='white'), \
                                            arrowprops=dict(arrowstyle='-[, widthB=1.0, lengthB=0.5', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=39', xy=(40, 201), xytext=(44, 210), xycoords='data', \
                                            fontsize=fs * 0.7, ha='left', va='center', rotation=0, color='peru', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=6', xy=(40, 138), xytext=(44, 130), xycoords='data', \
                                            fontsize=fs * 0.7, ha='left', va='center', rotation=0, color='peru', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('S', xy=(19, 104), xytext=(16, 104), xycoords='data', \
                                            fontsize=fs, ha='right', va='center', rotation=0, color='blue', \
                                            bbox=dict(boxstyle='square', fc='white'), \
                                            arrowprops=dict(arrowstyle='-[, widthB=1.0, lengthB=0.5', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=7', xy=(20, 137), xytext=(16, 137), xycoords='data', \
                                            fontsize=fs * 0.7, ha='right', va='center', rotation=0, color='blue', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=25', xy=(20, 71), xytext=(16, 71), xycoords='data', \
                                            fontsize=fs * 0.7, ha='right', va='center', rotation=0, color='blue', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('T', xy=(41, 37), xytext=(44, 37), xycoords='data', \
                                            fontsize=fs, ha='left', va='center', rotation=0, color='crimson', \
                                            bbox=dict(boxstyle='square', fc='white'), \
                                            arrowprops=dict(arrowstyle='-[, widthB=1.0, lengthB=0.5', lw=1.0)))
    plt.gcf().gca().add_artist(plt.annotate('fibre_num=26', xy=(40, 70), xytext=(44, 70), xycoords='data', \
                                            fontsize=fs * 0.7, ha='left', va='center', rotation=0, color='crimson', \
                                            arrowprops=dict(arrowstyle='-', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('fibre_num=2\n ★ U ★\n fibre_num=38', xy=(19, 220), xytext=(16, 210), \
                                            xycoords='data', fontsize=fs * 0.6, ha='right', va='center', rotation=0, \
                                            color='black', bbox=dict(boxstyle='square', fc='white'), \
                                            arrowprops=dict(arrowstyle='-[, widthB=0.7, lengthB=0.5', lw=1.0)))

    plt.gcf().gca().add_artist(plt.annotate('▮ BLOCK', xy=(40, 867), xytext=(44, 867), xycoords='data', \
                                            fontsize=fs, ha='left', va='center', rotation=0, color='red', \
                                            bbox=dict(boxstyle='square', fc='white'), \
                                            arrowprops=dict(arrowstyle='-', color='red', lw=0.7)))

    # for i in [793, 727, 661, 595, 199, 133, 67, 1]:
    #     plt.gcf().gca().add_artist(plt.annotate('▮ BLOCK', xy=(40, i), xytext=(45, 400), xycoords='data', \
    #                                             fontsize=fs * 0.7, ha='right', va='center', rotation=0, color='red', \
    #                                             arrowprops=dict(arrowstyle='-', color='red', lw=0.7)))


    plt.xlim(-10, 70)
    plt.ylim(-10, 930)
    plt.gcf().set_size_inches(6, 8)
    plt.axis('off')
    # plt.show()

    plt.savefig(fibreFigure_Spector, dpi=500)


def create_skyFibreSlitlet_figure( fileNameHexa, new_arrayAAOmega, new_arraySpector, skyFibre_AAOmegaFigure, skyFibre_SpectorFigure):

    plt.figure(5)
    plt.clf()
    fs = 5

    skyfibreDict = read_sky_fibre_file(fileNameHexa)
    # print(skyfibreDict)

    skyFibresArray_AAOmega = {}
    for i in new_arrayAAOmega[1:]:
        if str(i[1]) != 'P':
            key = i[6]
            value = str(i[5])
            value_2 = i[9]

            fibre_num = i[3]
            #print(value, fibre_num)
            if value == 'nan':
                value_3 = 0
            else:
                #print(skyfibreDict[value][fibre_num-1][fibre_num])
                value_3 = skyfibreDict[value][fibre_num-1][fibre_num] # position
            skyFibresArray_AAOmega.setdefault(key, []).append({value: [value_2,value_3,fibre_num]})

    # print('HERE skyfibre dictionary \n')
    # print(new_arrayAAOmega)
    # print(skyFibresArray_AAOmega)

    for slitlet_count in range(13):

        # plotting the slitlet as a rectangle
        x_start = 20
        y_start = 858 - (80 * (slitlet_count + 1))
        plt.gcf().gca().add_patch(
            patches.Rectangle((x_start, y_start), 10, 63, facecolor='grey', lw=2, zorder=2))

        # plotting the ending borderline for each slitlet after the slitlet number label
        y = y_start + 62
        plt.gcf().gca().add_patch(
            patches.Rectangle((x_start - 10, y - 70), 10, 2, facecolor='black', lw=2, zorder=2))

        # plotting the slitlet number as a label beside the slitlet
        plt.gcf().gca().add_artist(
            plt.annotate(str(slitlet_count + 1), xy=(x_start - 6, y - 30), xytext=(x_start - 8, y - 30), \
                         xycoords='data', fontsize=10, ha='right', va='center', rotation=0, color='black'))

        direction = 'left'
        x_text = x_start - 6
        for i in skyFibresArray_AAOmega:
            if (slitlet_count + 1) == i:
                k = 0
                for j in skyFibresArray_AAOmega[i]:

                    for l in j:

                        position = skyFibresArray_AAOmega[i][k][l][1]  # MUST CHANGE TO READING ACTUAL POSITION OF SKYFIBRE SUBPLATES
                        if position == 0:
                            fill_color = 'black'
                        elif position != 0:
                            fill_color = 'grey'


                        if l == 'nan':
                            color = 'red'
                            text = '▮'

                        else:
                            color = 'blue'
                            text = l+' - '+str(skyFibresArray_AAOmega[i][k][l][2])

                        if skyFibresArray_AAOmega[i][k][l][0] == 1:
                            Y_adjustment = 53
                        elif skyFibresArray_AAOmega[i][k][l][0] == 2:
                            Y_adjustment = 43
                        elif skyFibresArray_AAOmega[i][k][l][0] == 3:
                            Y_adjustment = 33
                        elif skyFibresArray_AAOmega[i][k][l][0] == 61:
                            Y_adjustment = 20
                        elif skyFibresArray_AAOmega[i][k][l][0] == 62:
                            Y_adjustment = 10
                        elif skyFibresArray_AAOmega[i][k][l][0] == 63:
                            Y_adjustment = 0
                        plt.gcf().gca().add_patch(
                            patches.Rectangle((x_start, y_start + Y_adjustment), 10, 10, edgecolor=color,
                                              facecolor=fill_color,
                                              lw=1, zorder=3))

                        plt.gcf().gca().add_artist(
                            plt.annotate(text, xy=(x_start, y_start + Y_adjustment + 5),
                                         xytext=(x_text, y_start + Y_adjustment + 5), xycoords='data', fontsize=fs, \
                                         ha=direction, va='center', rotation=0, color=color,
                                         arrowprops=dict(arrowstyle='-', lw=1.0)))

                        plt.gcf().gca().add_artist(
                            plt.annotate(str(position), xy=(x_start+10, y_start + Y_adjustment + 5),
                                         xytext=(x_text+18, y_start + Y_adjustment + 5), xycoords='data', fontsize=fs, \
                                         ha='right', va='center', rotation=0, color=color,
                                         arrowprops=dict(arrowstyle='-', lw=1.0)))

                        if x_text == x_start - 6:
                            x_text = x_start - 4
                        elif x_text == x_start - 4:
                            x_text = x_start - 6

                    k += 1

    plt.xlim(0, 40)
    plt.ylim(-200, 900)
    plt.axis('off')
    # plt.show()
    plt.gcf().set_size_inches(6, 8)
    plt.savefig(skyFibre_AAOmegaFigure)


    plt.figure(6)
    plt.clf()
    fs = 5

    skyFibresArray_Spector = {}
    for i in new_arraySpector[1:]:
        if str(i[1]) != 'P':
            key = i[6]
            value = str(i[5])
            value_2 = i[9]

            fibre_num = i[3]
            #print(value, fibre_num)
            if value == 'nan':
                value_3 = 0 # position
            else:
                #print(skyfibreDict[value][fibre_num - 1][fibre_num])
                value_3 = skyfibreDict[value][fibre_num - 1][fibre_num]  # position
            skyFibresArray_Spector.setdefault(key, []).append({value: [value_2, value_3, fibre_num]})


    #print(skyFibresArray_Spector)

    for slitlet_count in range(19):

        x_start = 20
        y_start = 858 - (80 * (slitlet_count +1))
        plt.gcf().gca().add_patch(
            patches.Rectangle((x_start, y_start), 10, 63, facecolor='grey', lw=2, zorder=2))

        y = y_start + 62
        plt.gcf().gca().add_patch(
            patches.Rectangle((x_start - 10, y - 70), 10, 2, facecolor='black', lw=2, zorder=2))



        plt.gcf().gca().add_artist(
            plt.annotate(str(slitlet_count + 1), xy=(x_start - 6, y - 30), xytext=(x_start - 8, y - 30), \
                         xycoords='data', fontsize=10, ha='right', va='center', rotation=0, color='black'))

        direction = 'left'
        x_text = x_start - 6
        for i in skyFibresArray_Spector:
            if (slitlet_count+1) == i:
                k = 0
                for j in skyFibresArray_Spector[i]:

                    for l in j:

                        position = skyFibresArray_Spector[i][k][l][1] # MUST CHANGE TO READING ACTUAL POSITION OF SKYFIBRE SUBPLATES
                        if position == 0:
                            fill_color = 'black'
                        elif position != 0:
                            fill_color = 'grey'

                        if l == 'nan':
                            color = 'red'
                            text = '▮'
                        else:
                            color = 'blue'
                            text = l + ' - ' + str(skyFibresArray_Spector[i][k][l][2])

                        if skyFibresArray_Spector[i][k][l][0] == 1:
                            Y_adjustment = 53
                        elif skyFibresArray_Spector[i][k][l][0] == 2:
                            Y_adjustment = 43
                        elif skyFibresArray_Spector[i][k][l][0] == 3:
                            Y_adjustment = 33
                        elif skyFibresArray_Spector[i][k][l][0] == 43:
                            Y_adjustment = 20
                        elif skyFibresArray_Spector[i][k][l][0] == 44:
                            Y_adjustment = 10
                        elif skyFibresArray_Spector[i][k][l][0] == 45:
                            Y_adjustment = 0
                        plt.gcf().gca().add_patch(
                            patches.Rectangle((x_start, y_start + Y_adjustment), 10, 10, edgecolor=color, facecolor=fill_color,
                                              lw=1, zorder=3))

                        plt.gcf().gca().add_artist(
                            plt.annotate(text, xy=(x_start, y_start + Y_adjustment + 5),
                                         xytext=(x_text, y_start + Y_adjustment + 5), xycoords='data', fontsize=fs, \
                                         ha=direction, va='center', rotation=0, color=color,
                                         arrowprops=dict(arrowstyle='-', lw=1.0)))

                        plt.gcf().gca().add_artist(
                            plt.annotate(str(position), xy=(x_start + 10, y_start + Y_adjustment + 5),
                                         xytext=(x_text + 18, y_start + Y_adjustment + 5), xycoords='data', fontsize=fs, \
                                         ha='right', va='center', rotation=0, color=color,
                                         arrowprops=dict(arrowstyle='-', lw=1.0)))

                        if x_text == x_start - 6:
                            x_text = x_start - 4
                        elif x_text == x_start - 4:
                            x_text = x_start - 6

                    k += 1


    plt.xlim(0, 40)
    # plt.ylim(420, 870)
    plt.ylim(-680, 900)
    plt.axis('off')
    # plt.show()
    plt.gcf().set_size_inches(6, 8)
    plt.savefig(skyFibre_SpectorFigure)

def plot_bar_from_dict(dict, ax=None):
    """"
    This function creates a bar plot from a dictionary.

    :param dict: A dictionary with the item as the key
     and the frequency as the value
    :param ax: an axis of matplotlib
    :return: the axis wit the object in it
    """

    if ax is None:
        fig = plt.figure()
        ax = fig.add_subplot(111)

    frequencies = dict.values()
    names = dict.keys()

    x_coordinates = np.arange(len(dict))
    ax.bar(x_coordinates, frequencies, align='center')

    ax.xaxis.set_major_locator(plt.FixedLocator(x_coordinates))
    ax.xaxis.set_major_formatter(plt.FixedFormatter(names))

    return ax

# check for magnet count per annulus and record any warnings on text file
def check_magnetCount_perAnnulus(self, tile_1_hexa, tile_2_hexa, tile_1_guide, tile_2_guide, annuliCount_batch):

    annuli_count_magnetCasing = {'Blu': 18, 'Gre': 19, 'Yel': 22, 'Mag': 21}

    # tile_1_hexa = f"{self.configuration_location}/HECTORConfig_Hexa_{self.config['output_filename_stem']}_{tile_number_1:03d}.txt"
    # tile_2_hexa = f"{self.configuration_location}/HECTORConfig_Hexa_{self.config['output_filename_stem']}_{(tile_number_2):03d}.txt"

    # tile_1_guide = f"{self.configuration_location}/HECTORConfig_Guides_{self.config['output_filename_stem']}_{tile_number_1:03d}.txt"
    # tile_2_guide = f"{self.configuration_location}/HECTORConfig_Guides_{self.config['output_filename_stem']}_{(tile_number_2):03d}.txt"

    tile_1_fname_stem = Path(tile_1_hexa).stem.strip('Hexas_')
    tile_2_fname_stem = Path(tile_2_hexa).stem.strip('Hexas_')

    # proxy output files
    plate_file_1 = f"{self.allocation_files_location_base}/Hexa_and_Guides_{tile_1_fname_stem}.txt"
    plate_file_2 = f"{self.allocation_files_location_base}/Hexa_and_Guides_{tile_2_fname_stem}.txt"
    # Output files 1
    guide_outputFile_1 = f"{self.allocation_files_location_tiles}/HECTOROutput_Guides_{self.config['output_filename_stem']}_tile_{tile_number_1:03d}.txt"
    guide_outputFile_2 = f"{self.allocation_files_location_tiles}/HECTOROutput_Guides_{self.config['output_filename_stem']}_tile_{tile_number_2:03d}.txt"

    # Adding ID column and removing the header line of Guides cluster to add to the hexa cluster
    df_guideFile_1, guideFileList_1 = file_arranging.arrange_guidesFile(tile_1_hexa, tile_1_guide, guide_outputFile_1)
    df_guideFile_2, guideFileList_2 = file_arranging.arrange_guidesFile(tile_2_hexa, tile_2_guide, guide_outputFile_2)

    # Adding guides cluster txt file to hexa cluster txt file
    file_arranging.merge_hexaAndGuides(tile_1_hexa, df_guideFile_1, plate_file_1)
    file_arranging.merge_hexaAndGuides(tile_2_hexa, df_guideFile_2, plate_file_2)

    # extracting all the magnets and making a list of them from the plate_file
    all_magnets_1 = extract_data.create_list_of_all_magnets_from_file(extract_data.get_file(plate_file_1),
                                                                      guideFileList_1)
    all_magnets_2 = extract_data.create_list_of_all_magnets_from_file(extract_data.get_file(plate_file_2),
                                                                      guideFileList_2)

    # annuli_count_Blu ,annuli_count_Gre, annuli_count_Yel, annuli_count_Mag = 0,0,0,0
    annuli_count = {'Blu': 0, 'Gre': 0, 'Yel': 0, 'Mag': 0}

    # counting magnets in each annulus for tiles 1 and 2 and storing count in annuli_count dictionary
    for magnet in np.concatenate((all_magnets_1, all_magnets_2)):
        if magnet.__class__.__name__ == 'circular_magnet':
            # print(magnet.__class__.__name__+' '+str(magnet.index)+' '+str(magnet.rads)+' '+magnet.magnet_label)
            annuli_count[magnet.magnet_label] += 1

    # file to report flags regarding special cases of hexabundle allocation
    flag_magnetCount = f'{self.allocation_files_location_base}/MagnetCount_warnings.txt'

    for i in annuli_count:

        if annuli_count[i] > annuli_count_magnetCasing[i]:
            # write warnings on file with tile numbers and particular annuli
            with open(flag_magnetCount, 'a') as fp:
                conflict = str(self.config['output_filename_stem']) + ', Tiles ' + str(tile_number_1) + ' and ' + \
                           str(tile_number_2) + ' have more magnets in annulus ' + str(i) + ':' + str(annuli_count[i]) \
                           + ' than available number of casings, with extra '\
                           + str(annuli_count[i] - annuli_count_magnetCasing[i]) + ' magnet/s. \n'
                fp.write(conflict)
                print(conflict)

        # add magnet count in each tile pair to the whole batch annuliCount dictionary
        annuliCount_batch[i] += [annuli_count[i]]

    return annuliCount_batch


def check_tile_pair_magnet_counts(robot_file_1, robot_file_2):

    magnet_colours = ['Blu', 'Gre', 'Yel', 'Mag']
    long_magnet_names = {'Blu' : 'Blue', 'Yel':'Yellow', 'Gre':'Green', 'Mag':'Magenta'}
    df_1 = pd.read_csv(robot_file_1, skiprows=6)
    df_2 = pd.read_csv(robot_file_2, skiprows=6)


    circular_magnets_1 = df_1.loc[df_1["#Magnet"] == "circular_magnet"]
    circular_magnets_2 = df_2.loc[df_2["#Magnet"] == "circular_magnet"]

    total_magnet_counts = {'Blu': 25, 'Gre': 27, 'Yel':31, 'Mag': 24}

    tile_change_passes = True
    failing_colours = []
    for magnet_colour in magnet_colours:

        tile_1_magnets = circular_magnets_1.loc[circular_magnets_1['Label'] == magnet_colour]
        tile_2_magnets = circular_magnets_2.loc[circular_magnets_2['Label'] == magnet_colour]

        N_tile_1 = len(tile_1_magnets)
        N_tile_2 = len(tile_2_magnets)
        N_total = N_tile_1 + N_tile_2
        print(f"Telecentricity Annulus: {long_magnet_names[magnet_colour]}")
        print(f"\tTile 1 has {N_tile_1}, Tile 2 has {N_tile_2}")
        print(f"\tTotal used: {N_tile_1 + N_tile_2} / {total_magnet_counts[magnet_colour]}")


        if N_total > total_magnet_counts[magnet_colour]:
            print(f"\t\t!!These two tiles use more {long_magnet_names[magnet_colour]} magnets than we have available!!")
            tile_change_passes = False
            failing_colours.append(magnet_colour)

    if not tile_change_passes:
        raise ValueError(f"\nThis combination of tiles doesn't work! It uses too many of the following magnet colours: {', '.join([long_magnet_names[f] for f in failing_colours])}")


# plot histogram for whole batch magnet count per annulus of tile pairs
def plotHist_annuliCount_batch(self, annuliCount_batch, tile_batch, tileBatch_count):

    # histogram colour for each annulus based on its name
    hist_colour = {'Blu':"blue", 'Gre':"green", 'Yel':"yellow", 'Mag':"Magenta"}

    for i in annuliCount_batch:

        # count of magnets in each pair of tiles for whole batch read as data from each of the annulus
        S = annuliCount_batch[i]

        # histogram plot file name
        magnetCount_barPlot = f"{self.plot_location}/magnetCount_barPlot_{self.config['output_filename_stem']}_{i}.pdf"

        plt.figure(9)
        plt.clf()

        # plotting histogram and saving the figure as pdf
        plt.style.use('ggplot')
        plt.hist(S, bins=range(1,30), weights=np.ones_like(S)/len(S), color=hist_colour[i], alpha=0.5, histtype='bar', ec='black')
        plt.gcf().set_size_inches(15,10)
        plt.suptitle('Batch '+str(tile_batch)+' : '+str(tileBatch_count+1)+' tiles, '+str(len(annuliCount_batch['Blu']))+' pairs -- '+str(hist_colour[i]))
        plt.xlabel('Magnet Count for each tile pair', fontsize=20)
        plt.ylabel('Distribution weights in batch (sums to total of 1.0)', fontsize=20)
        plt.savefig(magnetCount_barPlot, dpi=200)



def show_sky_fibre_changes(tile_1, tile_2):

    """
    Make a plot to show the changes in sky fibres between two plates.
    Inputs:
        tile_1 (str): Filename of the Hector Tile File (called Tile_FinalFormat...) file for plate 1
        tile_2 (str): Filename of the Hector Tile File (called Tile_FinalFormat...) file for plate 2
    Output:
        (fig, ax)
    """
    tile_1 = Path(tile_1)
    tile_2 = Path(tile_2)

    assert tile_1.stem.startswith("Tile_FinalFormat"), f"Tile 1 must be a Hector robot tile file which starts with 'Tile_FinalFormat'. Currently it is {tile_1.stem}" 
    assert tile_2.stem.startswith("Tile_FinalFormat"), f"Tile 2 must be a Hector robot tile file which starts with 'Tile_FinalFormat'. Currently it is {tile_2.stem}" 

    skyfibreDict_tile1 = read_sky_fibre_file(tile_1)
    skyfibreDict_tile2 = read_sky_fibre_file(tile_2)

    fig, ax = plt.subplots(constrained_layout=True)
    # Draw the circular telecentricity annulus
    ax = draw_circularSegments(ax)

    # sky_fibre_annotations(fig, tile_1)
    sky_fibre_colours = []
    for j, (key1, key2) in enumerate(zip(skyfibreDict_tile1, skyfibreDict_tile2)):
        sky_fibre_colours.append([])
        for fibre1, fibre2 in zip(skyfibreDict_tile1[key1], skyfibreDict_tile2[key2]):
            if np.array_equal(list(fibre1.values()), list(fibre2.values())):
                sky_fibre_colours[j].append('black')
            else:
                sky_fibre_colours[j].append('yellow')

    angle_subplate = [7,5,3,1,-1,-3,-5,-7]
    radius = 270

    #sky fibres top batch
    skyfibreTitles_top = ['H3','A3','H4','A4']
    skyfibreTitles_left = ['A1', 'H1', 'H2', 'A2']
    skyfibreTitles_right = ['H7', 'A5', 'H6', 'H5']

    ax = plot_skyfibre_section(fig, ax, skyfibreDict_tile2, angle=30, radius=radius, angle_subplate=angle_subplate, skyfibre_titles=skyfibreTitles_top, delta_ang=20, colour_array=sky_fibre_colours)
    ax = plot_skyfibre_section(fig, ax, skyfibreDict_tile2, angle=150, radius=radius, angle_subplate=angle_subplate, skyfibre_titles=skyfibreTitles_left, delta_ang=20, colour_array=sky_fibre_colours)
    ax = plot_skyfibre_section(fig, ax, skyfibreDict_tile2, angle=-160, radius=radius, angle_subplate=angle_subplate, skyfibre_titles=skyfibreTitles_right, delta_ang=-20, colour_array=sky_fibre_colours)

    return (fig, ax)

# PRODUCING PLOT FOR THE SECOND TILE BASED ON CHANGES IN SKYFIBRE SUB-PLATE NUMBERS COMPARED TO FIRST TILE
def createHexabundleFigure_withChangeShown(self, tile_number_1, tile_number_2, subplateSkyfibre_figureFile_tile1, subplateSkyfibre_figureFile_tile2):

    tile_1_hexa = f"{self.configuration_location}/HECTORConfig_Hexa_{self.config['output_filename_stem']}_{tile_number_1:03d}.txt"
    tile_2_hexa = f"{self.configuration_location}/HECTORConfig_Hexa_{self.config['output_filename_stem']}_{(tile_number_2):03d}.txt"

    plt.figure(7)
    plt.clf()

    skyfibreDict_tile1 = read_sky_fibre_file(tile_1_hexa)
    skyfibreDict_tile2 = read_sky_fibre_file(tile_2_hexa)

    draw_circularSegments()

    sky_fibre_annotations(tile_1_hexa)


    plt.axis('off')
    axes = plt.gca()
    axes.set_xlim([-350, 350])
    axes.set_ylim([-350, 350])
    # plt.show()                ## for showing the figure of magnets with pickup area
    plt.gcf().set_size_inches(6, 6)
    plt.savefig(subplateSkyfibre_figureFile_tile1)

    plt.figure(8)
    plt.clf()

    # skyfibreDict_tile1 = read_sky_fibre_file(tile_1)
    # skyfibreDict_tile2 = read_sky_fibre_file(tile_2)

    draw_circularSegments()

    skyfibreDict = skyfibreDict_tile2
    angle_subplate = [7, 5, 3, 1, -1, -3, -5, -7]
    radii = 270

    # filename = 'Sky_fibre_position_example.csv'
    # skyfibreDict = read_sky_fibre_file(skyfibre_file)
    # print(skyfibreDict)
    string = str(skyfibreDict['H3'][5].keys())


    # sky fibres top batch
    angle = 30
    skyfibreTitles_top = ['H3', 'A3', 'H4', 'A4']
    for i in range(0, 4):
        x, y, rotation = coordinates_and_angle_of_skyFibres(angle, radii)
        plt.annotate(skyfibreTitles_top[i], (x, y), color='black', rotation=rotation, fontsize=11, ha='center', va='center')
        for j in range(len(skyfibreDict[skyfibreTitles_top[i]])):
            if skyfibreDict[skyfibreTitles_top[i]][j][j + 1] == skyfibreDict_tile1[skyfibreTitles_top[i]][j][j + 1]:
                colour1 = colour2 = 'black'
            else:
                colour1 = 'orange'
                colour2 = 'yellow'
            angle_pos = angle + angle_subplate[j]
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 342)
            fibre_num = re.sub('[^0-9]', '', str(skyfibreDict[skyfibreTitles_top[i]][j].keys()))
            plt.annotate(fibre_num, (x, y), color=colour1, rotation=rotation, fontsize=5, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 308)
            plt.annotate('▮', (x, y), color=colour2, rotation=rotation, fontsize=7, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 322)
            plt.annotate(str(skyfibreDict[skyfibreTitles_top[i]][j][j + 1]), (x, y), color=colour2, rotation=rotation,
                         fontsize=6, weight='bold', ha='center', va='center')
        if skyfibreTitles_top[i][0] == 'H':
            alpha = 0.4
        elif skyfibreTitles_top[i][0] == 'A':
            alpha = 0.7
        draw_wedge = patches.Wedge((0, 0), r=333, theta1=angle - 9 + 90, theta2=angle + 9 + 90, width=90,
                                   facecolor='gray', edgecolor='black', alpha=alpha)
        plt.gcf().gca().add_artist(draw_wedge)
        angle = angle - 20

    # sky fibres left batch
    angle = 150
    skyfibreTitles_left = ['A1', 'H1', 'H2', 'A2']
    for i in range(0, 4):
        x, y, rotation = coordinates_and_angle_of_skyFibres(angle, radii)
        plt.annotate(skyfibreTitles_left[i], (x, y), color='black', rotation=rotation, fontsize=11, ha='center',
                     va='center')
        for j in range(len(skyfibreDict[skyfibreTitles_left[i]])):
            if skyfibreDict[skyfibreTitles_left[i]][j][j + 1] == skyfibreDict_tile1[skyfibreTitles_left[i]][j][j + 1]:
                colour1 = colour2 = 'black'
            else:
                colour1 = 'orange'
                colour2 = 'yellow'
            angle_pos = angle + angle_subplate[j]
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 342)
            fibre_num = re.sub('[^0-9]', '', str(skyfibreDict[skyfibreTitles_left[i]][j].keys()))
            plt.annotate(fibre_num, (x, y), color=colour1, rotation=rotation, fontsize=5, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 308)
            plt.annotate('▮', (x, y), color=colour2, rotation=rotation, fontsize=7, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 322)
            plt.annotate(str(skyfibreDict[skyfibreTitles_left[i]][j][j + 1]), (x, y), color=colour2, rotation=rotation,
                         fontsize=6, weight='bold', ha='center', va='center')
        if skyfibreTitles_left[i][0] == 'H':
            alpha = 0.4
        elif skyfibreTitles_left[i][0] == 'A':
            alpha = 0.7
        draw_wedge = patches.Wedge((0, 0), r=333, theta1=angle - 9 + 90, theta2=angle + 9 + 90, width=90,
                                   facecolor='gray', edgecolor='black', alpha=alpha)
        plt.gcf().gca().add_artist(draw_wedge)
        angle = angle - 20

    # sky fibres right batch
    angle = -160
    skyfibreTitles_right = ['H7', 'A5', 'H6', 'H5']
    for i in range(0, 4):
        x, y, rotation = coordinates_and_angle_of_skyFibres(angle, radii)
        plt.annotate(skyfibreTitles_right[i], (x, y), color='black', rotation=rotation, fontsize=11, ha='center',
                     va='center')
        for j in range(len(skyfibreDict[skyfibreTitles_right[i]])):
            if skyfibreDict[skyfibreTitles_right[i]][j][j + 1] == skyfibreDict_tile1[skyfibreTitles_right[i]][j][j + 1]:
                colour1 = colour2 = 'black'
            else:
                colour1 = 'orange'
                colour2 = 'yellow'
            angle_pos = angle + angle_subplate[j]
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 342)
            fibre_num = re.sub('[^0-9]', '', str(skyfibreDict[skyfibreTitles_right[i]][j].keys()))
            plt.annotate(fibre_num, (x, y), color=colour1, rotation=rotation, fontsize=5, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 308)
            plt.annotate('▮', (x, y), color=colour2, rotation=rotation, fontsize=7, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 322)
            plt.annotate(str(skyfibreDict[skyfibreTitles_right[i]][j][j + 1]), (x, y), color=colour2, rotation=rotation,
                         fontsize=6, weight='bold', ha='center', va='center')
        if skyfibreTitles_right[i][0] == 'H':
            alpha = 0.4
        elif skyfibreTitles_right[i][0] == 'A':
            alpha = 0.7
        draw_wedge = patches.Wedge((0, 0), r=333, theta1=angle - 9 + 90, theta2=angle + 9 + 90, width=90,
                                   facecolor='gray', edgecolor='black', alpha=alpha)
        plt.gcf().gca().add_artist(draw_wedge)
        angle = angle + 20


    plt.axis('off')
    axes = plt.gca()
    axes.set_xlim([-350, 350])
    axes.set_ylim([-350, 350])
    # plt.show()                ## for showing the figure of magnets with pickup area
    plt.gcf().set_size_inches(6, 6)
    plt.savefig(subplateSkyfibre_figureFile_tile2)


# PRODUCING PLOT FOR THE SECOND TILE BASED ON CHANGES IN SKYFIBRE SUB-PLATE NUMBERS COMPARED TO FIRST TILE
def createskyfibreChanges_plot(self, tile_1_hexa, tile_2_hexa, subplateSkyfibre_figureFile_tile1, subplateSkyfibre_figureFile_tile2, subplateSkyfibre_figureFile):

    # tile_1_hexa = f"{self.configuration_location}/HECTORConfig_Hexa_{self.config['output_filename_stem']}_{tile_number_1:03d}.txt"
    # tile_2_hexa = f"{self.configuration_location}/HECTORConfig_Hexa_{self.config['output_filename_stem']}_{(tile_number_2):03d}.txt"

    pistonChange_count = 0

    plt.figure(7)
    plt.clf()

    skyfibreDict_tile1 = read_sky_fibre_file(tile_1_hexa)
    skyfibreDict_tile2 = read_sky_fibre_file(tile_2_hexa)
    #print(skyfibreDict_tile1)

    draw_circularSegments()

    ##  sky_fibre_annotations function slightly edited to makr positions 2 & 3 in yellow
    skyfibreDict = skyfibreDict_tile1
    angle_subplate = [7, 5, 3, 1, -1, -3, -5, -7]
    radii = 270
    #print(skyfibreDict)

    # sky fibres top batch
    angle = 30
    skyfibreTitles_top = ['H3', 'A3', 'H4', 'A4']
    for i in range(0, 4):
        x, y, rotation = coordinates_and_angle_of_skyFibres(angle, radii)
        plt.annotate(skyfibreTitles_top[i], (x, y), color='black', rotation=rotation, fontsize=11, ha='center',
                     va='center')
        #print(range(len(skyfibreDict[skyfibreTitles_top[i]])))
        for j in range(0, len(skyfibreDict[skyfibreTitles_top[i]])):
            if int(skyfibreDict[skyfibreTitles_top[i]][j][j + 1]) in [2,3]:
                pistonChange_count += 1
                colour1 = 'orange'
                colour2 = 'yellow'
            else:
                colour1 = colour2 = 'black'
            angle_pos = angle + angle_subplate[j]
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 342)
            fibre_num = re.sub('[^0-9]', '', str(skyfibreDict[skyfibreTitles_top[i]][j].keys()))
            plt.annotate(fibre_num, (x, y), color=colour1, rotation=rotation, fontsize=5, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 308)
            plt.annotate('▮', (x, y), color=colour2, rotation=rotation, fontsize=7, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 322)
            plt.annotate(str(skyfibreDict[skyfibreTitles_top[i]][j][j + 1]), (x, y), color=colour2, rotation=rotation,
                         fontsize=6, weight='bold', ha='center', va='center')
        if skyfibreTitles_top[i][0] == 'H':
            alpha = 0.4
        elif skyfibreTitles_top[i][0] == 'A':
            alpha = 0.7
        draw_wedge = patches.Wedge((0, 0), r=333, theta1=angle - 9 + 90, theta2=angle + 9 + 90, width=90,
                                   facecolor='gray', edgecolor='black', alpha=alpha)
        plt.gcf().gca().add_artist(draw_wedge)
        angle = angle - 20

    # sky fibres left batch
    angle = 150
    skyfibreTitles_left = ['A1', 'H1', 'H2', 'A2']
    for i in range(0, 4):
        x, y, rotation = coordinates_and_angle_of_skyFibres(angle, radii)
        plt.annotate(skyfibreTitles_left[i], (x, y), color='black', rotation=rotation, fontsize=11, ha='center',va='center')
        for j in range(0, len(skyfibreDict[skyfibreTitles_left[i]])):
            if int(skyfibreDict[skyfibreTitles_left[i]][j][j + 1]) in [2,3]:
                pistonChange_count += 1
                colour1 = 'orange'
                colour2 = 'yellow'
            else:
                colour1 = colour2 = 'black'
            angle_pos = angle + angle_subplate[j]
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 342)
            fibre_num = re.sub('[^0-9]', '', str(skyfibreDict[skyfibreTitles_left[i]][j].keys()))
            plt.annotate(fibre_num, (x, y), color=colour1, rotation=rotation, fontsize=5, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 308)
            plt.annotate('▮', (x, y), color=colour2, rotation=rotation, fontsize=7, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 322)
            plt.annotate(str(skyfibreDict[skyfibreTitles_left[i]][j][j + 1]), (x, y), color=colour2, rotation=rotation,
                         fontsize=6, weight='bold', ha='center', va='center')
        if skyfibreTitles_left[i][0] == 'H':
            alpha = 0.4
        elif skyfibreTitles_left[i][0] == 'A':
            alpha = 0.7
        draw_wedge = patches.Wedge((0, 0), r=333, theta1=angle - 9 + 90, theta2=angle + 9 + 90, width=90,
                                   facecolor='gray', edgecolor='black', alpha=alpha)
        plt.gcf().gca().add_artist(draw_wedge)
        angle = angle - 20

    # sky fibres right batch
    angle = -160
    skyfibreTitles_right = ['H7', 'A5', 'H6', 'H5']
    for i in range(0, 4):
        x, y, rotation = coordinates_and_angle_of_skyFibres(angle, radii)
        plt.annotate(skyfibreTitles_right[i], (x, y), color='black', rotation=rotation, fontsize=11, ha='center',
                     va='center')
        for j in range(0, len(skyfibreDict[skyfibreTitles_right[i]])):
            if int(skyfibreDict[skyfibreTitles_right[i]][j][j + 1]) in [2,3]:
                pistonChange_count += 1
                colour1 = 'orange'
                colour2 = 'yellow'
            else:
                colour1 = colour2 = 'black'
            angle_pos = angle + angle_subplate[j]
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 342)
            fibre_num = re.sub('[^0-9]', '', str(skyfibreDict[skyfibreTitles_right[i]][j].keys()))
            plt.annotate(fibre_num, (x, y), color=colour1, rotation=rotation, fontsize=5, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 308)
            plt.annotate('▮', (x, y), color=colour2, rotation=rotation, fontsize=7, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 322)
            plt.annotate(str(skyfibreDict[skyfibreTitles_right[i]][j][j + 1]), (x, y), color=colour2, rotation=rotation,
                         fontsize=6, weight='bold', ha='center', va='center')
        if skyfibreTitles_right[i][0] == 'H':
            alpha = 0.4
        elif skyfibreTitles_right[i][0] == 'A':
            alpha = 0.7
        draw_wedge = patches.Wedge((0, 0), r=333, theta1=angle - 9 + 90, theta2=angle + 9 + 90, width=90,
                                   facecolor='gray', edgecolor='black', alpha=alpha)
        plt.gcf().gca().add_artist(draw_wedge)
        angle = angle + 20

    plt.axis('off')
    plt.title("Original",fontsize=12)
    axes = plt.gca()
    axes.set_xlim([-350, 350])
    axes.set_ylim([-350, 350])
    # plt.show()                ## for showing the figure of magnets with pickup area
    plt.gcf().set_size_inches(6, 6)
    plt.savefig(subplateSkyfibre_figureFile_tile1, dpi=1200)

    plt.figure(8)
    plt.clf()

    # skyfibreDict_tile1 = read_sky_fibre_file(tile_1)
    # skyfibreDict_tile2 = read_sky_fibre_file(tile_2)

    draw_circularSegments()

    skyfibreDict = skyfibreDict_tile2
    angle_subplate = [7, 5, 3, 1, -1, -3, -5, -7]
    radii = 270

    # filename = 'Sky_fibre_position_example.csv'
    # skyfibreDict = read_sky_fibre_file(skyfibre_file)
    # print(skyfibreDict)
    string = str(skyfibreDict['H3'][5].keys())

    # sky fibres top batch
    angle = 30
    skyfibreTitles_top = ['H3', 'A3', 'H4', 'A4']
    for i in range(0, 4):
        x, y, rotation = coordinates_and_angle_of_skyFibres(angle, radii)
        plt.annotate(skyfibreTitles_top[i], (x, y), color='black', rotation=rotation, fontsize=11, ha='center', va='center')
        for j in range(len(skyfibreDict[skyfibreTitles_top[i]])):
            if skyfibreDict[skyfibreTitles_top[i]][j][j + 1] in [2,3]:
                pistonChange_count += 1
                colour1 = colour2 = 'red'
            elif skyfibreDict[skyfibreTitles_top[i]][j][j + 1] == 0 and skyfibreDict_tile1[skyfibreTitles_top[i]][j][j + 1] != 0:
                pistonChange_count += 1
                colour1 = colour2 = 'blue'
            elif skyfibreDict[skyfibreTitles_top[i]][j][j + 1] == 1 and skyfibreDict_tile1[skyfibreTitles_top[i]][j][j + 1] == 0:
                pistonChange_count += 1
                colour1 = colour2 = 'lime'
            else:
                colour1 = colour2 = 'black'
            angle_pos = angle + angle_subplate[j]
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 342)
            fibre_num = re.sub('[^0-9]', '', str(skyfibreDict[skyfibreTitles_top[i]][j].keys()))
            plt.annotate(fibre_num, (x, y), color=colour1, rotation=rotation, fontsize=5, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 308)
            plt.annotate('▮', (x, y), color=colour2, rotation=rotation, fontsize=7, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 322)
            plt.annotate(str(skyfibreDict[skyfibreTitles_top[i]][j][j + 1]), (x, y), color=colour2, rotation=rotation,
                         fontsize=6, weight='bold', ha='center', va='center')
        if skyfibreTitles_top[i][0] == 'H':
            alpha = 0.4
        elif skyfibreTitles_top[i][0] == 'A':
            alpha = 0.7
        draw_wedge = patches.Wedge((0, 0), r=333, theta1=angle - 9 + 90, theta2=angle + 9 + 90, width=90,
                                   facecolor='gray', edgecolor='black', alpha=alpha)
        plt.gcf().gca().add_artist(draw_wedge)
        angle = angle - 20

    # sky fibres left batch
    angle = 150
    skyfibreTitles_left = ['A1', 'H1', 'H2', 'A2']
    for i in range(0, 4):
        x, y, rotation = coordinates_and_angle_of_skyFibres(angle, radii)
        plt.annotate(skyfibreTitles_left[i], (x, y), color='black', rotation=rotation, fontsize=11, ha='center',
                     va='center')
        for j in range(len(skyfibreDict[skyfibreTitles_left[i]])):
            if skyfibreDict[skyfibreTitles_left[i]][j][j + 1] in [2, 3]:
                pistonChange_count += 1
                colour1 = colour2 = 'red'
            elif skyfibreDict[skyfibreTitles_left[i]][j][j + 1] == 0 and skyfibreDict_tile1[skyfibreTitles_left[i]][j][j + 1] != 0:
                pistonChange_count += 1
                colour1 = colour2 = 'blue'
            elif skyfibreDict[skyfibreTitles_left[i]][j][j + 1] == 1 and skyfibreDict_tile1[skyfibreTitles_left[i]][j][j + 1] == 0:
                pistonChange_count += 1
                colour1 = colour2 = 'lime'
            else:
                colour1 = colour2 = 'black'
            angle_pos = angle + angle_subplate[j]
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 342)
            fibre_num = re.sub('[^0-9]', '', str(skyfibreDict[skyfibreTitles_left[i]][j].keys()))
            plt.annotate(fibre_num, (x, y), color=colour1, rotation=rotation, fontsize=5, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 308)
            plt.annotate('▮', (x, y), color=colour2, rotation=rotation, fontsize=7, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 322)
            plt.annotate(str(skyfibreDict[skyfibreTitles_left[i]][j][j + 1]), (x, y), color=colour2, rotation=rotation,
                         fontsize=6, weight='bold', ha='center', va='center')
        if skyfibreTitles_left[i][0] == 'H':
            alpha = 0.4
        elif skyfibreTitles_left[i][0] == 'A':
            alpha = 0.7
        draw_wedge = patches.Wedge((0, 0), r=333, theta1=angle - 9 + 90, theta2=angle + 9 + 90, width=90,
                                   facecolor='gray', edgecolor='black', alpha=alpha)
        plt.gcf().gca().add_artist(draw_wedge)
        angle = angle - 20

    # sky fibres right batch
    angle = -160
    skyfibreTitles_right = ['H7', 'A5', 'H6', 'H5']
    for i in range(0, 4):
        x, y, rotation = coordinates_and_angle_of_skyFibres(angle, radii)
        plt.annotate(skyfibreTitles_right[i], (x, y), color='black', rotation=rotation, fontsize=11, ha='center',
                     va='center')
        for j in range(len(skyfibreDict[skyfibreTitles_right[i]])):
            if skyfibreDict[skyfibreTitles_right[i]][j][j + 1] in [2, 3]:
                pistonChange_count += 1
                colour1 = colour2 = 'red'
            elif skyfibreDict[skyfibreTitles_right[i]][j][j + 1] == 0 and skyfibreDict_tile1[skyfibreTitles_right[i]][j][j + 1] != 0:
                pistonChange_count += 1
                colour1 = colour2 = 'blue'
            elif skyfibreDict[skyfibreTitles_right[i]][j][j + 1] == 1 and skyfibreDict_tile1[skyfibreTitles_right[i]][j][j + 1] == 0:
                pistonChange_count += 1
                colour1 = colour2 = 'lime'
            else:
                colour1 = colour2 = 'black'
            angle_pos = angle + angle_subplate[j]
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 342)
            fibre_num = re.sub('[^0-9]', '', str(skyfibreDict[skyfibreTitles_right[i]][j].keys()))
            plt.annotate(fibre_num, (x, y), color=colour1, rotation=rotation, fontsize=5, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 308)
            plt.annotate('▮', (x, y), color=colour2, rotation=rotation, fontsize=7, ha='center', va='center')
            x, y, rotation = coordinates_and_angle_of_skyFibres(angle_pos, 322)
            plt.annotate(str(skyfibreDict[skyfibreTitles_right[i]][j][j + 1]), (x, y), color=colour2, rotation=rotation,
                         fontsize=6, weight='bold', ha='center', va='center')
        if skyfibreTitles_right[i][0] == 'H':
            alpha = 0.4
        elif skyfibreTitles_right[i][0] == 'A':
            alpha = 0.7
        draw_wedge = patches.Wedge((0, 0), r=333, theta1=angle - 9 + 90, theta2=angle + 9 + 90, width=90,
                                   facecolor='gray', edgecolor='black', alpha=alpha)
        plt.gcf().gca().add_artist(draw_wedge)
        angle = angle + 20

    plt.axis('off')
    plt.title("New",fontsize=12)
    axes = plt.gca()
    axes.set_xlim([-350, 350])
    axes.set_ylim([-350, 350])
    # plt.show()                ## for showing the figure of magnets with pickup area
    plt.gcf().set_size_inches(6, 6)
    plt.savefig(subplateSkyfibre_figureFile_tile2, dpi=1200)

    # read the images
    im1 = plt.imread(subplateSkyfibre_figureFile_tile1)
    im2 = plt.imread(subplateSkyfibre_figureFile_tile2)

    # horizontally concatenates images of same height
    fig, axs = plt.subplots(ncols=2, figsize=(15, 8))
    axs[0].imshow(im1)
    axs[1].imshow(im2)
    #im_h = cv2.hconcat([img1, img2])
    for ax in axs:
        ax.axis('off')


    # show the output image
    # cv2.imshow('man_image.jpeg', im_h)
    fig.savefig(subplateSkyfibre_figureFile, bbox_inches='tight')

    return pistonChange_count


# PRODUCING PLOT FOR THE SECOND TILE BASED ON CHANGES IN SKYFIBRE SUB-PLATE NUMBERS COMPARED TO FIRST TILE
def skyfibreChanges_pistonChange_count(self, tile_number_1, tile_number_2):

    tile_1_hexa = f"{self.configuration_location}/HECTORConfig_Hexa_{self.config['output_filename_stem']}_{tile_number_1:03d}.txt"
    tile_2_hexa = f"{self.configuration_location}/HECTORConfig_Hexa_{self.config['output_filename_stem']}_{(tile_number_2):03d}.txt"

    pistonChange_count = 0

    plt.figure(7)
    plt.clf()

    skyfibreDict_tile1 = read_sky_fibre_file(tile_1_hexa)
    skyfibreDict_tile2 = read_sky_fibre_file(tile_2_hexa)
    #print(skyfibreDict_tile1)

    ##  sky_fibre_annotations function slightly edited to makr positions 2 & 3 in yellow
    skyfibreDict = skyfibreDict_tile1

    skyfibreTitles_top = ['H3', 'A3', 'H4', 'A4']
    for i in range(0, 4):
        for j in range(0, len(skyfibreDict[skyfibreTitles_top[i]])):
            if int(skyfibreDict[skyfibreTitles_top[i]][j][j + 1]) in [2,3]:
                pistonChange_count += 1

    # sky fibres left batch
    skyfibreTitles_left = ['A1', 'H1', 'H2', 'A2']
    for i in range(0, 4):
        for j in range(0, len(skyfibreDict[skyfibreTitles_left[i]])):
            if int(skyfibreDict[skyfibreTitles_left[i]][j][j + 1]) in [2,3]:
                pistonChange_count += 1

    # sky fibres right batch
    skyfibreTitles_right = ['H7', 'A5', 'H6', 'H5']
    for i in range(0, 4):
        for j in range(0, len(skyfibreDict[skyfibreTitles_right[i]])):
            if int(skyfibreDict[skyfibreTitles_right[i]][j][j + 1]) in [2,3]:
                pistonChange_count += 1


    skyfibreDict = skyfibreDict_tile2

    # sky fibres top batch
    skyfibreTitles_top = ['H3', 'A3', 'H4', 'A4']
    for i in range(0, 4):
        for j in range(len(skyfibreDict[skyfibreTitles_top[i]])):
            if skyfibreDict[skyfibreTitles_top[i]][j][j + 1] in [2,3]:
                pistonChange_count += 1
            elif skyfibreDict[skyfibreTitles_top[i]][j][j + 1] == 0 and skyfibreDict_tile1[skyfibreTitles_top[i]][j][j + 1] != 0:
                pistonChange_count += 1
            elif skyfibreDict[skyfibreTitles_top[i]][j][j + 1] == 1 and skyfibreDict_tile1[skyfibreTitles_top[i]][j][j + 1] == 0:
                pistonChange_count += 1

    # sky fibres left batch
    skyfibreTitles_left = ['A1', 'H1', 'H2', 'A2']
    for i in range(0, 4):
        for j in range(len(skyfibreDict[skyfibreTitles_left[i]])):
            if skyfibreDict[skyfibreTitles_left[i]][j][j + 1] in [2, 3]:
                pistonChange_count += 1
            elif skyfibreDict[skyfibreTitles_left[i]][j][j + 1] == 0 and skyfibreDict_tile1[skyfibreTitles_left[i]][j][j + 1] != 0:
                pistonChange_count += 1
            elif skyfibreDict[skyfibreTitles_left[i]][j][j + 1] == 1 and skyfibreDict_tile1[skyfibreTitles_left[i]][j][j + 1] == 0:
                pistonChange_count += 1

    # sky fibres right batch
    skyfibreTitles_right = ['H7', 'A5', 'H6', 'H5']
    for i in range(0, 4):
        for j in range(len(skyfibreDict[skyfibreTitles_right[i]])):
            if skyfibreDict[skyfibreTitles_right[i]][j][j + 1] in [2, 3]:
                pistonChange_count += 1
            elif skyfibreDict[skyfibreTitles_right[i]][j][j + 1] == 0 and skyfibreDict_tile1[skyfibreTitles_right[i]][j][j + 1] != 0:
                pistonChange_count += 1
            elif skyfibreDict[skyfibreTitles_right[i]][j][j + 1] == 1 and skyfibreDict_tile1[skyfibreTitles_right[i]][j][j + 1] == 0:
                pistonChange_count += 1

    #print(pistonChange_count)
    return pistonChange_count


# plot histogram for whole batch piston change per tile pair
def plotHist_pistonChange_count_batch(self, pistonChange_countBatch, tile_batch, tileBatch_count):

    # count of piston change in each pair of tiles for whole batch
    S = pistonChange_countBatch
    #print(S)

    # histogram plot file name
    magnetCount_barPlot = f"{self.plot_location}/pistonChangeCount_barPlot_{self.config['output_filename_stem']}.pdf"

    plt.figure(10)
    plt.clf()

    # plotting histogram and saving the figure as pdf
    plt.style.use('ggplot')
    plt.hist(S, bins=range(5,50), weights=np.ones_like(S)/len(S), color='orange', alpha=0.5, histtype='bar', ec='black')
    plt.gcf().set_size_inches(15,10)
    plt.suptitle('Batch '+str(tile_batch)+' : '+str(tileBatch_count+1)+' tiles, pairs -- '+str(len(pistonChange_countBatch)))
    plt.xlabel('piston change Count for each tile pair', fontsize=20)
    plt.ylabel('Distribution weights in batch (sums to total of 1.0)', fontsize=20)
    plt.savefig(magnetCount_barPlot, dpi=200)

