# STL Renderer Documentation

This document provides a detailed description of the structure and functionality of the STL Renderer project.

## Project Structure

```
STL Renderer/
├── src/             # Main source code
│   ├── app.cpp      # Main application logic
│   ├── app.h        # App class definition
│   ├── config.cpp   # Configuration management
│   ├── config.h     # Configuration structure definition
│   ├── gui.cpp      # Graphical interface implementation
│   ├── gui.h        # GUI class definition
│   ├── main.cpp     # Application entry point
│   ├── renderer.cpp # 3D renderer implementation
│   └── renderer.h   # Renderer class definition
├── glad/            # GLAD library for OpenGL
├── external/        # External libraries (ImGui, stb_image, etc.)
├── build/           # Build directory
└── CMakeLists.txt   # Build configuration
```

## Main Components

### App Class (`src/app.cpp`, `src/app.h`)
- Manages the application lifecycle
- Processes command line arguments
- Coordinates STL file rendering
- Implements functions for rendering individual files and entire directories

### Config Class (`src/config.cpp`, `src/config.h`)
- Defines the application configuration structure
- Manages loading and saving configurations
- Stores parameters such as colors, camera configuration, and rendering options

### GUI Class (`src/gui.cpp`, `src/gui.h`)
- Implements the graphical interface using ImGui
- Handles user interactions (menus, buttons, etc.)
- Provides a preview of the 3D model
- Implements drag and drop callback for files

### Renderer Class (`src/renderer.cpp`, `src/renderer.h`)
- Implements OpenGL rendering
- Loads and processes STL models
- Sets up shaders and buffers for rendering
- Provides functions for rendering to window and to file

## Execution Flows

### GUI Mode
1. The user starts the application without arguments
2. The GUI is initialized and the main window is displayed
3. The user can load STL files, configure options, and render

### Command Line Mode
1. The user starts the application with arguments (STL file paths)
2. The application processes each provided STL file
3. PNG files are generated for each STL model
4. The application terminates after processing all files

### Drag and Drop
1. The user drags STL files to the executable
2. The application processes each dragged file
3. PNG files are generated for each model

## Key Functions

### Rendering
- `renderSingleFile` in `src/app.cpp`: Renders a single STL file to PNG
- `renderDirectory` in `src/app.cpp`: Processes multiple STL files in a directory
- `renderToFile` in `src/renderer.cpp`: Main render-to-file function
- `renderPreviewWindow` in `src/gui.cpp`: Renders the preview in the GUI

### File Handling
- `dropCallback` in `src/gui.cpp`: Handles drag and drop events
- `loadModelFromFile` in `src/renderer.cpp`: Loads STL models

### Camera Configuration
- Camera configuration is handled in `src/renderer.cpp`
- Functions to adjust camera position, zoom, and rotation

## Maintenance Notes

### GUI Modification
When modifying the GUI, make sure to:
1. Maintain visual consistency
2. Not alter the behavior of existing functionalities
3. Test changes in both GUI mode and command line mode

### Renderer Modification
When modifying the renderer:
1. Ensure changes are reflected in both the preview and final rendering
2. Maintain compatibility with existing STL formats
3. Test performance with complex models

### Adding New Features
1. Document the new functionality in this file
2. Update README.md if it's a user-visible feature
3. Maintain backward compatibility when possible 