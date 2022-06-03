Checking two plates can be observed after each other
=====================================================

The Hector robot has a large but limited number of circular magnets corresponding to each telecentricity annulus: 

* 25 Blue magnets in the central telecentricity radius
* 27 Green magnets in the second annulus
* 31 Yellow magnets in the third annulus
* 24 Magneta magnets in the outer annulus

Very rarely, it may not be possible to have two plates configured at the same time if together they require more of a certain kind of magnet than we have available. This should have been flagged as an issue before the run, but if you're making changes on-the-fly at the telescope it's something to be mindful of. 

The script ``check_magnet_counts_between_tiles`` counts the number of circular magnets in each tile and makes sure we're not going to run out of a specific colour. 

Usage 
-----
The code uses the robot files from each plate to make the comparison. From the command line, run

.. code-block:: bash

	check_magnet_counts_between_tiles robot_file_1 robot_file_2

An example output is:

.. code-block :: console

	Telecentricity Annulus: Blue
		Tile 1 has 4, Tile 2 has 6
		Total used: 10 / 25
	Telecentricity Annulus: Green
		Tile 1 has 7, Tile 2 has 6
		Total used: 13 / 27
	Telecentricity Annulus: Yellow
		Tile 1 has 10, Tile 2 has 12
		Total used: 22 / 31
	Telecentricity Annulus: Magenta
		Tile 1 has 6, Tile 2 has 3
		Total used: 9 / 24

So these two plates can be configured and observed one after the other. If two tiles cannot be configured at the same time, the script will raise a ``ValueError`` and exit. 