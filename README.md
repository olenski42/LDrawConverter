# LDrawConverter
converts LDraw files to fbx


## Compilation & Usage
- Install the [FBX SDK](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-0) if you haven't already.
- Use cmake to compile the project, you may need to change the FBX_SDK_PATH in the CMakeLists.txt file.
- Download the complete.zip from [LDraw.org](https://www.ldraw.org/part-updates) and extract the files
(don't use the library studio came with, some faces have the wrong direction)
- currently instanced meshes with different colors only work in unreal, if you want to use this with blender or just view it in windows' 3D viewer either disable instancing(caching) or set the export depth to Multipart or Submodel (which works because only part and primitive have different colors), this problem might be solved entirely by changing the fbx sdk version
- additional export settings can be found in the LDrawExporter header

## Goal
#### Exporting ldraw files to fbx, optimized for games/ rendering.
I started this project because of certain features that i could not add to other exporters, namely adding an edge crease whilst not actually adding gaps to the mesh, automating the creation of skeletons and removing occluded vertices

## Features
- Merge to a custom filetype(multipart, submodel, part, primitive) depth 
- Export with instancing
- custom part sizes (scaling down for gaps)
- simple file dialog for file selection

## TODO
- support for the unofficial LDraw library
- set export settings without recompiling
- remove occluded studs
- remove occluded faces alltogether
- more accurate materials
- face smoothing
- edge beveling
- advanced edge beveling with detection to avoid leaving gaps
- using submodel names for automatic rigging (minifigurines, vehecles, ...)
- using submodel names for defining which models to create (e.g. creating a modular building in studio and exporting all of the submodels as single files)
- make unreal use correct materials (emission, transparency, ...), not just the color data
