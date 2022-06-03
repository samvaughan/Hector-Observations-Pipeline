Calculating Offsets from one Hexabundle to another
===================================================

The script ``Hector_Offsets`` will calculate the required telescope offsets to move a target between two Hexabundles. It can also be used to calculate the offset from the centre of a plate to a given Hexabundle. This is useful for performing SNAFUs at the telescope, for example, when we generally don't have a guide bundle located exactly in the middle of the plate.

Usage
-----

From the command line, call ``Hector_Offsets`` followed by the name of a Hector tile file and two hexabundle names:

.. code-block:: bash

	Hector_Offsets /path/to/Tile/file GS1 GS2

where ``GS1`` refers to guide bundle 1 and ``GS2`` refers to guide bundle 2. Other hexabundles are referred to by their letter. 

To find the offset between the centre of the plate and a given hexabundle, use the string 'centre':

.. code-block:: bash

	Hector_Offsets /path/to/Tile/file centre A


The output will be something like the following:

.. code-block:: console

	Moving from the centre of the plate to Hexabundle GS1:
		--> offset 3.7 arcseconds S and 0.1 arcseconds E

Note that these offsets are in the correct direction to read out to the night assistant- e.g. in this case, Hexabundle GS1 is actually North and West of the centre, so the telescope must be moved South and East.


Details
-------

The code uses the legacy function ``plate2sky`` from the ``sami`` package to convert a distance in millimetres on the plate to offsets in RA/DEC. Whilst it does a bit better than using a simple linear plate scale, we've found it to only be approximately correct for large offsets (e.g. from one side of the plate to another). Use with caution!
