cmake_minimum_required(VERSION 3.15)
project(SparkLEPlugin VERSION 0.1.0)

# Incluir JUCE como submódulo
add_subdirectory(JUCE)

# Habilitar la generación automática de JuceHeader.h
set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON)

# Inicializar la configuración de JUCE
juce_add_plugin(SparkLEPlugin
    VERSION 0.1.0
    COMPANY_NAME "SparkLE" # Cámbialo por tu nombre o empresa
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    PLUGIN_MANUFACTURER_CODE Sple  # Un código de 4 caracteres para identificar al fabricante
    PLUGIN_CODE Spk1               # Un código de 4 caracteres para identificar el plugin
    FORMATS VST3 Standalone        # Los formatos que deseas soportar
    PRODUCT_NAME "Spark LE Custom Plugin"
    VST3_CATEGORIES "Fx" # Categoría VST3 - importante para reconocimiento
    COPY_PLUGIN_AFTER_BUILD FALSE   # No copiar automáticamente - lo haremos manualmente
)

# Añade los archivos fuente
target_sources(SparkLEPlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/SequencerComponent.cpp
    Source/MidiHandler.cpp)

# Configura el destino
target_compile_definitions(SparkLEPlugin
    PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

# Añadir la ruta a JuceLibraryCode para que pueda encontrar JuceHeader.h
target_include_directories(SparkLEPlugin 
    PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}/JuceLibraryCode)

# Enlaza con los módulos JUCE necesarios
target_link_libraries(SparkLEPlugin
    PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_audio_devices
    juce::juce_audio_basics
    juce::juce_gui_extra
    juce::juce_gui_basics
    juce::juce_graphics
    juce::juce_data_structures
    juce::juce_events
    juce::juce_core
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

# Copia los binarios a una carpeta específica después de la compilación
set_target_properties(SparkLEPlugin_VST3 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/VST3")

# Opcional: Configuración específica para Windows
if(WIN32)
    set_property(TARGET SparkLEPlugin APPEND_STRING PROPERTY LINK_FLAGS " /IGNORE:4099")
    
    # Comentamos la copia automática que requiere privilegios de administrador
    # get_target_property(JUCE_VST3_COPY_DIR SparkLEPlugin_VST3 RUNTIME_OUTPUT_DIRECTORY)
    # add_custom_command(TARGET SparkLEPlugin_VST3 POST_BUILD
    #     COMMAND ${CMAKE_COMMAND} -E copy_directory
    #         "${JUCE_VST3_COPY_DIR}"
    #         "$ENV{PROGRAMFILES}/Common Files/VST3")
endif()
