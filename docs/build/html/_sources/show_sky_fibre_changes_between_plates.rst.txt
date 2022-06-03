Showing the Sky Fibre Changes between Plates
=============================================

The script ``show_sky_fibre_changes_between_plates`` will make a plot showing which sky fibres need to be moved when changing from plate A to plate B. It accepts two Hector tile files as arguments.

.. code-block:: bash
	
	show_sky_fibre_changes_between_plates Tile_file_on_Telescope Tile_file_to_change_to

This code shows a plot like the one below:

.. image:: images/show_sky_fibre_changes_example.png
  :width: 600
  :align: center
  :alt: The output of the show_sky_fibre_changes_between_plates script

This plot highlights in yellow the sky fibres which need to be changed. Sky fibres which don't need to be changed are in black. The numbers around the edge of the plot correspond to the required sky fibres for the new plate.  