# LDrawConverter
converts LDraw files to fbx

**WIP!**

## Compilation & Usage
- Install the [FBX SDK](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-0) if you haven't already.
- Use cmake to compile the project, you may need to change the FBX_SDK_PATH in the CMakeLists.txt file.
- Download the complete.zip from [LDraw.org](https://www.ldraw.org/part-updates) and extract the files
    (if you have Studio2.0 installed you can also use it's library, which is commonly found at "C:/Program Files/Studio 2.0/ldraw").
- Edit the main.cpp and set the correct paths. (This is a WIP a file dialog will (maybe) follow)