# Image-Blender
Blends two .bmp images together

## How to use
After compiling, run

    program_name image_file_1 image_file_2 ratio output_file
  
 Where program_name is the name of the compiled program,  
 image_file_1 and image_file_2 are bmp images that both have larger widths than heights,  
 ratio is a float between 0.0 and 1.0 that determines how much of each image is shown (0.5 is equally both, 0.0 and 1.0 show one image),  
 and output_file is a new file with the blended image.
