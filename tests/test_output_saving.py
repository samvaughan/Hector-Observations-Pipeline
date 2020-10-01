import numpy as np 
import pytest
import unittest
import pandas as pd

import sam_tiling.tiling_functions as T


class Test_save_target_outfile(unittest.TestCase):


    def test_RA_DEC_on_second_line(self):
        outfolder = "tests/test_save_files"
        out_name = 'test_save_targets.fld'

        tile_RA = 0.0
        tile_Dec = 0.0 


        tile_df = pd.DataFrame(dict(CATAID=[1, 2], r_mag=[19, 20], RA=[0, 0.1], DEC=[-1, -2], PRI_SAMI=[8, 8], isel=[9, 9]))
        standard_stars_for_tile = pd.DataFrame(dict(CoADD_ID=[100, 200], R_MAG_AUTO=[9, 10], RA=[2, 1.9], DEC=[-0.5, -1], isel=[27, 27]))

        print(dir(T))

        T.save_tile_text_file(outfolder, out_name, tile_df, standard_stars_for_tile, tile_RA, tile_Dec)

        with open(f"{outfolder}/Tiles/{out_name}", 'r') as f:
            lines = f.readlines()

        # Check that the second line is the correct RA and DEC
        r, d = lines[1].strip("#").strip().split()

        self.assertTrue((float(r)==tile_RA) & (float(d)==tile_Dec))

    def test_correct_type_entered(self):

        outfolder = "tests/test_save_files"
        out_name = 'test_save_targets.fld'

        tile_RA = 0.0
        tile_Dec = 0.0 

        tile_df = pd.DataFrame(dict(CATAID=[1, 2], r_mag=[19, 20], RA=[0, 0.1], DEC=[-1, -2], PRI_SAMI=[8, 8], isel=[9, 9]))
        standard_stars_for_tile = pd.DataFrame(dict(CoADD_ID=[100, 200], R_MAG_AUTO=[9, 10], RA=[2, 1.9], DEC=[-0.5, -1], isel=[27, 27]))

        T.save_tile_text_file(outfolder, out_name, tile_df, standard_stars_for_tile, tile_RA, tile_Dec)

        test_df = pd.read_csv(f"{outfolder}/Tiles/{out_name}", skiprows=2, delim_whitespace=True)

        # Check that this has types equal to [1, 1, 0, 0]
        self.assertTrue(np.array_equal(test_df['type'].values, np.array([1, 1, 0, 0])))



    def test_correct_column_names_for_targets(self):
        outfolder = "tests/test_save_files"
        out_name = 'test_save_targets.fld'

        tile_RA = 0.0
        tile_Dec = 0.0 


        tile_df = pd.DataFrame(dict(CATAID=[1, 2], r_mag=[19, 20], RA=[0, 0.1], DEC=[-1, -2], PRI_SAMI=[8, 8], isel=[9, 9]))
        standard_stars_for_tile = pd.DataFrame(dict(CoADD_ID=[100, 200], R_MAG_AUTO=[9, 10], RA=[2, 1.9], DEC=[-0.5, -1], isel=[27, 27]))

        T.save_tile_text_file(outfolder, out_name, tile_df, standard_stars_for_tile, tile_RA, tile_Dec)
        
        test_df = pd.read_csv(f"{outfolder}/Tiles/{out_name}", skiprows=2, delim_whitespace=True)

        # Check that this has column names 'ID', 'RA', 'DEC' and 'mag' and type
        self.assertTrue(np.array_equal(test_df.columns.tolist(), np.array(['ID', 'RA', 'DEC', 'mag', 'type', 'isel'])))

    def test_index_not_included(self):

        outfolder = "tests/test_save_files"
        out_name = 'test_save_targets.fld'

        tile_RA = 0.0
        tile_Dec = 0.0 

        tile_df = pd.DataFrame(dict(CATAID=[1, 2], r_mag=[19, 20], RA=[0, 0.1], DEC=[-1, -2], PRI_SAMI=[8, 8], isel=[9, 9]))
        standard_stars_for_tile = pd.DataFrame(dict(CoADD_ID=[100, 200], R_MAG_AUTO=[9, 10], RA=[2, 1.9], DEC=[-0.5, -1], isel=[27, 27]))

        T.save_tile_text_file(outfolder, out_name, tile_df, standard_stars_for_tile, tile_RA, tile_Dec)

        test_df = pd.read_csv(f"{outfolder}/Tiles/{out_name}", skiprows=2, delim_whitespace=True)
        data = np.genfromtxt(f"{outfolder}/Tiles/{out_name}", skip_header=3)

        # If we have an index column, numpy will pick it up but pandas won't. Therefore these two things won't be equal
        self.assertEqual(data.shape[1], len(test_df.columns.tolist()))

class Test_save_guide_outfile(unittest.TestCase):


    def test_RA_DEC_on_second_line(self):
        outfolder = "tests/test_save_files"
        out_name = 'test_save_guides.fld'

        tile_RA = 0.0
        tile_Dec = 0.0 

        guide_stars_for_tile = pd.DataFrame(dict(CoADD_ID=[100, 200], R_MAG_AUTO=[9, 10], RA=[2, 1.9], DEC=[-0.5, -1]))

        T.save_guide_text_file(outfolder, out_name, guide_stars_for_tile, tile_RA, tile_Dec)

        with open(f"{outfolder}/Tiles/{out_name}", 'r') as f:
            lines = f.readlines()

        # Check that the second line is the correct RA and DEC
        r, d = lines[1].strip("#").strip().split()

        self.assertTrue((float(r)==tile_RA) & (float(d)==tile_Dec))

    def test_correct_type_entered(self):

        outfolder = "tests/test_save_files"
        out_name = 'test_save_guides.fld'

        tile_RA = 0.0
        tile_Dec = 0.0 

        guide_stars_for_tile = pd.DataFrame(dict(CoADD_ID=[100, 200], R_MAG_AUTO=[9, 10], RA=[2, 1.9], DEC=[-0.5, -1]))
        T.save_guide_text_file(outfolder, out_name, guide_stars_for_tile, tile_RA, tile_Dec)

        test_df = pd.read_csv(f"{outfolder}/Tiles/{out_name}", skiprows=2, delim_whitespace=True)

        # Check that this has types equal to [2, 2]
        self.assertTrue(np.array_equal(test_df['type'].values, np.array([2, 2])))


    def test_if_correct_column_names_for_guide(self):

        outfolder = "tests/test_save_files"
        out_name = 'test_save_guides.fld'

        tile_RA = 0.0
        tile_Dec = 0.0 

        guide_stars_for_tile = pd.DataFrame(dict(CoADD_ID=[100, 200], R_MAG_AUTO=[9, 10], RA=[2, 1.9], DEC=[-0.5, -1]))
        T.save_guide_text_file(outfolder, out_name, guide_stars_for_tile, tile_RA, tile_Dec)

        test_df = pd.read_csv(f"{outfolder}/Tiles/{out_name}", skiprows=2, delim_whitespace=True)

        # Check that this has column names 'ID', 'RA', 'DEC' and 'mag' and type
        self.assertTrue(np.array_equal(test_df.columns.tolist(), np.array(['ID', 'RA', 'DEC', 'mag', 'type'])))

    def test_index_not_included(self):

        outfolder = "tests/test_save_files"
        out_name = 'test_save_guides.fld'

        tile_RA = 0.0
        tile_Dec = 0.0 

        guide_stars_for_tile = pd.DataFrame(dict(CoADD_ID=[100, 200], R_MAG_AUTO=[9, 10], RA=[2, 1.9], DEC=[-0.5, -1]))
        T.save_guide_text_file(outfolder, out_name, guide_stars_for_tile, tile_RA, tile_Dec)

        test_df = pd.read_csv(f"{outfolder}/Tiles/{out_name}", skiprows=2, delim_whitespace=True)


        test_df = pd.read_csv(f"{outfolder}/Tiles/{out_name}", skiprows=2, delim_whitespace=True)
        data = np.genfromtxt(f"{outfolder}/Tiles/{out_name}", skip_header=3)

        # If we have an index column, numpy will pick it up but pandas won't. Therefore these two things won't be equal
        self.assertEqual(data.shape[1], len(test_df.columns.tolist()))