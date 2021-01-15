# #!/usr/bin/env python3

"""
A pipeline for Hector observations 
"""

from hop import pipeline 


HP = pipeline.HectorPipe(config_filename="ConfigFiles/example_config.yaml")


HP.load_input_catalogue()
HP.tile_field(configure_tiles=True, apply_distortion_correction=True)

#HP.allocate_hexabundles_for_single_tile(tile_number=0)