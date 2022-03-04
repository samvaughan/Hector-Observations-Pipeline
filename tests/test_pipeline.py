import pytest
from hypothesis import given, strategies as st
import numpy as np
import pandas as pd
from pathlib import Path
import pickle
from mock import patch
import inspect
import string

from hop.hexabundle_allocation.problem_operations import extract_data, file_arranging, offsets, hexabundle
from hop.pipeline import HectorPipe


# pytest.mark.parametrize("input_tile_file, input_guide_file, specimen_tile_file, specimen_robot_file", 
#     [("tests/data/specimen_tile_files/Hexas_tile_150_m22_guides_central_CONFIGURED_correct_header.csv", "tests/data/specimen_tile_files/Guides_tile_150_m22_guides_central_CONFIGURED_correct_header.csv", f"tests/data/specimen_outputs/HECTOROutput_Hexas_tile_150_m22_guides_central_CONFIGURED_correct_header.txt", "tests/data/specimen_outputs/Robot_tile_150_m22_guides_central_CONFIGURED_correct_header.txt")])
# input_tile_files = ["tests/data/specimen_tile_files/Hexas_tile_150_m22_guides_central_CONFIGURED_correct_header.csv"]
# input_guide_files = ["tests/data/specimen_tile_files/Guides_tile_150_m22_guides_central_CONFIGURED_correct_header.csv"]
# output_tile_file = ["tests/data/specimen_outputs/HECTOROutput_Hexas_tile_150_m22_guides_central_CONFIGURED_correct_header.txt"]
# output_robot_file = ["tests/data/specimen_outputs/Robot_tile_150_m22_guides_central_CONFIGURED_correct_header.txt"]


inputs = Path("tests/data/specimen_tile_files/")
specimen_outputs = Path("tests/data/specimen_outputs/")

test_cases = [
    {"input_tile_file" : inputs / "Hexas_tile_150_m22_guides_central_CONFIGURED_correct_header.csv",
    "input_guide_file" : inputs / "Guides_tile_150_m22_guides_central_CONFIGURED_correct_header.csv",
    "specimen_tile_file" : specimen_outputs / "HECTOROutput_Hexas_tile_150_m22_guides_central_CONFIGURED_correct_header.txt",
    "specimen_robot_file" : specimen_outputs / "Robot_tile_150_m22_guides_central_CONFIGURED_correct_header.txt",
    "fname_stem" : "tile_150_m22_guides_central_CONFIGURED_correct_header"
    },
    {"input_tile_file" : inputs / "Hexas_G12_tile_008_CONFIGURED_correct_header.csv",
    "input_guide_file" : inputs / "Guides_G12_tile_008_CONFIGURED_correct_header.csv",
    "specimen_tile_file" : specimen_outputs / "HECTOROutput_Hexas_G12_tile_008_CONFIGURED_correct_header.txt",
    "specimen_robot_file" : specimen_outputs / "Robot_G12_tile_008_CONFIGURED_correct_header.txt",
    "fname_stem" : "G12_tile_008_CONFIGURED_correct_header"
    },
    {"input_tile_file" : inputs / "Hexas_A3376_tile_000_CONFIGURED_correct_header.csv",
    "input_guide_file" : inputs / "Guides_A3376_tile_000_CONFIGURED_correct_header.csv",
    "specimen_tile_file" : specimen_outputs / "HECTOROutput_Hexas_A3376_tile_000_CONFIGURED_correct_header.txt",
    "specimen_robot_file" : specimen_outputs / "Robot_A3376_tile_000_CONFIGURED_correct_header.txt",
    "fname_stem" : "A3376_tile_000_CONFIGURED_correct_header"
    }
]

ids = ['150_m22_guides_central', 'G12_tile_008', 'A3376_tile_000']


@pytest.fixture(params=test_cases, ids=ids)
def run_pipeline_and_check_results(request, config, tmp_path):

    with patch.object(HectorPipe, "__init__", lambda x: None):
        HP = HectorPipe()
        HP.header_dictionary = {"LABEL": 'test'}
        HP.allocation_files_location_base = tmp_path
        HP.allocation_files_location_tiles = tmp_path
        HP.allocation_files_location_robot = tmp_path
        HP.plot_location = tmp_path
        HP.config = dict(output_filename_stem='test')
        HP.offsetFile = config['offsetFile']
        HP.fibre_file = config['fibre_file']
        HP.galaxyIDrecord = {}

        HP.allocate_hexabundles_for_single_tile(request.param['input_guide_file'], request.param['input_tile_file'], robot_temperature=10, obs_temperature=10)

        output_tile_file = Path(f"{HP.allocation_files_location_tiles}/HECTOROutput_Hexas_{request.param['fname_stem']}.txt")
        output_robot_file = Path(f"{HP.allocation_files_location_tiles}/Robot_{request.param['fname_stem']}.txt")

        tile_df = pd.read_csv(output_tile_file, comment='#')
        robot_df = pd.read_csv(output_robot_file, comment='#')

    specimen_tile_df = pd.read_csv(request.param['specimen_tile_file'], comment='#')
    specimen_robot_df = pd.read_csv(request.param['specimen_robot_file'], comment='#')

    yield {'tile_df': tile_df, 
            'robot_df':robot_df,
            'specimen_tile_df': specimen_tile_df,
            'specimen_robot_df' : specimen_robot_df
            }

    # Cleanup the files we make
    output_tile_file.unlink()
    output_robot_file.unlink()

    for file in [
    "Hector_fibres_Spector.txt", 
    "Hector_fibres_AAOmega.txt", 
    f"Fibre_slitInfo_{request.param['fname_stem']}.csv", 
    f"HECTOROutput_Guides_{request.param['fname_stem']}.txt",
    f"Hexa_and_Guides_{request.param['fname_stem']}.txt",
    f"fibre_slitletAAOmega_{request.param['fname_stem']}.pdf",
    f"fibre_slitletSpector_{request.param['fname_stem']}.pdf",
    f"skyfibre_slitletAAOmega_{request.param['fname_stem']}.pdf",
    f"skyfibre_slitletSpector_{request.param['fname_stem']}.pdf"
    ]:
        Path(f"{HP.allocation_files_location_base}/{file}").unlink()
    

    for hexabundle in string.ascii_uppercase[:21]:
        Path(f"{HP.allocation_files_location_base}/Fibre_coordData_hexabundle_{hexabundle}.txt").unlink()


@pytest.fixture(scope='module')
def config():

    config = dict(
        #fileNameHexa="tests/data/specimen_tile_files/Hexas_tile_150_m22_guides_central_CONFIGURED_correct_header.csv",
        #fileNameGuides="tests/data/specimen_tile_files/Guides_tile_150_m22_guides_central_CONFIGURED_correct_header.csv",
        #plate_file = f"tests/data/full_pipeline_output/Hexa_and_Guides_150_m22_guides_central.txt",
        #guide_outputFile = f"tests/data/full_pipeline_output/HECTOROutput_Guides_150_m22_guides_central.txt",
        #offset_circularAnnulus = {'Blu':0.0, 'Gre':0.0, 'Yel':0.0, 'Mag':0.0},
        offsetFile="hop/hexabundle_allocation/Hexa_final_prism_gluing_PQ_table.xlsx",
        output_filename_stem='unit_tests',
        flagsfile = "tests/data/full_pipeline_output/Flags.txt",
        mu_1re_cutoff=22,
        fibre_file = "hop/hexabundle_allocation/Fibre_slitInfo_final.csv"
        )

    return config


# @pytest.fixture()
# def run_entire_allocation(config):

#     # Mock the HectorPipe __init__ function to run the allocation without the init function making all those directories/etc
#     with patch.object(HectorPipe, "__init__", lambda x: None):
#         HP = HectorPipe()
#         HP.header_dictionary = {"LABEL": 'test'}
#         HP.allocation_files_location_base = "tests/data/full_pipeline_output/"
#         HP.allocation_files_location_tiles = "tests/data/full_pipeline_output/"
#         HP.allocation_files_location_robot = "tests/data/full_pipeline_output/"
#         HP.plot_location = "tests/data/full_pipeline_output/"
#         HP.config = dict(output_filename_stem='test')
#         HP.offsetFile = config['offsetFile']
#         HP.fibre_file = config['fibre_file']
#         HP.galaxyIDrecord = {}


#         HP.allocate_hexabundles_for_single_tile(config['fileNameGuides'], config['fileNameHexa'], robot_temperature=10, obs_temperature=10)

#         output_tile_file = Path(f"{HP.allocation_files_location_tiles}/HECTOROutput_Hexas_tile_150_m22_guides_central_CONFIGURED_correct_header.txt")
#         output_robot_file = Path(f"{HP.allocation_files_location_tiles}/Robot_tile_150_m22_guides_central_CONFIGURED_correct_header.txt")

#         tile_df = pd.read_csv(output_tile_file, comment='#')
#         robot_df = pd.read_csv(output_robot_file, comment='#')

#         yield (tile_df, robot_df)

#         output_tile_file.unlink()
#         output_robot_file.unlink()




# @pytest.fixture()
# def specimen_output(config):

#     tile_df = pd.read_csv(f"tests/data/specimen_outputs/HECTOROutput_Hexas_tile_150_m22_guides_central_CONFIGURED_correct_header.txt", comment='#')
#     robot_df = pd.read_csv(f"tests/data/specimen_outputs/Robot_tile_150_m22_guides_central_CONFIGURED_correct_header.txt", comment='#')

#     yield tile_df, robot_df



# @pytest.mark.parametrize("input_tile_file, input_guide_file, specimen_tile_file, specimen_robot_file", 
#     [("tests/data/specimen_tile_files/Hexas_tile_150_m22_guides_central_CONFIGURED_correct_header.csv", "tests/data/specimen_tile_files/Guides_tile_150_m22_guides_central_CONFIGURED_correct_header.csv", f"tests/data/specimen_outputs/HECTOROutput_Hexas_tile_150_m22_guides_central_CONFIGURED_correct_header.txt", "tests/data/specimen_outputs/Robot_tile_150_m22_guides_central_CONFIGURED_correct_header.txt")])
class Test_pipeline:

    def test_pipeline_tile_files_match_specimen(self, run_pipeline_and_check_results):

        # tile_df, robot_df = run_entire_allocation

        # specimen_tile_df, specimen_robot_df = specimen_output

        # assert tile_df.equals(specimen_tile_df)
        assert run_pipeline_and_check_results['tile_df'].equals(run_pipeline_and_check_results['specimen_tile_df'])

    def test_pipeline_robot_files_match_specimen(self, run_pipeline_and_check_results):

        # tile_df, robot_df = run_entire_allocation

        # specimen_tile_df, specimen_robot_df = specimen_output

        assert run_pipeline_and_check_results['robot_df'].equals(run_pipeline_and_check_results['specimen_robot_df'])