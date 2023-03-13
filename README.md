# LDrawConverter
converts LDraw files to fbx


## Compilation & Usage
- Install the [FBX SDK](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-0) if you haven't already.
- Use cmake to compile the project, you may need to change the FBX_SDK_PATH in the CMakeLists.txt file.
- Download the complete.zip from [LDraw.org](https://www.ldraw.org/part-updates) and extract the files
    (if you have Studio2.0 installed you can also use it's library, which is commonly found at "C:/Program Files/Studio 2.0/ldraw").
- Edit the main.cpp and set the correct paths. (This is a WIP, a file dialog will (maybe) follow)
- Set your export settings in the LDrawExporter header file
- currently instanced meshes with different colors only work in unreal, if you want to use this with blender or just view it in windows' 3D viewer either disable instancing(caching) or set the export depth to Multipart or Submodel (which works because only part and primitive have different colors), this problem might be solved entirely by changing the fbx sdk version

## Goal
###Exporting ldraw files to fbx, optimized for games/ rendering.
I started this project because of certain features that just could not be created using other exporters, namely adding an edge crease whilst not actually adding gaps to the mesh, automating the creation of skeletons and removing occluded vertices

## Features
- Merge to a custom filetype(multipart, submodel, part, primitive) depth 
- Export with instancing
- custom part sizes (e.g. scaling all slightly down for gaps, not wanted in games tho, edge beveling preferred for lighting and unreals nanite as gaps increase rendered meshes)

## TODO
- support for the unofficial LDraw library
- set export settings and file paths without recompiling
- remove occluded studs
- remove occluded faces alltogether
- more accurate materials
- face smoothing
- edge beveling
- advanced edge beveling with detection to avoid leaving gaps
- using submodel names for automatic rigging (minifigurines, vehecles, ...)
- using submodel names for defining which models to create (e.g. creating a modular building in studio and exporting all of the submodels as single files)
- make unreal use correct materials (emission, transparency, ...), not just the color data
