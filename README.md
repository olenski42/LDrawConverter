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
- Export as single Mesh
- Export with instanced parts (aka. bricks)
- Export with fbx nodes for every part (use this if you want to use your file in blender. Alternitivly try to change the fbx version in the sdk to use instancing)

## TODO
- mesh optimizations
- more accurate materials
- edge beveling
- edge smoothing
- using submodel names for automatic rigging for minifigurines, vehecles, ...
- using submodel names for defining which models to create (e.g. creating a modular building in studio and exporting all of the submodels as single files)
