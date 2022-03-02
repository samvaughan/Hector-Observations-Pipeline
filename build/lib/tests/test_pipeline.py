import pytest
from hypothesis import given, strategies as st
import numpy as np
import pandas as pd
from pathlib import Path
import pickle
from mock import patch
import inspect

from hop.hexabundle_allocation.problem_operations import extract_data, file_arranging, offsets, hexabundle
from hop.pipeline import HectorPipe

@pytest.fixture(scope='module')
def config():

    config = dict(
        fileNameHexa="tests/data/specimen_tile_files/Hexas_tile_150_m22_guides_central_CONFIGURED_correct_header.csv",
        fileNameGuides="tests/data/specimen_tile_files/Guides_tile_150_m22_guides_central_CONFIGURED_correct_header.csv",
        plate_file = f"tests/data/full_pipeline_output/Hexa_and_Guides_150_m22_guides_central.txt",
        guide_outputFile = f"tests/data/full_pipeline_output/HECTOROutput_Guides_150_m22_guides_central.txt",
        offset_circularAnnulus = {'Blu':0.0, 'Gre':0.0, 'Yel':0.0, 'Mag':0.0},
        offsetFile="hop/hexabundle_allocation/Hexa_final_prism_gluing_PQ_table.xlsx",
        output_filename_stem='unit_tests',
        flagsfile = "tests/data/full_pipeline_output/Flags.txt",
        mu_1re_cutoff=22,
        fibre_file = "hop/hexabundle_allocation/Fibre_slitInfo_final.csv"
        )

    return config


@pytest.fixture()
def run_entire_allocation(config):

    # Mock the HectorPipe __init__ function to run the allocation without the init function making all those directories/etc
    with patch.object(HectorPipe, "__init__", lambda x: None):
        HP = HectorPipe()
        HP.header_dictionary = {"LABEL": 'test'}
        HP.allocation_files_location_base = "tests/data/full_pipeline_output/"
        HP.allocation_files_location_tiles = "tests/data/full_pipeline_output/"
        HP.allocation_files_location_robot = "tests/data/full_pipeline_output/"
        HP.plot_location = "tests/data/full_pipeline_output/"
        HP.config = dict(output_filename_stem='test')
        HP.offsetFile = config['offsetFile']
        HP.fibre_file = config['fibre_file']
        HP.galaxyIDrecord = {}


        HP.allocate_hexabundles_for_single_tile(config['fileNameGuides'], config['fileNameHexa'], robot_temperature=10, obs_temperature=10)

        output_tile_file = Path(f"{HP.allocation_files_location_tiles}/HECTOROutput_Hexas_tile_150_m22_guides_central_CONFIGURED_correct_header.txt")
        output_robot_file = Path(f"{HP.allocation_files_location_tiles}/Robot_tile_150_m22_guides_central_CONFIGURED_correct_header.txt")

        tile_df = pd.read_csv(output_tile_file, comment='#')
        robot_df = pd.read_csv(output_robot_file, comment='#')

        yield (tile_df, robot_df)

        output_tile_file.unlink()
        output_robot_file.unlink()




@pytest.fixture()
def specimen_output(config):

    tile_df = pd.read_csv(f"tests/data/specimen_outputs/HECTOROutput_Hexas_tile_150_m22_guides_central_CONFIGURED_correct_header.txt", comment='#')
    robot_df = pd.read_csv(f"tests/data/specimen_outputs/Robot_tile_150_m22_guides_central_CONFIGURED_correct_header.txt", comment='#')

    yield tile_df, robot_df

class Test_pipeline:

    def test_pipeline_tile_files_match_specimen(self, config, run_entire_allocation, specimen_output):

        tile_df, robot_df = run_entire_allocation

        specimen_tile_df, specimen_robot_df = specimen_output

        assert tile_df.equals(specimen_tile_df)

    def test_pipeline_robot_files_match_specimen(self, config, run_entire_allocation, specimen_output):

        tile_df, robot_df = run_entire_allocation

        specimen_tile_df, specimen_robot_df = specimen_output

        assert robot_df.equals(specimen_robot_df)