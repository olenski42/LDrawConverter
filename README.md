# LDrawConverter
converts LDraw files to fbx

**WIP, instancing causes materials to be mapped incorrectly in blender, in unreal it is correct tho. (probably too new fbx version for blender)**

## Compilation & Usage
- Install the [FBX SDK](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-0) if you haven't already.
- Use cmake to compile the project, you may need to change the FBX_SDK_PATH in the CMakeLists.txt file.
- Download the complete.zip from [LDraw.org](https://www.ldraw.org/part-updates) and extract the files
    (if you have Studio2.0 installed you can also use it's library, which is commonly found at "C:/Program Files/Studio 2.0/ldraw").
- Edit the main.cpp and set the correct paths. (This is a WIP, a file dialog will (maybe) follow)

## Goal
Exporting ldraw files to fbx, optimized for games/ rendering and automating the creation of skeletons.

## Features
- Merge to a custom merge depth (multipart, subpart, part, primitive)
- Export with instanced parts (or other file types, just not recommended for small meshes)
- Export with fbx nodes for every part (use this if you want to use your file in blender. Alternitivly try to change the fbx version in the sdk to use instancing)
- custom part sizes (e.g. scaling all slightly down for gaps, not wanted in games tho, edge beveling preferred for lighting and unreals nanite as gaps increase rendered meshes)

## TODO
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
