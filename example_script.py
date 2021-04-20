# #!/usr/bin/env python3

"""
A pipeline for Hector observations 
"""

from hop import pipeline 


HP = pipeline.HectorPipe(config_filename="ConfigFiles/GAMA_G09.yaml")


HP.load_input_catalogue()
HP.df_targets['priority'] = 1
HP.df_guide_stars.rename(columns=dict(CoADD_ID='ID'), inplace=True)
HP.df_standard_stars.rename(columns=dict(CoADD_ID='ID'), inplace=True)

HP.tile_field(configure_tiles=False, apply_distortion_correction=True, check_sky_fibres=False)

#HP.allocate_hexabundles_for_single_tile(tile_number=0)