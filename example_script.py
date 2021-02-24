# #!/usr/bin/env python3

"""
A pipeline for Hector observations 
"""

from hop import pipeline 


HP = pipeline.HectorPipe(config_filename="ConfigFiles/GAMA_G09.yaml")


HP.load_input_catalogue()
HP.df_targets['PRI_SAMI'] = 8

HP.tile_field(configure_tiles=False, apply_distortion_correction=True, check_sky_fibres=False)

#HP.allocate_hexabundles_for_single_tile(tile_number=0)