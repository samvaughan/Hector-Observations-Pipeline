Plotting a plate configuration
==============================

The script plot_plate_configuration will make an interactive plot of the plate configuration of a given robot file. This can be saved as a pdf. This is primarily useful for debugging things, especially the angles of the circular magnets. 

Usage
-----

Call ``plot_plate_configuration`` followed by the name of a Hector robot file:

.. code-block:: bash

	plot_plate_configuration /path/to/robot/file

This will make a plot like the following:

.. image:: images/plot_plate_configuration_example.png
  :width: 600
  :align: center
  :alt: A Hector plate as plotted by the plot_plate_configuration script

Note that this image is in *robot* coordinates, with the :math:`y` axis increasing to the right and the :math:`x` axis increasing downwards. This plot matches the the layout of the plate as you look at it on the telescope. 

The plot can be saved by passing the ``--save`` flag followed by a filename. 