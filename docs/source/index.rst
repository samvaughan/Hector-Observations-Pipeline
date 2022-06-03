.. Hector Observations Pipeline documentation master file, created by
   sphinx-quickstart on Fri Jun  3 17:42:59 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to the Hector Observations Pipeline's documentation!
============================================================

The Hector Observations Pipeline is a package to make Hector Galaxy tiles. It also contains useful scripts and tools for observers to run at the telescope. This documentation describes these tools for astronomers; questions about the tiling pipeline should be directed to Sam Vaughan.

Installation
------------

The Hector Observation Pipeline is already installed on the data reduction PC at the AAT. To install it on your own laptop, use pip:

.. code-block:: console

   $ pip install Hector-Observations-Pipeline


Tools for Observers
--------------------

The package contains a number of scripts to help with observing. Each script runs on either a Hector "tile" file or a Hector "robot" file, which will be provided before the run and can probably be found in the ``/instoft/hector_configs`` directory on ``aatlxe`` or ``aatliu``.  The scripts provide (hopefully!) helpful information if you call them from the commmand line with the ``-h`` option.

We have scripts to help with the following tasks:

* :doc:`calculating_offsets`
* :doc:`plot_plate_configuration`
* :doc:`show_sky_fibre_changes_between_plates`
* :doc:`check_magnet_counts_between_tiles`

Preparing files for the Hector robot
-------------------------------------

You'll also find a script from this package installed on the Hector PC which is used to apply corrections to the magnet positions in a given Hector robot file. Full instructions on how and when to use this script can be found in the Hector robot manual. More details for the interested user can be found :ref:`here <Preparing files to be configured by the robot>`.


Issues, bugfixes and feedback
------------------------------

The best way to raise an issue with the code is to go to the project's `github page <https://github.com/samvaughan/Hector-Observations-Pipeline>`_. Click on the 'issues' tab followed by 'new issue' on the right. Pull requests to fix something or add a feature are also especially appreciated!  

Acknowledgements 
-----------------

The Hector Observations Pipeline is written and maintained by Sam Vaughan. Caroline Foster, Keith Shortridge and Ayoan Sadman each made major and substantial contributions. Caro wrote the code to find the best layout of the magnets on the Hector field plate to avoid clashes and collisions. Keith wrote the code to turn an RA and Dec into a position on the plate, as well as applying the optical distortion model. Ayoan wrote the first version of the code to allocate hexabundles to each target and sorted out the order in which magnets are placed on the plate by the robot. Hector wouldn't be possible without each of their contributions!

Index
======
.. toctree::
   :maxdepth: 2

   calculating_offsets
   plot_plate_configuration
   show_sky_fibre_changes_between_plates
   check_magnet_counts_between_tiles
   prepare_files_for_robot

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`


