# #!/usr/bin/env python3

"""
A pipeline for Hector observations 
"""

from hop import pipeline 


HP = pipeline.HectorPipe(config_filename="ConfigFiles/Ayoan_example_config_Cluster2.yaml")

HP.allocate_hexabundles_for_single_tile(tile_number=21,plot=True)



