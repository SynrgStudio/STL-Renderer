# Documentación de STL Renderer (Desactualizado)

> **NOTA**: Este documento ha sido reemplazado por la versión en inglés [DOCUMENTATION.md](DOCUMENTATION.md). Por favor, consulte ese archivo para obtener la documentación más actualizada.

Este documento proporciona una descripción detallada de la estructura y funcionalidad del proyecto STL Renderer.

## Estructura del Proyecto

```
STL Renderer/
├── src/             # Código fuente principal
│   ├── app.cpp      # Lógica principal de la aplicación
│   ├── app.h        # Definición de la clase App
│   ├── config.cpp   # Manejo de configuración 
│   ├── config.h     # Definición de la estructura de configuración
│   ├── gui.cpp      # Implementación de la interfaz gráfica
│   ├── gui.h        # Definición de la clase GUI
│   ├── main.cpp     # Punto de entrada de la aplicación
│   ├── renderer.cpp # Implementación del renderizador 3D
│   └── renderer.h   # Definición de la clase Renderer
├── glad/            # Biblioteca GLAD para OpenGL
├── external/        # Bibliotecas externas (ImGui, stb_image, etc.)
├── build/           # Directorio de compilación
└── CMakeLists.txt   # Configuración de compilación
```

## Componentes Principales

### Clase App (`src/app.cpp`, `src/app.h`)
- Maneja el ciclo de vida de la aplicación
- Procesa los argumentos de línea de comandos
- Coordina el renderizado de archivos STL
- Implementa funciones para renderizar archivos individuales y directorios completos

### Clase Config (`src/config.cpp`, `src/config.h`)
- Define la estructura de configuración de la aplicación
- Gestiona la carga y guardado de configuraciones
- Almacena parámetros como colores, configuración de cámara y opciones de renderizado

### Clase GUI (`src/gui.cpp`, `src/gui.h`)
- Implementa la interfaz gráfica utilizando ImGui
- Maneja interacciones del usuario (menús, botones, etc.)
- Proporciona una vista previa del modelo 3D
- Implementa el callback de arrastrar y soltar para archivos

### Clase Renderer (`src/renderer.cpp`, `src/renderer.h`)
- Implementa la renderización OpenGL
- Carga y procesa modelos STL
- Configura shaders y buffers para el renderizado
- Proporciona funciones para renderizar a ventana y a archivo

## Flujos de Ejecución

### Modo GUI
1. El usuario inicia la aplicación sin argumentos
2. Se inicializa la GUI y se muestra la ventana principal
3. El usuario puede cargar archivos STL, configurar opciones y renderizar

### Modo Línea de Comandos
1. El usuario inicia la aplicación con argumentos (rutas de archivos STL)
2. La aplicación procesa cada archivo STL proporcionado
3. Se generan archivos PNG para cada modelo STL
4. La aplicación termina después de procesar todos los archivos

### Arrastrar y Soltar
1. El usuario arrastra archivos STL al ejecutable
2. La aplicación procesa cada archivo arrastrado
3. Se generan archivos PNG para cada modelo

## Funciones Clave

### Renderizado
- `renderSingleFile` en `src/app.cpp`: Renderiza un solo archivo STL a PNG
- `renderDirectory` en `src/app.cpp`: Procesa múltiples archivos STL en un directorio
- `renderToFile` en `src/renderer.cpp`: Función principal de renderizado a archivo
- `renderPreviewWindow` en `src/gui.cpp`: Renderiza la vista previa en la GUI

### Manejo de Archivos
- `dropCallback` en `src/gui.cpp`: Maneja los eventos de arrastrar y soltar
- `loadModelFromFile` en `src/renderer.cpp`: Carga modelos STL

### Configuración de Cámara
- La configuración de cámara se maneja en `src/renderer.cpp`
- Funciones para ajustar posición, zoom y rotación de la cámara

## Notas de Mantenimiento

### Modificación de la GUI
Al modificar la GUI, asegúrate de:
1. Mantener la coherencia visual
2. No alterar el comportamiento de funcionalidades existentes
3. Probar los cambios tanto en modo GUI como en línea de comandos

### Modificación del Renderizador
Al modificar el renderizador:
1. Asegurarse de que los cambios se reflejen tanto en la vista previa como en el renderizado final
2. Mantener la compatibilidad con formatos STL existentes
3. Probar el rendimiento con modelos complejos

### Adición de Nuevas Características
1. Documentar la nueva funcionalidad en este archivo
2. Actualizar el README.md si es una característica visible para el usuario
3. Mantener la compatibilidad con versiones anteriores cuando sea posible 