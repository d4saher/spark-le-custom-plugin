#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Forward declaration para evitar dependencias circulares
class SparkLEPluginAudioProcessor;

//==============================================================================
/**
 * Componente del secuenciador que muestra una cuadrícula para programar patrones rítmicos.
 */
class SequencerComponent : public juce::Component,
                           private juce::Timer
{
public:
    // Constructor y destructor
    SequencerComponent(SparkLEPluginAudioProcessor& p);
    ~SequencerComponent() override;

    // Métodos de juce::Component sobrescritos
    void paint(juce::Graphics&) override;
    void resized() override;
    // void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    
    // Timer callback para auto-apagado de notas
    void timerCallback() override {}
    
    // Actualiza la visualización (llamado desde el timer del editor)
    void updateDisplay();
    
    // Método de depuración
    void debug();
    
private:
    // Referencia al procesador de audio
    SparkLEPluginAudioProcessor& audioProcessor;
    
    // Número de pasos y pads
    static constexpr int numSteps = 16;
    static constexpr int numPads = 8;
    
    // Estado del secuenciador
    bool sequencerGrid[numPads][numSteps] = {{false}};
    int currentStep = 0;
    
    // Matriz de botones para la cuadrícula
    juce::OwnedArray<juce::DrawableButton> padButtons;
    
    // Métodos para gestionar la cuadrícula
    void createGridButtons();
    juce::Rectangle<int> getStepRect(int row, int col);
    void toggleStep(int row, int col);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SequencerComponent)
};