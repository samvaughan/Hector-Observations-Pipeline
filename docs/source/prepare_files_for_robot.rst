Preparing files to be configured by the robot
==============================================

The Hector files you'll be provided with before the run contain the :math:`x` and :math:`y` positions on the field plate which correspond to the RA and Dec positions of galaxies on the sky. In a perfect world, this would be enough for the robot to place the magnets at exactly the right positions every time- but in fact, you'll need to run a script to correct these values depending on the conditions at the telescope on the night the tile will be observed. 

Step-bystep instructions on how to do this can be found in the Robot Manual (TODO: add link here). For completeness, some background information is also provided below. These corrections 

The prepare_files_for_robot.py script
--------------------------------------

You'll find a script called ``prepare_files_for_robot`` in the ``Z:\`` folder on the Windows machine used to run the robot. This script updates the positions in the Hector robot file to account for things like the temperature of the dome at the time when the roboot is configuring the plate and a number of other small effects which can impact the positioning at the level of a few tens of microns. Most of these small corrections are caused the design of the robot itself, and require us to account for these imperfections when placing the magnets. In essence, we tell the robot to place the magnet at a slightly incorrect position in the knowledge that by doing so it will end up exactly where we want it. 

The metrology Correction
~~~~~~~~~~~~~~~~~~~~~~~~

The largest change to the plate positions come from something we're calling the "metrology" correction. There is a script in Labview which tells the robot to go and find a number of tiny holes in each corner of the bench. These "metrology markers" have been drilled to sub-micron precision, and by seeing how far the robot thinks it has to drive in order to centre above each one, we can account for things like the thermal expansion of the robot's components due to changes in temperature and the slight mis-alignment between the robot gantry and the :math:`x` and :math:`y` coorrdinates of the field plate. 

You can run a metrology calibration from Labview by following the instructions in the Robot Manual. This will create a file called 'robot_shifts_YYYYMMDDHHMMSS.csv' based on today's date and time, which should be passed to the ``prepare_files_for_robot.py`` file

The roll correction
~~~~~~~~~~~~~~~~~~~~

The torque on the robot arm is different when placing a magnet on the left-hand side of the plate compared to the right-hand side of the plate, since it is further from its gantry. We correct for this effect using a precisely fitted polynomial, derived from tests in the lab back in Sydny.

The telecentricity correction
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The geometry of having light enter the hexabundle at an angle moves the apparent position of the galaxy with respect to the centre of the magnet. We therefore apply a radial correction to the circular magnet's position to account for this, moving its corresponding rectangular magnet at the same time.

The robot arm miscentring correction
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The axis of rotation of the robot arm is slightly offset from the centre of the pick-up arm. We apply a correction to account for this which depends on the rotation of the magnet when it is placed on the field plate. 

Usage
-----

From a powershell terminal on the Windows PC, run the following:

.. code-block:: bash
	
	python prepare_files_for_robot.py robot_file_name metrology_calibration_file_name

This will create a file with the same name as the input robot file with ``_CorrectionsApplied.csv`` appended to the end. This is the file which you should load into the Robot PC and use to configure a field. 