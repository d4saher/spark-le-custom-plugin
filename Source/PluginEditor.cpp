#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SparkLEPluginAudioProcessorEditor::SparkLEPluginAudioProcessorEditor(SparkLEPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    juce::Logger::writeToLog("SparkLEPlugin: Creando el editor del plugin");
    
    // Configura un tamaño mayor para acomodar el secuenciador
    setSize(800, 600);
    
    // Primero creamos el secuenciador antes que otros componentes
    try {
        juce::Logger::writeToLog("SparkLEPlugin: Inicializando componente del secuenciador");
        sequencerComponent = std::make_unique<SequencerComponent>(audioProcessor);
        
        // Forzamos un tamaño específico para el secuenciador para depuración
        sequencerComponent->setBounds(20, 100, 760, 400); 
        
        // Lo añadimos a la jerarquía de componentes
        addAndMakeVisible(*sequencerComponent);
        juce::Logger::writeToLog("SparkLEPlugin: Secuenciador añadido correctamente");
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog("SparkLEPlugin ERROR: Error al crear secuenciador: " + juce::String(e.what()));
    }
    
    // Añade el botón de carga de samples por encima
    addAndMakeVisible(loadSampleButton);
    loadSampleButton.setBounds(20, 60, 120, 30);
    loadSampleButton.setButtonText("Cargar Sample");
    loadSampleButton.onClick = [this] { 
        juce::Logger::writeToLog("SparkLEPlugin: Botón de carga pulsado");
        loadSampleButtonClicked();
    };
    
    // Añade el botón de reproducción
    addAndMakeVisible(playButton);
    playButton.setBounds(150, 60, 40, 30);
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    playButton.setButtonText("Play");
    playButton.onClick = [this] {
        if (audioProcessor.getMidiHandler()->isSequencerPlaying()) {
            audioProcessor.getMidiHandler()->stopSequencer();
            playButton.setButtonText("Play");
            playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
            juce::Logger::writeToLog("SparkLEPlugin: Secuenciador detenido");
        } else {
            audioProcessor.getMidiHandler()->startSequencer();
            playButton.setButtonText("Stop");
            playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
            juce::Logger::writeToLog("SparkLEPlugin: Secuenciador iniciado");
        }
    };

    // Añade el botón de click
    addAndMakeVisible(clickButton);
    clickButton.setBounds(220, 60, 80, 30);
    clickButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgreen);
    clickButton.onClick = [this] {
        // Actualiza el estado del click en el MidiHandler
        audioProcessor.getMidiHandler()->clickEnabled = !audioProcessor.getMidiHandler()->clickEnabled;
        clickButton.setButtonText(audioProcessor.getMidiHandler()->clickEnabled ? "Click ON" : "Click OFF");
        clickButton.setColour(juce::TextButton::buttonColourId, 
                        audioProcessor.getMidiHandler()->clickEnabled ? juce::Colours::darkgreen : juce::Colours::darkgrey);
        juce::Logger::writeToLog("SparkLEPlugin: Click " + juce::String(audioProcessor.getMidiHandler()->clickEnabled ? "activado" : "desactivado"));
    };
    
    // Añade el control de tempo
    addAndMakeVisible(tempoLabel);
    tempoLabel.setBounds(310, 60, 60, 30);
    tempoLabel.setJustificationType(juce::Justification::right);

    addAndMakeVisible(tempoSlider);
    tempoSlider.setRange(60.0, 200.0, 1.0);
    tempoSlider.setValue(120.0);
    tempoSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    tempoSlider.setBounds(370, 60, 150, 30);
    tempoSlider.onValueChange = [this] {
        double newTempo = tempoSlider.getValue();
        audioProcessor.getMidiHandler()->setTempo(newTempo);
        juce::Logger::writeToLog("SparkLEPlugin: Tempo establecido a " + juce::String(newTempo) + " BPM");
    };
    
    // Inicia el timer para actualizar la UI
    startTimer(16); // Aproximadamente 60 fps
    
    juce::Logger::writeToLog("SparkLEPlugin: Editor creado correctamente");
}

SparkLEPluginAudioProcessorEditor::~SparkLEPluginAudioProcessorEditor()
{
    juce::Logger::writeToLog("SparkLEPlugin: Destruyendo el editor del plugin");
}

//==============================================================================
void SparkLEPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Dibuja un fondo simple
    g.fillAll(juce::Colours::darkgrey);
    
    // Dibuja el título
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawText("Spark LE Plugin", getLocalBounds(), juce::Justification::centred);
}

void SparkLEPluginAudioProcessorEditor::resized()
{
    // Nota: No hacemos nada aquí porque hemos establecido las posiciones explícitamente
    // en el constructor para fines de depuración
    juce::Logger::writeToLog("SparkLEPlugin: resized() llamado - no ajustamos posiciones aquí");
}

void SparkLEPluginAudioProcessorEditor::timerCallback()
{
    // Actualiza los componentes que necesiten redibujarse
    if (sequencerComponent != nullptr)
        sequencerComponent->updateDisplay();
    
    repaint();
}

void SparkLEPluginAudioProcessorEditor::loadSampleButtonClicked()
{
    juce::Logger::writeToLog("SparkLEPlugin: Función de carga de samples llamada");
    
    // Crea un FileChooser básico para probar
    juce::FileChooser chooser("Selecciona un sample...",
                             juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                             "*.wav;*.aif;*.aiff");
    
    // La API moderna de JUCE usa launchAsync
    chooser.launchAsync(juce::FileBrowserComponent::openMode, 
                       [this](const juce::FileChooser& fc)
    {
        if (fc.getResults().size() > 0)
        {
            juce::File file = fc.getResults().getReference(0);
            juce::Logger::writeToLog("SparkLEPlugin: Archivo seleccionado: " + file.getFullPathName());
        }
    });
}

void SparkLEPluginAudioProcessorEditor::setupPatternSelector()
{
    // Versión simplificada sin implementación
    juce::Logger::writeToLog("SparkLEPlugin: setupPatternSelector - no implementado en versión simple");
}

void SparkLEPluginAudioProcessorEditor::setupTempoControl()
{
    // Versión simplificada sin implementación
    juce::Logger::writeToLog("SparkLEPlugin: setupTempoControl - no implementado en versión simple");
}