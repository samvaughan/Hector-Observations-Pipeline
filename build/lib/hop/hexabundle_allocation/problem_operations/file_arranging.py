import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import csv
import time


def arrange_guidesFile(fileNameGuides,proxyGuideFile):

    ExtraCols = 1
    # f = open('galaxy_fields/newfile.txt', "w+")
    # f.close()
    df = pd.read_csv(fileNameGuides)
    for i in range(35):
        df['ColFillUp%d' % ExtraCols] = 'NaN'
        ExtraCols += 1

    df.to_csv(proxyGuideFile, index=False, sep=' ', quoting=csv.QUOTE_NONE, escapechar=' ')

    rows = [line.split(' ') for line in open(proxyGuideFile)]
    cols = list(zip(*rows))
    cols.insert(1, ['"1stCol"', 'NaN', 'NaN', 'NaN', 'NaN', 'NaN', 'NaN'])
    cols.insert(1, ['21', '22', '23', '24', '25', '26', '27'])
    cols.insert(1, ['"IDs"', 'NaN', 'NaN', 'NaN', 'NaN', 'NaN', 'NaN'])
    cols = cols[1:]
    rows = list(zip(*cols))

    with open(proxyGuideFile, 'w') as f:
        f.writelines([' '.join(row) for row in rows[1:]])

def merge_hexaAndGuides(fileNameHexa, proxyGuideFile, plate_file):

    with open(fileNameHexa) as fp:
        file1 = fp.read()
    with open(proxyGuideFile) as fp:
        file2 = fp.read()
    file1 += file2

    # with open('GAMA_'+batch+'/Output/Hexa_and_Guides_GAMA_'+batch+'_tile_%03d.txt' % (tileNum),'w+') as fp:
    with open(plate_file, 'w+') as fp:
        fp.write(file1)

    print("Filename being checked for conflicts:")
    print(fileNameHexa)

def create_robotFileArray(positioning_array,robotFile,newrow,fully_blocked_magnets_dictionary):

    positioning_array[:, 8] = [i if i != 'nan' else -999999 for i in positioning_array[:, 8]]
    for i in range(len(positioning_array[:, 8])):
        positioning_array[:, 8][i] = round(float(positioning_array[:, 8][i]))

    robotFilearray = sorted(positioning_array, key=lambda x: x[6])
    robotFilearray = np.insert(robotFilearray, 0, np.array(newrow), 0)

    # rePosition_col = ['0'] * len(positioning_array[:, 8])
    # Creates a list containing 5 lists, each of 8 items, all set to 0
    w, h = len(positioning_array[:, 8])+1,1
    rePosition_col = [['[0]' for x in range(w)] for y in range(h)]

    # rePosition_col = np.transpose(rePosition_col)
    print(len(positioning_array[:, 8]))
    print(len(rePosition_col))
    # positioning_array = np.hstack((positioning_array[:, :8], rePosition_col, positioning_array[:, 8:]))

    for each_magnet in fully_blocked_magnets_dictionary:
        for i in range(len(robotFilearray)):
            if (robotFilearray[i][0] + ' ' + str(robotFilearray[i][9])) == each_magnet:
                rePosition_col[0][i] = '['+str(fully_blocked_magnets_dictionary[each_magnet])+']'

    rePosition_col[0][0] = 'rePosition_magnets'
    rePosition_col = np.transpose(rePosition_col)
    print(rePosition_col)

    print(rePosition_col.shape)
    print(robotFilearray.shape)

    # print(np.append(robotFilearray, rePosition_col, axis=1))
    robotFilearray = np.hstack((robotFilearray[:, :8], rePosition_col, robotFilearray[:, 8:]))
    print(robotFilearray)

    # CSV file for the robot
    # with open('GAMA_'+batch+'/Output_for_Robot/Robot_GAMA_'+batch+'_tile_%03d.txt' % (tileNum),'w+') as robotFile:
    with open(robotFile, 'w+') as robotFile:
        writer = csv.writer(robotFile, delimiter=' ')
        writer.writerows(robotFilearray)

    return positioning_array,robotFilearray

def positioningArray_adjust_and_mergetoFile(positioning_array, plate_file, outputFile, newrow,newrow_circular):

    positioning_array_circular = np.vsplit(positioning_array, 2)[0]
    positioning_array_circular = np.insert(positioning_array_circular, 0, np.array(newrow_circular), 0)
    positioning_array_circular = np.delete(positioning_array_circular, [2, 3, 4, 5, 9, 10], 1)

    positioning_array = np.vsplit(positioning_array, 2)[1]
    positioning_array = np.insert(positioning_array, 0, np.array(newrow), 0)
    positioning_array = np.delete(positioning_array, [2, 3, 4, 5, 8], 1)

    index = 0
    # Open the input_file in read mode and output_file in write mode
    with open(plate_file, 'r') as read_obj, \
            open(outputFile, 'w+', newline='') as write_obj:
    # with open('GAMA_'+batch+'/Output/Hexa_and_Guides_GAMA_'+batch+'_tile_%03d.txt' % (tileNum), 'r') as read_obj, \
    #         open('GAMA_'+batch+'/Output_with_Positioning_array/Hexa_and_Guides_with_PositioningArray_GAMA_'+batch+'_tile_%03d.txt' % (tileNum), 'w+', newline='') as write_obj:
        # Create a csv.reader object from the input file object
        csv_reader = csv.reader(read_obj)
        # Create a csv.writer object from the output file object
        csv_writer = csv.writer(write_obj)
        # Read each row of the input csv file as list
        for row in csv_reader:
            roww_circular = np.array2string(positioning_array_circular[index], separator=' ',formatter={'str_kind': lambda x: x})
            roww = np.array2string(positioning_array[index], separator=' ', formatter={'str_kind': lambda x: x})
            # Append the default text in the row / list
            row.append(str(roww).replace('[', ' ').rstrip(']'))
            row.append(str(roww_circular).replace('[', ' ').rstrip(']'))
            # Add the updated row / list to the output file
            csv_writer.writerow(row)
            # index = index+32
            index += 1
            # open('galaxy_fields/output_1.txt', 'w').writelines([' '.join(row)])

    return positioning_array,positioning_array_circular

def finalFiles(outputFile, robotFile):

    df3 = pd.read_csv(outputFile, header=0)
    df3.to_csv(outputFile, index=False, sep=' ', quoting=csv.QUOTE_NONE, escapechar=' ')

    # df4 = pd.read_csv(robotFile, header=0, error_bad_lines=False)
    # df4.to_csv(robotFile, index=False, sep=' ', quoting=csv.QUOTE_NONE, escapechar=' ')

    # df3 = pd.read_csv('GAMA_'+batch+'/Output_with_Positioning_array/Hexa_and_Guides_with_PositioningArray_GAMA_'+batch+'_tile_%03d.txt' % (tileNum), header=0)
    # df3.to_csv('GAMA_'+batch+'/Output_with_Positioning_array/Hexa_and_Guides_with_PositioningArray_GAMA_'+batch+'_tile_%03d.txt' % (tileNum), \
    #             index=False, sep=' ', quoting=csv.QUOTE_NONE, escapechar=' ')
    # df4 = pd.read_csv('GAMA_' + batch + '/Output_for_Robot/Robot_GAMA_' + batch + '_tile_%03d.txt' % (tileNum),header=0)
    # df4.to_csv('GAMA_' + batch + '/Output_for_Robot/Robot_GAMA_' + batch + '_tile_%03d.txt' % (tileNum),\
    #            index=False, sep=' ', quoting=csv.QUOTE_NONE, escapechar=' ')

