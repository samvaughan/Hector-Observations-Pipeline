import numpy as np
import unittest
import itertools
import pandas as pd
from collections import Counter
import os

from sam_tiling import tiling_functions as T


class Test_find_clashes(unittest.TestCase):

    def test_no_clashes_in_data_when_proximity_small(self):

        RA = np.array([1, 2, 2.01 ,4, 5])
        DEC = np.array([1, 2, 2.01, 4, 5])

        df = pd.DataFrame(dict(RA=RA, DEC=DEC))

        # The actual distance between these points is 0.01*np.sqrt(2) degrees, which is 50 arcseconds. Therefore setting this to 50 should not cause a clash

        clashes = T.find_clashes(df, df, proximity=50)

        self.assertTrue(clashes.sum() == 0.0)

    def test_recover_clashes_when_two_points_are_close(self):

        RA = np.array([1, 2, 2.01 ,4, 5])
        DEC = np.array([1, 2, 2.01, 4, 5])

        df = pd.DataFrame(dict(RA=RA, DEC=DEC))

        # The actual distance between these points is 0.01*np.sqrt(2) degrees, which is 50 arcseconds. Therefore setting this to 51 should cause a clash
        clashes = T.find_clashes(df, df, proximity=51)

        self.assertFalse(clashes.sum() == 0.0)


    def test_recover_clashes_with_single_new_point(self):

        #This is what we'll be doing in the code- adding a point and checking for a clash

        RA = np.array([1, 1.1, 0.98, 1.05, 0.97, 1.1, 1.04])
        DEC = np.array([0.0, -0.1, 0.02, -0.05, 0.3, 0.2, 0.1])
        tile_df = pd.DataFrame(dict(RA=RA, DEC=DEC))

        RA2 = np.array([1.041])
        DEC2 = np.array([0.101])
        new_point = pd.DataFrame(dict(RA=RA2, DEC=DEC2))

        # Distance between the new point is 5.09, so this shouldn't give a clash...
        clashes = T.find_clashes(tile_df, new_point, proximity=5)
        self.assertTrue(clashes.sum() == 0.0)

        #... but this should:
        clashes = T.find_clashes(tile_df, new_point, proximity=6)
        self.assertFalse(clashes.sum() == 0.0)

    def test_raises_error_when_proximity_is_negative(self):

        RA = np.array([1, 1.1, 0.98, 1.05, 0.97, 1.1, 1.04])
        DEC = np.array([0.0, -0.1, 0.02, -0.05, 0.3, 0.2, 0.1])
        tile_df = pd.DataFrame(dict(RA=RA, DEC=DEC))

        RA2 = np.array([1.041])
        DEC2 = np.array([0.101])
        new_point = pd.DataFrame(dict(RA=RA2, DEC=DEC2))

        self.assertRaises(ValueError, T.find_clashes, tile_df, new_point, proximity=-10)


class Test_get_best_tile_centre_greedy(unittest.TestCase):

    def test_get_best_tile_centre_recovers_loc_with_most_stars_in_FOV(self):

        """
        Make a cluster of targets at (10, 10), with width 0.1. Then add a few more going from 8 to 15. 
        Get_best_tile_centre with a FOV of 1 degree and a fine grid should recover (10, 10) as the best tile centre 
        """
        file_location = os.path.dirname(os.path.abspath(__file__))

        RA, DEC = np.genfromtxt(f'{file_location}/test_save_files/input_test_data/input_gaussian_cluster.txt', unpack=True)
        targets_df = pd.DataFrame(dict(RA=RA, DEC=DEC))

        centre = np.round(T.get_best_tile_centre_greedy(targets_df, 0.1, n_xx_yy=100), 1)
        self.assertTrue(np.array_equal([10, 10], centre))

        

class Test_check_if_in_FOV(unittest.TestCase):

    def test_reject_when_FOV_is_small(self):

        RA = np.array([1, 2, 2.01 ,4, 5])
        DEC = np.array([1, 2, 2.01, 4, 5])

        df = pd.DataFrame(dict(RA=RA, DEC=DEC)) 

        FOV_mask = T.check_if_in_fov(df, 2.5, 2.5, 0.0)

        self.assertTrue(FOV_mask.sum() == 0 )

    def test_raises_error_when_radius_negative(self):

        RA = np.array([1, 2, 2.01 ,4, 5])
        DEC = np.array([1, 2, 2.01, 4, 5])

        df = pd.DataFrame(dict(RA=RA, DEC=DEC)) 

        self.assertRaises(ValueError, T.check_if_in_fov, df, 2.5, 2.5, -1)


class Test_find_nearest(unittest.TestCase):

    def test_find_nearest_raises_error_with_x_y_unequal(self):
        
        RA = np.array([1, 2])
        DEC = np.array([1, 2, 2.01, 4, 5])

        xx = np.linspace(RA.min(), RA.max(), 100)
        yy = np.linspace(DEC.min(), DEC.max(), 100)

        grid_coords = np.array(list(itertools.product(xx, yy))) 

        self.assertRaises(ValueError, T.find_nearest, RA, DEC, grid_coords)

    def test_raise_error_when_grid_not_two_dimensional(self):

        RA = np.array([1, 2, 2.01, 4, 5])
        DEC = np.array([1, 2, 2.01, 4, 5])

        xx = np.linspace(RA.min(), RA.max(), 100)

        self.assertRaises(ValueError, T.find_nearest, RA, DEC, xx)

    def test_find_nearest_picks_correct_value_known_data(self):

        # Make a random sample of 10k points around (0, 0), then find the closest grid point
        # This is enough points that it _should_ always be (0, 0). 
        points = np.random.randn(10000, 2)
        RA = points[:, 0]
        DEC = points[:, 1]

        xx = np.linspace(-5, 5, 11)
        yy = np.linspace(-5, 5, 11)
        grid_coords = np.array(list(itertools.product(xx, yy))) 

        vals = T.find_nearest(RA, DEC, grid_coords)

        mode = int(Counter(vals).most_common(1)[0][0])

        self.assertTrue(np.array_equal(grid_coords[mode], np.array([0.0, 0.0])))

class Test_select_stars_for_tile(unittest.TestCase):

    def check_wrong_star_type_raises_error(self):

        star_df = pd.DataFrame(dict(RA=[0, 1, 2, 3], DEC=[0, 1, 2, 3]))
        tile_df = pd.DataFrame(dict(RA=[0], DEC=[0]))

        self.assertRaises(NameError, T.select_stars_for_tile, star_df, tile_df, 100, 10, 'wrong_value')

    def check_stars_sorted_properly_standards(self):

        star_df = pd.DataFrame(dict(RA=[0, 1, 2, 3], DEC=[0, 1, 2, 3], priority=[1, 2, 10, 3], R_MAG_AUTO=[10, 11, 10.5, 9]))
        tile_df = pd.DataFrame(dict(RA=[-1], DEC=[-1]))

        non_clashing_stars = select_stars_for_tile(star_df, tile_df, proximity=100, Nsel=10, star_type='standards')

        # This array should be sorted by priority in a _descending_ order. Check this is true...
        vals = non_clashing_stars['priority'].values
        # This is the array sorted descening by numpy
        sorted_desending = -np.sort(-star_df['priority'])

        self.assertTrue(np.array_equal(vals, sorted_desending))

    def check_stars_sorted_properly_guides(self):

        star_df = pd.DataFrame(dict(RA=[0, 1, 2, 3], DEC=[0, 1, 2, 3], priority=[1, 2, 10, 3], R_MAG_AUTO=[10, 11, 10.5, 9]))
        tile_df = pd.DataFrame(dict(RA=[-1], DEC=[-1]))

        non_clashing_stars = select_stars_for_tile(star_df, tile_df, proximity=100, Nsel=10, star_type='guides')

        # This array should be sorted by priority in a _descending_ order. Check this is true...
        vals = non_clashing_stars['R_MAG_AUTO'].values
        # This is the array sorted descening by numpy
        sorted_ascending = np.sort(star_df['R_MAG_AUTO'])

        self.assertTrue(np.array_equal(vals, sorted_ascending))


