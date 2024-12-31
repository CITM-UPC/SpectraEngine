# SpectraEngine

## Description
**SpectraEngine** is a 3D game engine developed in **C++** with **OpenGL**, currently capable of rendering geometry with a range of controls and information displays.

The project is under active development, and the code is available on [GitHub](https://github.com/CITM-UPC/SpectraEngine).

## Team Members
- [Guillem Alqueza Mayench](https://github.com/guillemalqueza)
- [Miguel Iglesias Abarca](https://github.com/MiguelIglesiasAbarca)
- [Lluc Cid](https://github.com/Lluccib)

## Viewer Controls
- **Right-click**: Move the current view.
- **Right-click + WASD**: Fly around the scene.
- **Alt + Right-click or Mouse Wheel**: Zoom in or out.
- **Alt + Left-click**: Orbit the camera around a pivot point.
- **Alt + Mouse Wheel**: Pan the camera.
- **F**: Frame the selected object.

## Features
- **Model Import**: Import models in FBX format.
- **Texture Import**: Supports PNG, DDS and TGA texture formats.
- **Drag and Drop**: Allows importing files by dragging them directly.
- **Transform Controls**: Apply transformations like position, rotation, and scale to models.
- **Logging System**: Provides detailed logging.
- **Custom File Library**: Manage files with a custom file format.
- **Shading Modes**: Swap between Shaded, Wireframe & Shaded Wireframe options.
- **Engine Monitor**: Visualize monitoring information.
- **Octree**: Efficiently manage and optimize scenes using space partitioning.
- **Frustum Culling** Cull objects outside the camera's viewing frustum to improve performance.
- **Time Management**: Handle game time.
- **Mouse Picking**: Select objects in the scene using mouse.

## Panels
- **Hierarchy**: Displays all game objects currently present in the scene. Includes a search feature to locate specific game objects and provides options to create empty game objects and basic primitive shapes.
- **Inspector**: Allows viewing and editing the properties of a currently selected game object, such as transform, mesh renderer, and material options.
- **Scene**: Allows visualizing and interacting with the world you create in the editor.
- **Game**: View game camera and play game.
- **Project**: Access various project folders, manage files, and import assets. Displays all assets within the project, including models, textures, and other engine resources.
- **Console**: View errors, warnings, and information related to the engine. Allows filtering by type and clearing the console.
- **Resources**: Manage the engine's resources.
- **Octree**: Visualize octree views and offers settings for the octree.
- **Preferences**: Offers settings for the user, including screen, grid, and rendering options.
- **Performance**: Displays information about engine performance and its impact on system resources.
- **About**: Displays basic information about the engine.

## Libraries and Dependencies
- [SDL2](https://github.com/libsdl-org/SDL): Cross-platform development library.
- [OpenGL](https://www.opengl.org/): Graphics API for rendering graphics.
- [ImGui](https://github.com/ocornut/imgui): Immediate mode GUI for C++.
- [GLM](https://github.com/g-truc/glm): C++ mathematics library for graphics software based on OpenGL.
- [Assimp](https://github.com/assimp/assimp): Library to load various 3D file formats into a shared, in-memory immediate format.
- [DeviL](https://github.com/DentonW/DevIL): Image library supporting loading, saving, and manipulating various image formats.
