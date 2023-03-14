# LDrawConverter
converts LDraw files to fbx


## Compilation & Usage
- Install the [FBX SDK](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-0) if you haven't already.
- Use cmake to compile the project, you may need to change the FBX_SDK_PATH in the CMakeLists.txt file.
- Download the complete.zip from [LDraw.org](https://www.ldraw.org/part-updates) and extract the files. Optionally add the unofficial library into the library folder (where parts/ and p/ is located).
(don't use the library that studio came with, some faces have the wrong direction)
- additional export settings can be found in the LDrawExporter.hpp

## Goal
#### Exporting ldraw files to fbx, optimized for games/ rendering.
I started this project because of certain features that i needed to deeply integrate into an exporter, namely adding an edge crease whilst not actually adding gaps to the mesh, automating the creation of skeletons not only for rigging characters but also snapping bricks together(maybe in vr...), and optimizing meshes.

## Features
- Merge to a custom filetype(multipart, submodel, part, primitive) depth 
- Export with instancing
- custom part sizes (scaling down for gaps)
- simple file dialog for file selection
- support for the unofficial LDraw library

## TODO
- support for other export formats supported by the fbx sdk(obj, collada)
- set export settings without recompiling
- mesh optimizing
  - remove occluded studs
  - remove occluded vertices alltogether
- more accurate materials
- face smoothing
- edge beveling
- advanced edge beveling with detection to avoid leaving gaps
- using submodel names for automatic rigging (minifigurines, vehecles, ...)
- add bones for connectors
- make unreal use correct materials (emission, transparency, ...), not just the raw color

## Troubleshooting
- if there are missing files try to dig around in the ldraw library that came with your ldraw file editor and copy paste the missing file into your unofficial ldraw library folder (in part/ or p/ accordingly)
- if colors are missing during export and you used studio check if you used any "render only" colors and change those to colors without a (i) logo
- currently instanced meshes with different colors only work in unreal, if you want to use this with blender or just view it in windows' 3D viewer either disable instancing(caching) in the LDrawExporter.hpp or set the export depth to Multipart or Submodel (which works because only part and primitive have different colors which means that all instances are the same), this problem might be solved entirely by changing the fbx sdk version
