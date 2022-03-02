import pytest
from hypothesis import given, strategies as st
import numpy as np
import pandas as pd
from pathlib import Path
import pickle
from mock import patch

from hop.hexabundle_allocation.problem_operations import extract_data, file_arranging, offsets, hexabundle
from hop.pipeline import HectorPipe

@pytest.fixture(scope='module')
def config():

    config = dict(
        fileNameHexa="tests/data/specimen_tile_files/Hexas_tile_150_m22_guides_central_CONFIGURED_correct_header.csv",
        fileNameGuides="tests/data/specimen_tile_files/Guides_tile_150_m22_guides_central_CONFIGURED_correct_header.csv",
        plate_file = f"tests/data/offsets/Hexa_and_Guides_150_m22_guides_central.txt",
        guide_outputFile = f"tests/data/offsets/HECTOROutput_Guides_150_m22_guides_central.txt",
        offset_circularAnnulus = {'Blu':0.0, 'Gre':0.0, 'Yel':0.0, 'Mag':0.0},
        offsetFile="hop/hexabundle_allocation/Hexa_final_prism_gluing_PQ_table.xlsx",
        output_filename_stem='unit_tests',
        flagsfile = "tests/data/offsets/Flags.txt",
        mu_1re_cutoff=22,
        fibre_file = "hop/hexabundle_allocation/Fibre_slitInfo_final.csv"
        )

    return config


# Make a list of all magnets as we'd have in the code
@pytest.fixture()
def all_magnets(config):

    plate_file = Path(config['plate_file'])
    guide_outputFile = Path(config['guide_outputFile'])

    # Adding ID column and removing the header line of Guides clusterconf to add to the hexa cluster
    df_guideFile, guideFileList = file_arranging.arrange_guidesFile(config['fileNameHexa'], config['fileNameGuides'], guide_outputFile)

    # Adding guides cluster txt file to hexa cluster txt file
    file_arranging.merge_hexaAndGuides(config['fileNameHexa'], df_guideFile, plate_file)

    # extracting all the magnets and making a list of them from the plate_file
    all_magnets = extract_data.create_list_of_all_magnets_from_file(extract_data.get_file(plate_file), guideFileList)

    yield all_magnets
    # Remove the intermediate files we've made
    guide_outputFile.unlink()
    plate_file.unlink()

# Make the all_magnets list after allocation
# Make a list of all magnets as we'd have in the code
@pytest.fixture()
def all_magnets_after_allocation(config, all_magnets):


    #all_magnets = offsets.magnetPair_radialPositionOffset_circularAnnulus(config['offset_circularAnnulus'], all_magnets)
    galaxyIDrecord = {}
    galaxyIDrecord, MagnetDict = hexabundle.overall_hexabundle_size_allocation_operation_version3_largerBundlePriority(all_magnets, galaxyIDrecord, config['mu_1re_cutoff'], config['output_filename_stem'], config['flagsfile'])

    yield all_magnets
    # Remove the intermediate files we've made
    Path(config['flagsfile']).touch()
    Path(config['flagsfile']).unlink()



@pytest.fixture()
def specimen_values_magnetPair_radialPositionOffset_circularAnnulus():
    specimen_values_file = "tests/data/offsets/specimen_all_magnets_after_magnetPair_radialPositionOffset_circularAnnulus.pkl"
    with open(specimen_values_file, 'rb') as f:
        specimen_values = pickle.load(f)

    yield specimen_values

@pytest.fixture()
def specimen_values_hexaPositionOffset():
    specimen_values_file = "tests/data/offsets/specimen_all_magnets_after_hexaPositionOffset.pkl"
    with open(specimen_values_file, 'rb') as f:
        specimen_values = pickle.load(f)

    yield specimen_values



class Test_magnetPair_radialPositionOffset_circularAnnulus:

    def test_center_from_magnetPair_radialPositionOffset_circularAnnulus_against_specimen_output(self, all_magnets, config, specimen_values_magnetPair_radialPositionOffset_circularAnnulus):

        all_magnets_updated = offsets.magnetPair_radialPositionOffset_circularAnnulus(config['offset_circularAnnulus'], all_magnets)

        for specimen, test in zip(specimen_values_magnetPair_radialPositionOffset_circularAnnulus, all_magnets_updated):
            assert np.allclose(specimen.center, test.center)

    def test_view_x_y_from_magnetPair_radialPositionOffset_circularAnnulus_against_specimen_output(self, all_magnets, config, specimen_values_magnetPair_radialPositionOffset_circularAnnulus):

        all_magnets_updated = offsets.magnetPair_radialPositionOffset_circularAnnulus(config['offset_circularAnnulus'], all_magnets)

        for specimen, test in zip(specimen_values_magnetPair_radialPositionOffset_circularAnnulus, all_magnets_updated):
            assert np.allclose([specimen.view_x, specimen.view_y],  [test.view_x, test.view_y])


class Test_calculate_rectangular_magnet_center_coordinates:

    def test_against_specimen_values(self,all_magnets):

        """
        Test that the function gives us back the rectangular magnet positions we already have when we don't move any circular magnets
        """

        circular_magnets = [magnet for magnet in all_magnets if magnet.__class__.__name__ == "circular_magnet"]
        rectangular_magnets = [magnet for magnet in all_magnets if magnet.__class__.__name__ == "rectangular_magnet"]

        for c, r in zip(circular_magnets, rectangular_magnets):
            x_rect, y_rect = offsets.calculate_rectangular_magnet_center_coordinates(c.center[0], c.center[1], r.angs)
            assert np.allclose([x_rect, y_rect], r.center)



class Test_hexaPositionOffset:

    def test_center_from_hexaPositionOffset_against_specimen_values(self, all_magnets_after_allocation, config, specimen_values_hexaPositionOffset):

        all_magnets_updated = offsets.hexaPositionOffset(all_magnets_after_allocation, config['offsetFile'])

        for specimen, test in zip(specimen_values_hexaPositionOffset, all_magnets_updated):
            assert np.allclose(specimen.center, test.center)

    def test_view_from_hexaPositionOffset_against_specimen_values(self, all_magnets_after_allocation, config, specimen_values_hexaPositionOffset):
        
        all_magnets_updated = offsets.hexaPositionOffset(all_magnets_after_allocation, config['offsetFile'])

        for specimen, test in zip(specimen_values_hexaPositionOffset, all_magnets_updated):
            assert np.allclose([specimen.view_x, specimen.view_y], [test.view_x, test.view_y])

    def test_azAngs_from_hexaPositionOffset_against_specimen_values(self, all_magnets_after_allocation, config, specimen_values_hexaPositionOffset):
        
        all_magnets_updated = offsets.hexaPositionOffset(all_magnets_after_allocation, config['offsetFile'])

        for specimen, test in zip(specimen_values_hexaPositionOffset, all_magnets_updated):
            assert np.allclose(specimen.azAngs, test.azAngs)

