import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import datetime
import string
import csv
import re

def extract_fibreInfo(fibre_file,all_magnets,robotFilearray,output_fibreAAOmega,output_fibreSpector):

    fibre_data = pd.read_excel(fibre_file)

    print(fibre_data)

    # Creates a list containing w lists, each of h item/s, all filled with 0
    w, h1, h2 = 10, 820, 856
    new_arrayAAOmega = [['0' for x in range(w)] for y in range(h1)]
    new_arraySpector = [['0' for x in range(w)] for y in range(h2)]
    # print(len(new_array))

    # headers for the spectograph fibres file
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

    print(new_arrayAAOmega)
    print(new_arraySpector)

    # for i in all_magnets:
    #     for j in robotFilearray[1:]:
    #         if i.__class__.__name__ == 'rectangular_magnet' and i.index == int(j[11]):
    #             for k in new_arrayAAOmega:
    #                 if k[2] == j[12]:
    #
    #                     ang = i.orientation
    #                     while ang > 180:
    #                         ang = ang - 360
    #                     while ang < -180:
    #                         ang = ang + 360
    #
    #                     new_arrayAAOmega[k[0]][4] = ang
    #             for k in new_arraySpector:
    #                 if k[2] == j[12]:
    #
    #                     ang = i.orientation
    #                     while ang > 180:
    #                         ang = ang - 360
    #                     while ang < -180:
    #                         ang = ang + 360
    #
    #                     new_arraySpector[k[0]][4] = ang


    for i in range(2):
        if i == 0:
            output_fibreFile = output_fibreAAOmega
            fibre_array = new_arrayAAOmega
            string = 'AAOmega'
        elif i == 1:
            output_fibreFile = output_fibreSpector
            fibre_array = new_arraySpector
            string = 'Spector'

        # write the fibre file array into the CSV file for the fibre spectrograph
        with open(output_fibreFile, 'w+') as output_fibre:
            output_fibre.write('# Experimental implementation of Hector_fibres_')
            output_fibre.write(string+'.txt\n')
            output_fibre.write('# \n')
            output_fibre.write('#   Ayoan Sadman\n')
            output_fibre.write('#   ')
            output_fibre.write(str(datetime.datetime.now().strftime('%B %d %y'))+'\n')
            output_fibre.write('#   Generated by "@(#) $Id: ACMM:sami/samigenfile.C,v 1.17 21-Jan-2013 17:07:23+11 nlorente $"\n')
            output_fibre.write('# \n')
            # output_fibre.write('#   Probe orientation is the rotation of the hexabundle on sky between -180 to +180 degrees \n')
            # output_fibre.write('#   where 0 degrees is the orientation in which the cable end of the hexabundle ferule is \n')
            # output_fibre.write('#   pointing north on the field plate and the hexabundle end of the ferule is pointing South, \n')
            # output_fibre.write('#   and the table defining the centre positions of the fibre cores is oriented such that North \n')
            # output_fibre.write('#   is at the top, as imaged at input face of the prism. The image is flipped N-S through the \n')
            # output_fibre.write('#   prism and hence the north direction maps to the field-plate side of the hexabundle face. The \n')
            # output_fibre.write('#   positive angle rotation rotates the ferule tail towards the astronomical west on the field \n')
            # output_fibre.write('#   plate, which rotates the image on the hexabundle towards from north towards east.\n')
            # output_fibre.write('#   \n')
            output_fibre.write('# ')
            writer = csv.writer(output_fibre, delimiter=' ', lineterminator='\n')
            writer.writerows(fibre_array)

    # print(new_array)

    return fibre_data


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

        print(file_array)

        output_file = output_hexabundle_coordData + 'hexabundle_' + i + '.txt'

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



