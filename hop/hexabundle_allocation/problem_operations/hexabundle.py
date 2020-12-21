import numpy as np
import string
import sys
import re

def overall_hexabundle_size_allocation_operation_version1(all_magnets, galaxyIDrecord, clusterNum, tileNum, flagsFile):

    hexabundleIndexAAOmega = 0
    hexabundleIndexHector = 0
    hexabundleIndexRemaining = 0

    answer = []
    answer2 = []
    answer3 = []

    hexabundleRepeatwithsameID = []

    hexabundleDict = {'A':'A169','B':'A169','C':'A127','D':'A91','E':'A61','F':'A61','G':'A61','H':'A37','I':'S91','J':'S61','K':'S61','L':'S61','M':'S61','N':'S61','O':'S61','P':'S61','Q':'S61','R':'S61','S':'S61','T':'S61','U':'S37'}

    for hexabundleLetter in hexabundleDict:
        hexabundleDict[hexabundleLetter] = re.split('(\d+)',hexabundleDict[hexabundleLetter])[0:2]

    # Creating magnet dictionary with all necessary parameters required for hexabundle allocation
    MagnetDict = create_Magnet_Dictionary(all_magnets)

    for i in all_magnets:

        ## Check for match of galaxies with same ID
        for recordID in galaxyIDrecord:

            # searching each magnet ID in the records
            if i.IDs == recordID and i.__class__.__name__ == 'rectangular_magnet' \
            and galaxyIDrecord[recordID] != 'U' and galaxyIDrecord[recordID] != 'H':
                print('Galaxy ID match found with previous tiles !')
                # print(recordID)
                # print(galaxyIDrecord[recordID])

                # Galaxy ID match found in the records
                if galaxyIDrecord[recordID] in hexabundleRepeatwithsameID:

                    # storing hexabundle letter in the 'case' variable
                    case = galaxyIDrecord[recordID]

                    # Allocating the next largest hexabundle when a repeat of same hexabundle allocation occurred
                    while (case in hexabundleRepeatwithsameID):
                        # AAOmega hexabundle repeat- shifting to next largest same type of bundle
                        if 'A' < case <= 'G':
                            case = chr(ord(case)-1)
                        # Spector hexabundle repeat- shifting to next largest same type of bundle
                        elif 'I' <= case < 'T':
                            case = chr(ord(case)+1)
                        # All AAOmega hexabundles allocated, so shifting to Spector and recording as flag
                        elif hexabundleIndexAAOmega == 7:
                            case = 'J'
                            with open(flagsFile, 'a') as fp:
                                writeToFile = 'ID CHECK. All AAOmega  hexabundles allocated, so shifting to Spector. Cluster/Batch :' + str(clusterNum) + ',tile ' + str(tileNum) + '; ' \
                                              + i.__class__.__name__+' '+str(int(i.index))+', galaxy ID:'+str(recordID) \
                                              + ', hexabundle: ' + str(galaxyIDrecord[recordID]) + '\n'
                                fp.write(writeToFile)
                            print('ALL AAOMEGA HAS BEEN ALLOCATED SO HECTOR WILL BE SELECTED NOW')
                        # All Spector hexabundles allocated, so shifting to AAOmega and recording as flag
                        elif hexabundleIndexHector == 12:
                            case = 'G'
                            with open(flagsFile, 'a') as fp:
                                writeToFile = 'ID CHECK. All Spector hexabundles allocated, so shifting to AAOmega. Cluster/Batch :'+str(clusterNum)+',tile '+str(tileNum)+'; ' \
                                              + i.__class__.__name__+' '+str(int(i.index))+', galaxy ID:'+str(recordID) \
                                              + ', hexabundle: '+ str(galaxyIDrecord[recordID]) + '\n'
                                fp.write(writeToFile)
                            print('ALL SPECTOR HAS BEEN ALLOCATED SO AAOMEGA WILL BE SELECTED NOW')
                        # Move back to smallest AAOmega hexabundle in case of A and B both being in records already
                        elif case == 'A':
                            if ('B' in hexabundleRepeatwithsameID):
                                case = 'G'
                            elif  ('B' not in hexabundleRepeatwithsameID):
                                case = 'B'
                        # Move back to first Spector hexabundle in case of last one being in records already
                        elif case == 'T':
                            case = 'I'
                        else:
                        # Record the exceptional case of an unresolved repeat as a flag
                            with open(flagsFile, 'a') as fp:
                                writeToFile = 'EXCEPTIONAL CASE OF UNRESOLVED REPEAT !!! \n'
                                fp.write(writeToFile)
                            break

                    # Galaxy with ID repeat being allocated the same hexabundle or nearest size bundle and recorded
                    MagnetDict[i.__class__.__name__ + ' ' + str(int(i.index))]['hexabundle'] = case
                    hexabundleRepeatwithsameID.insert(len(hexabundleRepeatwithsameID), case)
                    print(hexabundleRepeatwithsameID)

                    # Respective hexabundle being taken out of hexabundle dictionary as it's allocated already
                    hexabundleDict.pop(case, None)

                    # Index being updated with hexabundle allocation
                    if ('A' <= case <= 'G'):
                        hexabundleIndexAAOmega += 1
                    elif ('I' <= case <= 'T'):
                        hexabundleIndexHector += 1

                # Galaxy ID match not found in the records
                elif galaxyIDrecord[recordID] not in hexabundleRepeatwithsameID:

                    # Galaxy without ID repeat being allocated the same hexabundle and recorded
                    MagnetDict[i.__class__.__name__ + ' ' + str(int(i.index))]['hexabundle'] = galaxyIDrecord[recordID]
                    hexabundleRepeatwithsameID.insert(len(hexabundleRepeatwithsameID),galaxyIDrecord[recordID])
                    print(hexabundleRepeatwithsameID)

                    # Respective hexabundle being taken out of hexabundle dictionary as it's allocated already
                    hexabundleDict.pop(galaxyIDrecord[recordID], None)

                    # Index being updated with hexabundle allocation
                    if ('A' <= galaxyIDrecord[recordID] <= 'G'):
                        hexabundleIndexAAOmega += 1
                    elif ('I' <= galaxyIDrecord[recordID] <= 'T'):
                        hexabundleIndexHector += 1

                    print(hexabundleIndexHector)
                    print(MagnetDict[i.__class__.__name__+' '+str(int(i.index))]['hexabundle'])


        # To avoid multiple recording of galaxy ID, recording only the ones not available in galaxy ID records
        if str(i.IDs) != 'nan' and i.__class__.__name__ == 'rectangular_magnet' and (i.IDs not in galaxyIDrecord):
                galaxyIDrecord[round(i.IDs)] = ''

    #     # Arrays to store the respective parameters for visualizing the hexabundle allocation criteria for convenience
    #     if i.__class__.__name__ == 'rectangular_magnet' and i.galaxyORstar == 1 and (12 > i.Re > 5) and len(answer) < 7 and i.mu_1re < 23:
    #         answer = answer + [i.mu_1re]
    #     elif i.__class__.__name__ == 'rectangular_magnet' and i.galaxyORstar == 1 and i.Mstar < 9.5 and len(answer2) < 12:
    #         answer2 = answer2 + [i.Mstar]
    #     elif i.__class__.__name__ == 'rectangular_magnet' and i.galaxyORstar == 1 and len(answer3) < (21 - len(answer) - len(answer2)):
    #         answer3 = answer3 + [i.mu_1re]
    #
    #     # sorted index of the arrays as per the respective parameters
    #     mu_1re_INDEX = np.argsort(np.argsort(answer))
    #     Mstar_INDEX = np.argsort(np.argsort(answer2))
    #     mu_1re_INDEX_remaining = np.argsort(np.argsort(answer3))
    #
    # #print(answer)
    # print(mu_1re_INDEX)
    # print(Mstar_INDEX)
    # print(mu_1re_INDEX_remaining)
    #print(string.ascii_uppercase[25])
    #print(MagnetDict.keys())


    # allocation of 'I' and 'U' hexabundles to the stars
    MagnetDict, hexabundleDict = star_hexabundle_allocation(MagnetDict, hexabundleDict)

    # getting a count of the probe types- hexa and guides
    GuidesCount, HexaCount = get_probe_count(MagnetDict)

    print(hexabundleDict)
    print(GuidesCount)
    print(HexaCount)

    Mu_1re_Sort = sorted(MagnetDict.items(), key=lambda x: x[1]['mu_1re'])


    ## FIRST CRITERIA FOR HEXABUNDLE ALLOCATION: AAOmega

    Re_Sort = sorted(MagnetDict.items(), key=lambda x: x[1]['Re'], reverse=True)  # largest Re at first
    for magnet in Re_Sort:

        # allocating AAOmega hexabundles to rectangular magnets of Re within range 5-12 and mu_1re less than 23
        if ('rectangular_magnet' in magnet[0]) and (magnet[1]['hexabundle'] == 'NA') and (12 > magnet[1]['Re'] > 5) \
        and hexabundleIndexAAOmega < 7 and magnet[1]['mu_1re'] < 23:

            for hexabundle in hexabundleDict:
                if hexabundleDict[hexabundle][0] == 'A':
                    MagnetDict[magnet[0]]['hexabundle'] = hexabundle
                    print(magnet)
                    hexabundleIndexAAOmega += 1
                    break
            # removing the allocated hexabundle from the hexabundle dictionary
            del hexabundleDict[MagnetDict[magnet[0]]['hexabundle']]

    print(hexabundleDict)
    print(hexabundleIndexAAOmega)

    ## SECOND CRITERIA FOR HEXABUNDLE ALLOCATION: Spector
    Mstar_Sort = sorted(MagnetDict.items(), key=lambda x: x[1]['Mstar'])
    for magnet in Mstar_Sort:

        # allocating Spector hexabundles to rectangular magnets of Mstar less than 9.5
        if ('rectangular_magnet' in magnet[0]) and (magnet[1]['hexabundle'] == 'NA') and (magnet[1]['Mstar'] < 9.5) \
        and hexabundleIndexHector < 12:

            for hexabundle in reversed(hexabundleDict):
                if hexabundleDict[hexabundle][0] == 'S':
                    MagnetDict[magnet[0]]['hexabundle'] = hexabundle
                    print(magnet)
                    hexabundleIndexHector += 1
                    break
            # removing the allocated hexabundle from the hexabundle dictionary
            del hexabundleDict[MagnetDict[magnet[0]]['hexabundle']]

    print(hexabundleDict)
    print(hexabundleIndexHector)

    # THIRD CRITERIA FOR HEXABUNDLE ALLOCATION FOR REMAINING AAOMEGA AND SPECTOR
    hexabundleIndexThirdCriteria = (HexaCount-2) - hexabundleIndexAAOmega - hexabundleIndexHector
    for magnet in Re_Sort:

        # allocating hexabundles larger than 61-cores to rectangular magnets based on larger Re values
        if ('rectangular_magnet' in magnet[0]) and (magnet[1]['hexabundle'] == 'NA') \
                and hexabundleIndexRemaining < hexabundleIndexThirdCriteria:

            for hexabundle in hexabundleDict:
                if hexabundleDict[hexabundle][1] != '61':
                    MagnetDict[magnet[0]]['hexabundle'] = hexabundle
                    print(magnet)
                    hexabundleIndexRemaining += 1
                    break
            # removing the allocated hexabundle from the hexabundle dictionary
            hexabundleDict.pop(MagnetDict[magnet[0]]['hexabundle'], None)#del hexabundleDict[MagnetDict[magnet[0]]['hexabundle']]


    print(hexabundleDict)
    print(hexabundleIndexRemaining)
    print(hexabundleIndexThirdCriteria)
    print('\n')

    i = 0
    j = 0
    for magnet in Mstar_Sort:

        # allocating rest of 61-core hexabundles with lower Mstar value going to Spector
        if ('rectangular_magnet' in magnet[0]) and (magnet[1]['hexabundle'] == 'NA') \
        and hexabundleIndexRemaining < hexabundleIndexThirdCriteria:

            print(sorted(hexabundleDict.items(), key = lambda p: p[0][0],reverse=True))
            for item in sorted(hexabundleDict.items(), key = lambda p: p[0][0],reverse=True):
                print(item[0])
                if hexabundleDict[item[0]][0] == 'S':
                    MagnetDict[magnet[0]]['hexabundle'] = item[0]
                    print(magnet)
                    hexabundleIndexRemaining += 1
                    break
                elif hexabundleDict[item[0]][0] == 'A':
                    MagnetDict[magnet[0]]['hexabundle'] = item[0]
                    print(magnet)
                    hexabundleIndexRemaining += 1
                    break
            # removing the allocated hexabundle from the hexabundle dictionary
            hexabundleDict.pop(MagnetDict[magnet[0]]['hexabundle'], None)  # del hexabundleDict[MagnetDict[magnet[0]]['hexabundle']]

    print(hexabundleDict)

    # I AND D hexabundles of 91 cores being checked and resolved to make hexabundle 'I' have lower Mstar value
    MagnetDict = cores91_hexabundles_sortedByMstar(MagnetDict)

    # checking for repeats of hexabundle
    result = check_for_hexabundle_allocation_repeats(MagnetDict)
    # resolving repeats of hexabundle
    MagnetDict, hexabundleRepeatwithsameID, indexx = resolve_hexabundle_allocation_repeats(result, \
                                                                                hexabundleRepeatwithsameID,MagnetDict)

    # checking for hexabundles not allocated initially
    unallocatedOnes = check_for_unallocated_hexabundle(MagnetDict, HexaCount)
    # resolving hexabundles not allocated initially
    MagnetDict = resolve_unallocated_hexabundle(unallocatedOnes,result,MagnetDict,indexx)


    # recording hexabundle as per galaxy records; IDs in current tile have been recorded above
    galaxyIDrecord, MagnetDict = update_galaxyIDrecord_withHexabundle(galaxyIDrecord,MagnetDict)

    return galaxyIDrecord, MagnetDict

def overall_hexabundle_size_allocation_operation_version3_largerBundlePriority(all_magnets, galaxyIDrecord, clusterNum, tileNum, flagsFile):

    hexabundleIndexAAOmega = 0
    hexabundleIndexHector = 0
    hexabundleIndexRemaining = 0

    answer = []
    answer2 = []

    hexabundleRepeatwithsameID = []

    hexabundleDict = {'A':'A169','B':'A169','C':'A127','D':'A91','E':'A61','F':'A61','G':'A61','H':'A37','I':'S91','J':'S61','K':'S61','L':'S61','M':'S61','N':'S61','O':'S61','P':'S61','Q':'S61','R':'S61','S':'S61','T':'S61','U':'S37'}

    for hexabundleLetter in hexabundleDict:
        hexabundleDict[hexabundleLetter] = re.split('(\d+)',hexabundleDict[hexabundleLetter])[0:2]

    # Creating magnet dictionary with all necessary parameters required for hexabundle allocation
    MagnetDict = create_Magnet_Dictionary(all_magnets)

    for i in all_magnets:

        ## Check for match of galaxies with same ID
        for recordID in galaxyIDrecord:

            # searching each magnet ID in the records
            if i.IDs == recordID and i.__class__.__name__ == 'rectangular_magnet' \
            and galaxyIDrecord[recordID] != 'U' and galaxyIDrecord[recordID] != 'H':
                print('Galaxy ID match found with previous tiles !')
                # print(recordID)
                # print(galaxyIDrecord[recordID])

                # Galaxy ID match found in the records
                if galaxyIDrecord[recordID] in hexabundleRepeatwithsameID:

                    # storing hexabundle letter in the 'case' variable
                    case = galaxyIDrecord[recordID]

                    # Allocating the next largest hexabundle when a repeat of same hexabundle allocation occurred
                    while (case in hexabundleRepeatwithsameID):
                        # AAOmega hexabundle repeat- shifting to next largest same type of bundle
                        if 'A' < case <= 'G':
                            case = chr(ord(case)-1)
                        # Spector hexabundle repeat- shifting to next largest same type of bundle
                        elif 'I' <= case < 'T':
                            case = chr(ord(case)+1)
                        # All AAOmega hexabundles allocated, so shifting to Spector and recording as flag
                        elif hexabundleIndexAAOmega == 7:
                            case = 'J'
                            with open(flagsFile, 'a') as fp:
                                writeToFile = 'ID CHECK. All AAOmega  hexabundles allocated, so shifting to Spector. Cluster/Batch :' + str(clusterNum) + ',tile ' + str(tileNum) + '; ' \
                                              + i.__class__.__name__+' '+str(int(i.index))+', galaxy ID:'+str(recordID) \
                                              + ', hexabundle: ' + str(galaxyIDrecord[recordID]) + '\n'
                                fp.write(writeToFile)
                            print('ALL AAOMEGA HAS BEEN ALLOCATED SO HECTOR WILL BE SELECTED NOW')
                        # All Spector hexabundles allocated, so shifting to AAOmega and recording as flag
                        elif hexabundleIndexHector == 12:
                            case = 'G'
                            with open(flagsFile, 'a') as fp:
                                writeToFile = 'ID CHECK. All Spector hexabundles allocated, so shifting to AAOmega. Cluster/Batch :'+str(clusterNum)+',tile '+str(tileNum)+'; ' \
                                              + i.__class__.__name__+' '+str(int(i.index))+', galaxy ID:'+str(recordID) \
                                              + ', hexabundle: '+ str(galaxyIDrecord[recordID]) + '\n'
                                fp.write(writeToFile)
                            print('ALL SPECTOR HAS BEEN ALLOCATED SO AAOMEGA WILL BE SELECTED NOW')
                        # Move back to smallest AAOmega hexabundle in case of A and B both being in records already
                        elif case == 'A':
                            if ('B' in hexabundleRepeatwithsameID):
                                case = 'G'
                            elif  ('B' not in hexabundleRepeatwithsameID):
                                case = 'B'
                        # Move back to first Spector hexabundle in case of last one being in records already
                        elif case == 'T':
                            case = 'I'
                        else:
                        # Record the exceptional case of an unresolved repeat as a flag
                            with open(flagsFile, 'a') as fp:
                                writeToFile = 'EXCEPTIONAL CASE OF UNRESOLVED REPEAT !!! \n'
                                fp.write(writeToFile)
                            break

                    # Galaxy with ID repeat being allocated the same hexabundle or nearest size bundle and recorded
                    MagnetDict[i.__class__.__name__ + ' ' + str(int(i.index))]['hexabundle'] = case
                    hexabundleRepeatwithsameID.insert(len(hexabundleRepeatwithsameID), case)
                    print(hexabundleRepeatwithsameID)

                    # Respective hexabundle being taken out of hexabundle dictionary as it's allocated already
                    hexabundleDict.pop(case, None)

                    # Index being updated with hexabundle allocation
                    if ('A' <= case <= 'G'):
                        hexabundleIndexAAOmega += 1
                    elif ('I' <= case <= 'T'):
                        hexabundleIndexHector += 1

                # Galaxy ID match not found in the records
                elif galaxyIDrecord[recordID] not in hexabundleRepeatwithsameID:

                    # Galaxy without ID repeat being allocated the same hexabundle and recorded
                    MagnetDict[i.__class__.__name__ + ' ' + str(int(i.index))]['hexabundle'] = galaxyIDrecord[recordID]
                    hexabundleRepeatwithsameID.insert(len(hexabundleRepeatwithsameID),galaxyIDrecord[recordID])
                    print(hexabundleRepeatwithsameID)

                    # Respective hexabundle being taken out of hexabundle dictionary as it's allocated already
                    hexabundleDict.pop(galaxyIDrecord[recordID], None)

                    # Index being updated with hexabundle allocation
                    if ('A' <= galaxyIDrecord[recordID] <= 'G'):
                        hexabundleIndexAAOmega += 1
                    elif ('I' <= galaxyIDrecord[recordID] <= 'T'):
                        hexabundleIndexHector += 1

                    print(hexabundleIndexHector)
                    print(MagnetDict[i.__class__.__name__+' '+str(int(i.index))]['hexabundle'])


        # To avoid multiple recording of galaxy ID, recording only the ones not available in galaxy ID records
        if str(i.IDs) != 'nan' and i.__class__.__name__ == 'rectangular_magnet' and (i.IDs not in galaxyIDrecord):
                galaxyIDrecord[round(i.IDs)] = ''

        # Getting an array count of mu_1re graeter and less than 22
        if i.__class__.__name__ == 'rectangular_magnet' and i.galaxyORstar == 1 and i.Re < 12:
            answer = answer + [i.Re]
        elif i.__class__.__name__ == 'rectangular_magnet' and i.galaxyORstar == 1 and i.Re >= 12:
            answer2 = answer2 + [i.Re]

        Re_lt12_INDEX = np.argsort(np.argsort(answer))
        Re_gt12_INDEX = np.argsort(np.argsort(answer2))


    print("\nRe below and above 12:  ")
    print(Re_lt12_INDEX)
    print(Re_gt12_INDEX)

    # allocation of 'I' and 'U' hexabundles to the stars
    MagnetDict, hexabundleDict = star_hexabundle_allocation(MagnetDict, hexabundleDict)

    # getting a count of the probe types- hexa and guides
    GuidesCount, HexaCount = get_probe_count(MagnetDict)

    print(hexabundleDict)
    print(GuidesCount)
    print(HexaCount)

    Re_Sort = sorted(MagnetDict.items(), key=lambda x: x[1]['Re'], reverse=True)  # largest Re at first
    Mstar_Sort = sorted(MagnetDict.items(), key=lambda x: x[1]['Mstar'])

    # Getting a count of hexabundles larger than 61
    large_bundle_count = 0
    for hexabundle in hexabundleDict:
        if int(hexabundleDict[hexabundle][1]) > 61:
            large_bundle_count += 1

    print(large_bundle_count)
    large_bundle_index = 0
    # Allocating all hexabundles greater than 61 based on Re<12
    for magnet in Re_Sort:
        if ('rectangular_magnet' in magnet[0]) and (magnet[1]['galaxyOrStar'] == 1) and (
                magnet[1]['hexabundle'] == 'NA') and (magnet[1]['Re'] < 12) and large_bundle_index < large_bundle_count:
            for hexabundle in hexabundleDict:
                if int(hexabundleDict[hexabundle][1]) > 61:  # check for larger hexabundles
                    MagnetDict[magnet[0]]['hexabundle'] = hexabundle
                    print(magnet)
                    print('Larger hexabundles')
                    large_bundle_index += 1
                    if hexabundleDict[hexabundle][0] == 'A':
                        hexabundleIndexAAOmega += 1
                    elif hexabundleDict[hexabundle][0] == 'S':
                        hexabundleIndexHector += 1
                    break
            del hexabundleDict[MagnetDict[magnet[0]]['hexabundle']]

    # In case any larger hexabundles more than 61-cores have not been allocated yet
    # (which is highly unlikely and might happen due to repeats)
    for magnet in Re_Sort:
        if ('rectangular_magnet' in magnet[0]) and (magnet[1]['galaxyOrStar'] == 1) and (
                magnet[1]['hexabundle'] == 'NA') and large_bundle_index < large_bundle_count:
            for hexabundle in hexabundleDict:
                if int(hexabundleDict[hexabundle][1]) > 61:  # check for larger hexabundles
                    MagnetDict[magnet[0]]['hexabundle'] = hexabundle
                    print(magnet)
                    print('Larger hexabundles but with more than Re=12')
                    large_bundle_index += 1
                    if hexabundleDict[hexabundle][0] == 'A':
                        hexabundleIndexAAOmega += 1
                    elif hexabundleDict[hexabundle][0] == 'S':
                        hexabundleIndexHector += 1
                    with open(flagsFile, 'a') as fp:
                        writeToFile = 'RARE case of larger hexabundles greater than 61-cores left for allocation after Re<12: ' \
                                      'Cluster/Batch :' + str(clusterNum) + ',tile ' + str(tileNum) + '; ' \
                                      + str(magnet[0]) + ', ID:' + str(recordID) + ', hexabundle: ' + str(
                            hexabundle) + '\n'
                        fp.write(writeToFile)
                    break
            del hexabundleDict[MagnetDict[magnet[0]]['hexabundle']]

    # I AND D hexabundles of 91 cores being checked and resolved to make hexabundle 'I' have lower Mstar value
    MagnetDict = cores91_hexabundles_sortedByMstar(MagnetDict)

    print('\nLarger hexabundles more than 61 have been allocated.\n')

    X = 22.5
    print('mu_1re cutoff, X = ' + str(X))
    # Allocating all 61 hexabundles based on mu_1Re < X
    for magnet in Mstar_Sort:  # Spector
        if ('rectangular_magnet' in magnet[0]) and (magnet[1]['galaxyOrStar'] == 1) and (
                magnet[1]['hexabundle'] == 'NA') and (magnet[1]['mu_1re'] < X) and hexabundleIndexHector < 12:
            for hexabundle in hexabundleDict:
                if int(hexabundleDict[hexabundle][1]) == 61 and hexabundleDict[hexabundle][0] == 'S':  # check for larger hexabundles
                    MagnetDict[magnet[0]]['hexabundle'] = hexabundle
                    print(magnet)
                    print('mu_1re<X: 61 hexabundles to Spector')
                    hexabundleIndexHector += 1
                    break
            del hexabundleDict[MagnetDict[magnet[0]]['hexabundle']]

    for magnet in Mstar_Sort:  # AAOmega
        if ('rectangular_magnet' in magnet[0]) and (magnet[1]['galaxyOrStar'] == 1) and (
                magnet[1]['hexabundle'] == 'NA') and (magnet[1]['mu_1re'] < X) and hexabundleIndexAAOmega < 7:
            for hexabundle in hexabundleDict:
                if int(hexabundleDict[hexabundle][1]) == 61 and hexabundleDict[hexabundle][0] == 'A':  # check for larger hexabundles
                    MagnetDict[magnet[0]]['hexabundle'] = hexabundle
                    print(magnet)
                    print('mu_1re<X: 61 hexabundles to AAOmega')
                    hexabundleIndexAAOmega += 1
                    break
            del hexabundleDict[MagnetDict[magnet[0]]['hexabundle']]

    print('\n61-core hexabundles based with mu_1Re<X have been allocated \n')

    # Allocating rest of 61 hexabundles based on mu_1Re > X
    for magnet in Mstar_Sort:  # Spector
        if ('rectangular_magnet' in magnet[0]) and (magnet[1]['galaxyOrStar'] == 1) and (
                magnet[1]['hexabundle'] == 'NA') and (magnet[1]['mu_1re'] >= X) and hexabundleIndexHector < 12:
            for hexabundle in hexabundleDict:
                if int(hexabundleDict[hexabundle][1]) == 61 and hexabundleDict[hexabundle][0] == 'S':  # check for larger hexabundles
                    MagnetDict[magnet[0]]['hexabundle'] = hexabundle
                    print(magnet)
                    print('mu_1re>X: 61 hexabundles to Spector')
                    hexabundleIndexHector += 1
                    break
            del hexabundleDict[MagnetDict[magnet[0]]['hexabundle']]

    for magnet in Mstar_Sort:  # AAOmega
        if ('rectangular_magnet' in magnet[0]) and (magnet[1]['galaxyOrStar'] == 1) and (
                magnet[1]['hexabundle'] == 'NA') and (magnet[1]['mu_1re'] >= X) and hexabundleIndexAAOmega < 7:
            for hexabundle in hexabundleDict:
                if int(hexabundleDict[hexabundle][1]) == 61 and hexabundleDict[hexabundle][0] == 'A':  # check for larger hexabundles
                    MagnetDict[magnet[0]]['hexabundle'] = hexabundle
                    print(magnet)
                    print('mu_1re>X: 61 hexabundles to AAOmega')
                    hexabundleIndexAAOmega += 1
                    break
            del hexabundleDict[MagnetDict[magnet[0]]['hexabundle']]

    print('\nAll 61 hexabundles have been allocated as well.\n')
    print(hexabundleDict)

    # checking for repeats of hexabundle
    result = check_for_hexabundle_allocation_repeats(MagnetDict)
    # resolving repeats of hexabundle
    MagnetDict, hexabundleRepeatwithsameID, indexx = resolve_hexabundle_allocation_repeats(result, \
                                                                                hexabundleRepeatwithsameID,MagnetDict)

    # checking for hexabundles not allocated initially
    unallocatedOnes = check_for_unallocated_hexabundle(MagnetDict, HexaCount)
    # resolving hexabundles not allocated initially
    MagnetDict = resolve_unallocated_hexabundle(unallocatedOnes,result,MagnetDict,indexx)


    # recording hexabundle as per galaxy records; IDs in current tile have been recorded above
    galaxyIDrecord, MagnetDict = update_galaxyIDrecord_withHexabundle(galaxyIDrecord,MagnetDict)

    return galaxyIDrecord, MagnetDict

def overall_hexabundle_size_allocation_operation_version2_median(all_magnets, galaxyIDrecord, clusterNum, tileNum, flagsFile):

    hexabundleIndexAAOmega = 0
    hexabundleIndexHector = 0
    hexabundleIndexRemaining = 0

    answer = []
    answer2 = []
    answer3 = []

    hexabundleRepeatwithsameID = []

    hexabundleDict = {'A':'A169','B':'A169','C':'A127','D':'A91','E':'A61','F':'A61','G':'A61','H':'A37','I':'S91','J':'S61','K':'S61','L':'S61','M':'S61','N':'S61','O':'S61','P':'S61','Q':'S61','R':'S61','S':'S61','T':'S61','U':'S37'}

    for hexabundleLetter in hexabundleDict:
        hexabundleDict[hexabundleLetter] = re.split('(\d+)',hexabundleDict[hexabundleLetter])[0:2]

    # Creating magnet dictionary with all necessary parameters required for hexabundle allocation
    MagnetDict = create_Magnet_Dictionary(all_magnets)

    for i in all_magnets:

        ## Check for match of galaxies with same ID
        for recordID in galaxyIDrecord:

            # searching each magnet ID in the records
            if i.IDs == recordID and i.__class__.__name__ == 'rectangular_magnet' \
            and galaxyIDrecord[recordID] != 'U' and galaxyIDrecord[recordID] != 'H':
                print('Galaxy ID match found with previous tiles !')
                # print(recordID)
                # print(galaxyIDrecord[recordID])

                # Galaxy ID match found in the records
                if galaxyIDrecord[recordID] in hexabundleRepeatwithsameID:

                    # storing hexabundle letter in the 'case' variable
                    case = galaxyIDrecord[recordID]

                    # Allocating the next largest hexabundle when a repeat of same hexabundle allocation occurred
                    while (case in hexabundleRepeatwithsameID):
                        # AAOmega hexabundle repeat- shifting to next largest same type of bundle
                        if 'A' < case <= 'G':
                            case = chr(ord(case)-1)
                        # Spector hexabundle repeat- shifting to next largest same type of bundle
                        elif 'I' <= case < 'T':
                            case = chr(ord(case)+1)
                        # All AAOmega hexabundles allocated, so shifting to Spector and recording as flag
                        elif hexabundleIndexAAOmega == 7:
                            case = 'J'
                            with open(flagsFile, 'a') as fp:
                                writeToFile = 'ID CHECK. All AAOmega  hexabundles allocated, so shifting to Spector. Cluster/Batch :' + str(clusterNum) + ',tile ' + str(tileNum) + '; ' \
                                              + i.__class__.__name__+' '+str(int(i.index))+', galaxy ID:'+str(recordID) \
                                              + ', hexabundle: ' + str(galaxyIDrecord[recordID]) + '\n'
                                fp.write(writeToFile)
                            print('ALL AAOMEGA HAS BEEN ALLOCATED SO HECTOR WILL BE SELECTED NOW')
                        # All Spector hexabundles allocated, so shifting to AAOmega and recording as flag
                        elif hexabundleIndexHector == 12:
                            case = 'G'
                            with open(flagsFile, 'a') as fp:
                                writeToFile = 'ID CHECK. All Spector hexabundles allocated, so shifting to AAOmega. Cluster/Batch :'+str(clusterNum)+',tile '+str(tileNum)+'; ' \
                                              + i.__class__.__name__+' '+str(int(i.index))+', galaxy ID:'+str(recordID) \
                                              + ', hexabundle: '+ str(galaxyIDrecord[recordID]) + '\n'
                                fp.write(writeToFile)
                            print('ALL SPECTOR HAS BEEN ALLOCATED SO AAOMEGA WILL BE SELECTED NOW')
                        # Move back to smallest AAOmega hexabundle in case of A and B both being in records already
                        elif case == 'A':
                            if ('B' in hexabundleRepeatwithsameID):
                                case = 'G'
                            elif  ('B' not in hexabundleRepeatwithsameID):
                                case = 'B'
                        # Move back to first Spector hexabundle in case of last one being in records already
                        elif case == 'T':
                            case = 'I'
                        else:
                        # Record the exceptional case of an unresolved repeat as a flag
                            with open(flagsFile, 'a') as fp:
                                writeToFile = 'EXCEPTIONAL CASE OF UNRESOLVED REPEAT !!! \n'
                                fp.write(writeToFile)
                            break

                    # Galaxy with ID repeat being allocated the same hexabundle or nearest size bundle and recorded
                    MagnetDict[i.__class__.__name__ + ' ' + str(int(i.index))]['hexabundle'] = case
                    hexabundleRepeatwithsameID.insert(len(hexabundleRepeatwithsameID), case)
                    print(hexabundleRepeatwithsameID)

                    # Respective hexabundle being taken out of hexabundle dictionary as it's allocated already
                    hexabundleDict.pop(case, None)

                    # Index being updated with hexabundle allocation
                    if ('A' <= case <= 'G'):
                        hexabundleIndexAAOmega += 1
                    elif ('I' <= case <= 'T'):
                        hexabundleIndexHector += 1

                # Galaxy ID match not found in the records
                elif galaxyIDrecord[recordID] not in hexabundleRepeatwithsameID:

                    # Galaxy without ID repeat being allocated the same hexabundle and recorded
                    MagnetDict[i.__class__.__name__ + ' ' + str(int(i.index))]['hexabundle'] = galaxyIDrecord[recordID]
                    hexabundleRepeatwithsameID.insert(len(hexabundleRepeatwithsameID),galaxyIDrecord[recordID])
                    print(hexabundleRepeatwithsameID)

                    # Respective hexabundle being taken out of hexabundle dictionary as it's allocated already
                    hexabundleDict.pop(galaxyIDrecord[recordID], None)

                    # Index being updated with hexabundle allocation
                    if ('A' <= galaxyIDrecord[recordID] <= 'G'):
                        hexabundleIndexAAOmega += 1
                    elif ('I' <= galaxyIDrecord[recordID] <= 'T'):
                        hexabundleIndexHector += 1

                    print(hexabundleIndexHector)
                    print(MagnetDict[i.__class__.__name__+' '+str(int(i.index))]['hexabundle'])


        # To avoid multiple recording of galaxy ID, recording only the ones not available in galaxy ID records
        if str(i.IDs) != 'nan' and i.__class__.__name__ == 'rectangular_magnet' and (i.IDs not in galaxyIDrecord):
                galaxyIDrecord[round(i.IDs)] = ''

        # Getting an array count of mu_1re greater and less than 22
        if i.__class__.__name__ == 'rectangular_magnet' and i.galaxyORstar == 1 and i.mu_1re < 22:
            answer = answer + [i.mu_1re]
        elif i.__class__.__name__ == 'rectangular_magnet' and i.galaxyORstar == 1 and i.mu_1re >= 22:
            answer2 = answer2 + [i.mu_1re]

        mu_1re_lt22_INDEX = np.argsort(np.argsort(answer))
        mu_1re_gt22_INDEX = np.argsort(np.argsort(answer2))

    # print(answer)
    print("\nmu_1re above and below 22:  ")
    print(mu_1re_gt22_INDEX)
    print(mu_1re_lt22_INDEX)

    # allocation of 'I' and 'U' hexabundles to the stars
    MagnetDict, hexabundleDict = star_hexabundle_allocation(MagnetDict, hexabundleDict)

    # getting a count of the probe types- hexa and guides
    GuidesCount, HexaCount = get_probe_count(MagnetDict)

    print(hexabundleDict)
    print(GuidesCount)
    print(HexaCount)

    Re_Sort = sorted(MagnetDict.items(), key=lambda x: x[1]['Re'], reverse=True)  # largest Re at first
    Mstar_Sort = sorted(MagnetDict.items(), key=lambda x: x[1]['Mstar'])

    lowMstar_spector_number = round((12 / 19) * len(mu_1re_lt22_INDEX))
    print(lowMstar_spector_number)
    Mstar_median_lowerhalf = {}
    Mstar_median_higherhalf = {}
    Mstar_Sort_count = 0
    for magnet in Mstar_Sort:
        if ('rectangular_magnet' in magnet[0]) and (magnet[1]['mu_1re'] < 22) and (magnet[1]['galaxyOrStar'] == 1) and (
                Mstar_Sort_count < lowMstar_spector_number):
            key = str(magnet[0])
            Mstar_median_lowerhalf[key] = MagnetDict[key]
            Mstar_Sort_count += 1
        elif ('rectangular_magnet' in magnet[0]) and (magnet[1]['mu_1re'] < 22) and (
                magnet[1]['galaxyOrStar'] == 1) and ((HexaCount - 2) > Mstar_Sort_count >= lowMstar_spector_number):
            key = str(magnet[0])
            Mstar_median_higherhalf[key] = MagnetDict[key]
            Mstar_Sort_count += 1

    print(Mstar_median_lowerhalf)
    print(Mstar_median_higherhalf)
    print(len(Mstar_median_lowerhalf))
    print(len(Mstar_median_higherhalf))
    print('\nMedian based sorting')
    print(lowMstar_spector_number)

    lowMstar_Sort_count = 0
    highMstar_Sort_count = 0

    # Lower mass galaxies with mu_1Re < 22 go to Spector
    for magnet in Re_Sort:
        if ('rectangular_magnet' in magnet[0]) and (magnet[1]['hexabundle'] == 'NA') and (magnet[1]['mu_1re'] < 22):
            for magnet_2nd in Mstar_median_lowerhalf:
                if str(magnet_2nd) == str(magnet[0]) and hexabundleIndexHector < 12:
                    print('MATCH')
                    for hexabundle in hexabundleDict:
                        if hexabundleDict[hexabundle][0] == 'S':  # check for first Spector
                            MagnetDict[magnet[0]]['hexabundle'] = hexabundle
                            print(magnet)
                            hexabundleIndexHector += 1
                            lowMstar_Sort_count += 1
                            break
                    del hexabundleDict[MagnetDict[magnet[0]]['hexabundle']]

    # Higher mass with mu_1Re < 22 go to AAOmega
    for magnet in Re_Sort:
        if ('rectangular_magnet' in magnet[0]) and (magnet[1]['hexabundle'] == 'NA') and (magnet[1]['mu_1re'] < 22):
            for magnet_2nd in Mstar_median_higherhalf:
                if str(magnet_2nd) == str(magnet[0]) and hexabundleIndexAAOmega < 7:
                    print('MATCH2')
                    for hexabundle in hexabundleDict:
                        if hexabundleDict[hexabundle][0] == 'A':  # check for first AAOmega
                            MagnetDict[magnet[0]]['hexabundle'] = hexabundle
                            print(magnet)
                            hexabundleIndexAAOmega += 1
                            highMstar_Sort_count += 1
                            break
                    del hexabundleDict[MagnetDict[magnet[0]]['hexabundle']]

    # Remaining galaxies with mu_1re<22 placed on remaining bundles based on largest â€œRe" placed on the largest bundles
    for magnet in Re_Sort:
        if ('rectangular_magnet' in magnet[0]) and (magnet[1]['hexabundle'] == 'NA') and magnet[1]['mu_1re'] < 22:
            for hexabundle in hexabundleDict:
                if int(hexabundleDict[hexabundle][1]) > 61:  # check for greater hexabundle
                    MagnetDict[magnet[0]]['hexabundle'] = hexabundle
                    print(magnet)
                    if hexabundleDict[hexabundle][0] == 'A':
                        hexabundleIndexAAOmega += 1
                    elif hexabundleDict[hexabundle][0] == 'S':
                        hexabundleIndexHector += 1
                    # highMstar_Sort_count += 1
                    break
                elif int(hexabundleDict[hexabundle][1]) == 61:
                    MagnetDict[magnet[0]]['hexabundle'] = hexabundle
                    print(magnet)
                    if hexabundleDict[hexabundle][0] == 'A':
                        hexabundleIndexAAOmega += 1
                    elif hexabundleDict[hexabundle][0] == 'S':
                        hexabundleIndexHector += 1
                    # highMstar_Sort_count += 1
                    break
            del hexabundleDict[MagnetDict[magnet[0]]['hexabundle']]

    print(hexabundleIndexAAOmega)
    print(hexabundleIndexHector)

    print(lowMstar_Sort_count)
    print(highMstar_Sort_count)

    print('Galaxies with less than Mu_1re=22 allocated\n')

    # Remaining galaxies with mu_1re>22 placed on remaining bundles based on largest â€œRe" placed on the largest bundles
    for magnet in Re_Sort:
        if ('rectangular_magnet' in magnet[0]) and (magnet[1]['hexabundle'] == 'NA') and magnet[1]['mu_1re'] >= 22:
            for hexabundle in hexabundleDict:
                if int(hexabundleDict[hexabundle][1]) > 61:  # check for greater hexabundle
                    MagnetDict[magnet[0]]['hexabundle'] = hexabundle
                    print(magnet)
                    if hexabundleDict[hexabundle][0] == 'A':
                        hexabundleIndexAAOmega += 1
                    elif hexabundleDict[hexabundle][0] == 'S':
                        hexabundleIndexHector += 1
                    # highMstar_Sort_count += 1
                    break
                elif int(hexabundleDict[hexabundle][1]) == 61:
                    MagnetDict[magnet[0]]['hexabundle'] = hexabundle
                    print(magnet)
                    if hexabundleDict[hexabundle][0] == 'A':
                        hexabundleIndexAAOmega += 1
                    elif hexabundleDict[hexabundle][0] == 'S':
                        hexabundleIndexHector += 1
                    # highMstar_Sort_count += 1
                    break
            del hexabundleDict[MagnetDict[magnet[0]]['hexabundle']]

    print('Galaxies with more than Mu_1re=22 allocated\n')

    print(hexabundleDict)

    # I AND D hexabundles of 91 cores being checked and resolved to make hexabundle 'I' have lower Mstar value
    MagnetDict = cores91_hexabundles_sortedByMstar(MagnetDict)

    # checking for repeats of hexabundle
    result = check_for_hexabundle_allocation_repeats(MagnetDict)
    # resolving repeats of hexabundle
    MagnetDict, hexabundleRepeatwithsameID, indexx = resolve_hexabundle_allocation_repeats(result, \
                                                                                hexabundleRepeatwithsameID,MagnetDict)

    # checking for hexabundles not allocated initially
    unallocatedOnes = check_for_unallocated_hexabundle(MagnetDict, HexaCount)
    # resolving hexabundles not allocated initially
    MagnetDict = resolve_unallocated_hexabundle(unallocatedOnes,result,MagnetDict,indexx)


    # recording hexabundle as per galaxy records; IDs in current tile have been recorded above
    galaxyIDrecord, MagnetDict = update_galaxyIDrecord_withHexabundle(galaxyIDrecord,MagnetDict)

    return galaxyIDrecord, MagnetDict


def create_Magnet_Dictionary(all_magnets):

    MagnetDict = {}

    # storing all respective parameters to create a magnet dictionary
    for i in all_magnets:
        MagnetDict[i.__class__.__name__+' '+str(int(i.index))] = {
                    'index': int(i.index),
                    'galaxyOrStar': i.galaxyORstar,
                    'Re': i.Re,
                    'mu_1re': i.mu_1re,
                    'Mstar': i.Mstar,
                    'hexabundle': 'NA',
                    'IDs': i.IDs
                }
    return MagnetDict

def get_probe_count(MagnetDict):

    GuidesCount = 0
    HexaCount = 0

    # getting count of probe type- hexa or guide- based on galaxyorStar parameter
    for magnet in MagnetDict:
        if ('rectangular_magnet' in magnet):
            if (MagnetDict[magnet]['galaxyOrStar'] == 1 or MagnetDict[magnet]['galaxyOrStar'] == 0):
                HexaCount += 1
            else:
                GuidesCount += 1

    return GuidesCount, HexaCount

def star_hexabundle_allocation(MagnetDict,hexabundleDict):

    # identifying stars and allocating the 37-core bundles H and U
    for magnet in MagnetDict:
        if ('rectangular_magnet' in magnet) and (MagnetDict[magnet]['galaxyOrStar'] == 0) and (MagnetDict[magnet]['hexabundle'] == 'NA'):
            for hexabundle in hexabundleDict:
                if hexabundleDict[hexabundle][1] == '37':
                    MagnetDict[magnet]['hexabundle'] = hexabundle
                    break
            del hexabundleDict[MagnetDict[magnet]['hexabundle']]

    return MagnetDict,hexabundleDict

## I AND D hexabundles of 91 cores being checked and resolved to make hexabundle I have lower Mstar value
def cores91_hexabundles_sortedByMstar(MagnetDict):

    countD = 0
    countI = 0
    I_mstar_91Core = 0
    D_mstar_91Core = 0

    for magnet in MagnetDict:
        # print(str(MagnetDict[magnet]['index'])+' '+str(MagnetDict[magnet]['hexabundle']))

        # storing Mstar values and magnet labels of I and D hexabundles of 91 cores
        if MagnetDict[magnet]['hexabundle'] == 'I':
            I_mstar_91Core = MagnetDict[magnet]['Mstar']
            magnet_I = magnet
            countI == 1
        elif MagnetDict[magnet]['hexabundle'] == 'D':
            D_mstar_91core = MagnetDict[magnet]['Mstar']
            magnet_D = magnet
            countD == 1

    # Ensuring I and D hexabundles of 91 cores have been allocated based on Mstar value, with lower one going to I
    if countI == 1 and countD == 1 and I_mstar_91Core > D_mstar_91Core:
        MagnetDict[magnet_I]['hexabundle'] = 'D'
        MagnetDict[magnet_D]['hexabundle'] = 'I'
        print('I AND D SWAPPED')

    return MagnetDict

def resolve_hexabundle_allocation_repeats(result,hexabundleRepeatwithsameID,MagnetDict):

    indexx = 0
    if result != False:
        print('Allocation WRONG WRONG WRONG WRONG WRONG!')
        hexabundleRepeatwithsameID = sorted(hexabundleRepeatwithsameID)
        hexabundleRepeatwithsameID.insert(len(hexabundleRepeatwithsameID), '')
        print(hexabundleRepeatwithsameID)
        sourcelist = 'ABCDEFGIJKLMNOPQRST'
        for repeatOnes in sourcelist:
            checkForRepeat = 0
            for magnet in MagnetDict:
                if MagnetDict[magnet]['hexabundle'] == repeatOnes:
                    if checkForRepeat > 0:
                        MagnetDict[magnet]['hexabundle'] = result[indexx]
                        indexx += 1
                        print('HERE')
                        print(MagnetDict[magnet]['hexabundle'])
                    checkForRepeat += 1
            # print(MagnetDict[magnet]['hexabundle'])
        if checkForRepeat == 0 and indexx == 0:
            sys.exit('WRONG ALLOCATION FOUND!')
    else:
        print('No repeats in allocation.')

    return MagnetDict, hexabundleRepeatwithsameID,indexx

def resolve_unallocated_hexabundle(unallocatedOnes,result,MagnetDict,indexx):

    if unallocatedOnes != False:
        print('NOT ALL ARE ALLOCATED')
        MagnetDict[unallocatedOnes]['hexabundle'] = result[indexx]
        indexx += 1
        print(str(MagnetDict[unallocatedOnes]['hexabundle'])+'has been allocated as it was found unallocated.')
    else:
        print('All Hexabundles are  ALLOCATED!')

    return MagnetDict


def check_for_hexabundle_allocation_repeats(nested_dictionary):
    setOfElems = set()
    repeatLetter = 0
    print(setOfElems)
    for key, value in nested_dictionary.items():

        if type(value) is dict:

            for key1, value1 in value.items():
                if key1 == 'hexabundle':
                    if value1 in setOfElems:
                        repeatLetter = value1
                        print(repeatLetter)
                    elif value1 != 'NA':
                        setOfElems.add(value1)

    sourcelist = 'ABCDEFGIJKLMNOPQRST'
    missingLetter = [letter for letter in sourcelist if letter not in setOfElems]

    if missingLetter != [] and repeatLetter != 0:
        print(missingLetter)
        return missingLetter

    return False

def check_for_unallocated_hexabundle(nested_dictionary, HexaCount):
    setOfElems = set()
    print(setOfElems)
    for key, value in nested_dictionary.items():

        if type(value) is dict:

            for key1, value1 in value.items():
                if (key1 == 'hexabundle'):
                    if (value1 == 'NA') and ('rectangular_magnet' in key) and \
                    (nested_dictionary[key]['galaxyOrStar'] == 1 or nested_dictionary[key]['galaxyOrStar'] == 0):
                    #(0 < nested_dictionary[key]['index'] <= HexaCount):
                        return key

    return False

def update_galaxyIDrecord_withHexabundle(galaxyIDrecord, MagnetDict):

    for recordID in galaxyIDrecord:
        for magnet in MagnetDict:
            if recordID == MagnetDict[magnet]['IDs']:
                galaxyIDrecord[recordID] = MagnetDict[magnet]['hexabundle']

    return galaxyIDrecord, MagnetDict

def hexabundle_allocation_fromMagnetDict(MagnetDict, magnet, guideIndex):
    # copying allocated hexabundle from Magnet Dictionary
    magnet.hexabundle = MagnetDict[magnet.__class__.__name__ + ' ' + str(int(magnet.index))]['hexabundle']

    # allocating hexabundle to guide probes in order GS1,2..
    if (magnet.hexabundle == 'NA') and (magnet.__class__.__name__ == 'rectangular_magnet'):
        magnet.hexabundle = 'GS' + str(guideIndex)
        guideIndex += 1

    return magnet, guideIndex


