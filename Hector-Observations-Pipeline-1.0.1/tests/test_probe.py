import pytest
from hypothesis import given, strategies as st
import numpy as np
import pandas as pd
from pathlib import Path
import pickle
from mock import patch
import inspect

from hop.hexabundle_allocation.hector import probe


@pytest.fixture(scope='module')
def mock_probe():

    # Mock the probe class so we cant test things easily
    with patch.object(probe.probe, "__init__", lambda x: None):
        mock_probe = probe.probe()

    yield mock_probe



class Test_probe_functions:


    @given(magnet_centre=st.tuples(st.floats(min_value=0.0, max_value=1000.0), st.floats(min_value=0.0, max_value=1000.0)))
    def test_circular_magnet_centre_orientation(self, mock_probe, magnet_centre):

        mock_probe.circular_magnet_center = magnet_centre

        circular_magnet_orientation = mock_probe.calculate_circular_magnet_orientation()

        # Work out the true value using np.arctan2
        # Change the angle of a magnet at (0.0, -0.0) to be 0 rather than 180
        true_value = (np.degrees(np.arctan2(magnet_centre[1], magnet_centre[0]))) % 360
        if (magnet_centre[1] == 0.0) & (magnet_centre[0] == -0.0):
            true_value = 0.0
        print(f"Magnet Centre is {magnet_centre}, code value is {circular_magnet_orientation}, true value is {true_value}")
        assert np.allclose(circular_magnet_orientation, true_value)


