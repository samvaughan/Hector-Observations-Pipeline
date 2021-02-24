[![Build Status](https://travis-ci.com/samvaughan/Hector-Observations-Pipeline.svg?branch=master)](https://travis-ci.com/samvaughan/Hector-Observations-Pipeline)
[![Coverage Status](https://coveralls.io/repos/github/samvaughan/Hector-Observations-Pipeline/badge.svg?branch=master)](https://coveralls.io/github/samvaughan/Hector-Observations-Pipeline?branch=master)

# The Hector Galaxy Survey Observation Pipeline

### From an input catalogue to a list of hexabundle positions

This pipeline:

* creates a catalogue of galaxies to observe from a master catalogue (or can be given an input catalogue)
* assigns galaxies to individual survey tiles, avoiding clashes
* converts the RA and DEC of objects on the sky to xy coordinates on the TwoDF field plate
* configures the positions of the Hector hexabundles so they don't clash with each other
* assigns which hexabundle size will be used to observe each galaxy
* creates final output files which can be read in by the robot software to actually position each hexabundle on the plate
