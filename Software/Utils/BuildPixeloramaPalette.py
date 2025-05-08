#
# Utils/BuildPixeloramaPalette.py
#
# This file is part of Portatil source code.
# Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
#

import math
import json

palette_data = {
    "comment": "Portatil Default Palette",
    "width": 16,
    "height": 16,
    "colors": []
}

def build_palette():
    min_values = [
        0, 0, 0,      
        32, 0, 0,     
        32, 8, 0,     
        32, 16, 0,   
        32, 16, 0,   
        32, 32, 0,   
        16, 32, 0,    
        0, 32, 0,     
        0, 32, 16,    
        0, 32, 32,    
        0, 16, 32,    
        0, 0, 32,     
        8, 0, 32,     
        16, 0, 32,    
        32, 0, 32,    
        32, 0, 16,    
    ]

    mid_values = [
        128, 128, 128,    
        255, 0, 0,        
        255, 64, 0,       
        255, 128, 0,      
        255, 192, 0,      
        255, 255, 0,      
        128, 255, 0,      
        0, 255, 0,        
        0, 255, 128,      
        0, 255, 255,      
        0, 128, 255,      
        0, 0, 255,        
        64, 0, 255,       
        128, 0, 255,      
        255, 0, 255,      
        255, 0, 128,      
    ]

    max_values = [
        255, 255, 255,    
        255, 224, 224,    
        255, 224, 224,    
        255, 240, 224,    
        255, 255, 224,    
        255, 255, 224,    
        240, 255, 224,    
        224, 255, 224,    
        224, 255, 240,    
        224, 255, 255,    
        224, 240, 255,    
        224, 224, 255,    
        240, 224, 255,    
        240, 224, 255,    
        255, 224, 255,    
        255, 224, 240,    
    ]

    color_index = 0

    for row_index in range(0, 16):
        redStep   = (mid_values[row_index * 3] - min_values[row_index * 3]) / 7.0
        greenStep = (mid_values[row_index * 3 + 1] - min_values[row_index * 3 + 1]) / 7.0
        blueStep  = (mid_values[row_index * 3 + 2] - min_values[row_index * 3 + 2]) / 7.0

        for column_index in range(0, 8):
            palette_data["colors"].append({
                "color": "(%.1f, %.1f, %.1f, 1)" % (
                    (min_values[row_index * 3] + math.floor(column_index * redStep)) / 255.0,
                    (min_values[row_index * 3 + 1] + math.floor(column_index * greenStep)) / 255.0,
                    (min_values[row_index * 3 + 2] + math.floor(column_index * blueStep)) / 255.0
                ),
                "index": color_index
            })
            color_index += 1

        redStep   = (max_values[row_index * 3] - mid_values[row_index * 3]) / 8.0
        greenStep = (max_values[row_index * 3 + 1] - mid_values[row_index * 3 + 1]) / 8.0
        blueStep  = (max_values[row_index * 3 + 2] - mid_values[row_index * 3 + 2]) / 8.0

        for column_index in range(1, 9):
            palette_data["colors"].append({
                "color": "(%.1f, %.1f, %.1f, 1)" % (
                    (mid_values[row_index * 3] + math.floor(column_index * redStep)) / 255.0,
                    (mid_values[row_index * 3 + 1] + math.floor(column_index * greenStep)) / 255.0,
                    (mid_values[row_index * 3 + 2] + math.floor(column_index * blueStep)) / 255.0
                ),
                "index": color_index
            })
            color_index += 1

build_palette()

paletteJson = json.dumps(palette_data, indent=4)

with open("Portatil.Pixelorama.json", "w") as paletteFile:
    paletteFile.write(paletteJson)