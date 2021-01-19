import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import csv
import re
import time

# arranging the guides files in consistent format with hexas files
def arrange_guidesFile(fileNameGuides,proxyGuideFile):

    # index to keep count
    ExtraCols = 1

    # reading the guides cdv file
    df = pd.read_csv(fileNameGuides)

    # The columns being filled up with NaN to keep the guides file same format as the hexas file
    for i in range(35):
        df['ColFillUp%d' % ExtraCols] = 'NaN'
        ExtraCols += 1

    # writing to the proxy guides file
    df.to_csv(proxyGuideFile, index=False, sep=' ', quoting=csv.QUOTE_NONE, escapechar=' ')

    # adding in two more columns at the start of the proxy file for the IDs and the magnet indexes
    rows = [line.split(' ') for line in open(proxyGuideFile)]
    cols = list(zip(*rows))
    cols.insert(1, ['"1stCol"', 'NaN', 'NaN', 'NaN', 'NaN', 'NaN', 'NaN'])
    cols.insert(1, ['21', '22', '23', '24', '25', '26', '27'])
    cols.insert(1, ['"IDs"', 'NaN', 'NaN', 'NaN', 'NaN', 'NaN', 'NaN'])
    cols = cols[1:]
    rows = list(zip(*cols))

    # writing those two columns created above to the file
    with open(proxyGuideFile, 'w') as f:
        f.writelines([' '.join(row) for row in rows[1:]])

# merging the hexas and guides file to create one plate file with all the magnets
def merge_hexaAndGuides(fileNameHexa, proxyGuideFile, plate_file):

    # adding the proxy guides file to the hexas file
    with open(fileNameHexa) as fp:
        file1 = fp.read()
    with open(proxyGuideFile) as fp:
        file2 = fp.read()
    file1 += file2

    # creating a plate file to write the added hexas and guides
    with open(plate_file, 'w+') as fp:
        fp.write(file1)

    # print statement to check from terminal the plate file of the tile which is being checked
    print("Filename being checked for conflicts:")
    print(fileNameHexa)

# creating the robotFile array for
def create_robotFileArray(positioning_array,robotFile,newrow,fully_blocked_magnets_dictionary):

    # guide probes do not have ID, so they are allocated a large negative integer
    positioning_array[:, 8] = [i if i != 'nan' else -999999 for i in positioning_array[:, 8]]
    for i in range(len(positioning_array[:, 8])):
        positioning_array[:, 8][i] = round(float(positioning_array[:, 8][i]))

    # an array for the robot file is created which is sorted based on the placement order numbering
    robotFilearray = sorted(positioning_array, key=lambda x: x[6])

    # the title row is inserted at the first row of the array
    robotFilearray = np.insert(robotFilearray, 0, np.array(newrow), 0)

    # add the reposition column to robot file by using the fully blocked magnets dictionary
    robotFilearray = add_repositionCol_to_robotFile(positioning_array,robotFilearray,fully_blocked_magnets_dictionary)

    # TEST PRINT
    print(robotFilearray)

    # write the robot file array into the CSV file for the robot
    with open(robotFile, 'w+') as robotFile:
        writer = csv.writer(robotFile, delimiter=' ')
        writer.writerows(robotFilearray)

    return positioning_array,robotFilearray

def add_repositionCol_to_robotFile(positioning_array,robotFilearray,fully_blocked_magnets_dictionary):

    # Creates a list containing w lists, each of h item/s, all filled with 0
    w, h = len(positioning_array[:, 8]) + 1, 1
    nameColumn = [['[0]' for x in range(w)] for y in range(h)]

    j=0
    for i in robotFilearray:
        if hasNumbers(i[1]):
            nameColumn[0][j] = i[1]
        else:
            nameColumn[0][j] = i[1] + str(i[9])
        j += 1

    print(nameColumn)

    # transposing the list to a column with a title assigned
    nameColumn[0][0] = 'Magnet_title'
    nameColumn = np.transpose(nameColumn)

    # add the 'list' column to the robot file array in desired position
    robotFilearray = np.hstack((robotFilearray[:, :8], nameColumn, robotFilearray[:, 8:]))

    # Creates a list containing w lists, each of h item/s, all filled with 0
    w, h = len(positioning_array[:, 9]) + 1, 1
    rePosition_col = [['[0]' for x in range(w)] for y in range(h)]

    # TEST PRINT to check the arrays
    print(len(positioning_array[:, 8]))
    print(len(rePosition_col))
    print(len(nameColumn))

    # filling out the created list with the blocked magnets dictionary in order with the robot file array
    for each_magnet in fully_blocked_magnets_dictionary:
        for i in range(len(robotFilearray)):
            if (robotFilearray[i][0] + ' ' + str(robotFilearray[i][10])) == each_magnet:
                for j in range(len(fully_blocked_magnets_dictionary[each_magnet])):
                    if j>0:
                        rePosition_col[0][i] += '_'
                    else:
                        rePosition_col[0][i] = ''
                    for k in range(len(robotFilearray)):
                        if robotFilearray[k][0]+' '+str(robotFilearray[k][10]) == fully_blocked_magnets_dictionary[each_magnet][j]:
                            rePosition_col[0][i] += str(robotFilearray[k][8])

    # transposing the list to a column with a title assigned
    rePosition_col[0][0] = 'rePosition_magnets'
    rePosition_col = np.transpose(rePosition_col)

    ## TEST PRINTs
    print(rePosition_col)
    print(rePosition_col.shape)
    print(robotFilearray.shape)

    # add the 'list' column to the robot file array in desired position
    robotFilearray = np.hstack((robotFilearray[:, :9], rePosition_col, robotFilearray[:, 9:]))

    return robotFilearray

def hasNumbers(inputString):
    return any(char.isdigit() for char in inputString)

def positioningArray_adjust_and_mergetoFile(positioning_array, plate_file, outputFile, newrow,newrow_circular):

    # splitting positioning array into circular
    positioning_array_circular = np.vsplit(positioning_array, 2)[0]

    # adding the title row and getting rid of some columns not required
    positioning_array_circular = np.insert(positioning_array_circular, 0, np.array(newrow_circular), 0)
    positioning_array_circular = np.delete(positioning_array_circular, [2, 3, 4, 5, 9, 10], 1)

    # splitting positioning array to keep only rectangular ones
    positioning_array = np.vsplit(positioning_array, 2)[1]

    # adding the title row and getting rid of some columns not required
    positioning_array = np.insert(positioning_array, 0, np.array(newrow), 0)
    positioning_array = np.delete(positioning_array, [2, 3, 4, 5, 8], 1)

    # index for keeping count
    index = 0

    # Open the input_file in read mode and output_file in write mode
    with open(plate_file, 'r') as read_obj, \
            open(outputFile, 'w+', newline='') as write_obj:

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

            # updating index count
            index += 1

    return positioning_array,positioning_array_circular

# final files being formatted to maintain consistency
def finalFiles(outputFile, robotFile):

    df3 = pd.read_csv(outputFile, header=0)
    df3.to_csv(outputFile, index=False, sep=' ', quoting=csv.QUOTE_NONE, escapechar=' ')

    # df4 = pd.read_csv(robotFile, header=0, error_bad_lines=False)
    # df4.to_csv(robotFile, index=False, sep=' ', quoting=csv.QUOTE_NONE, escapechar=' ')
