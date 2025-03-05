#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "SequencerComponent.h"

// Forward declarations
class SparkLEPluginAudioProcessor;

//==============================================================================
class SparkLEPluginAudioProcessorEditor : public juce::AudioProcessorEditor,
                                         public juce::Timer
{
public:
    SparkLEPluginAudioProcessorEditor (SparkLEPluginAudioProcessor&);
    ~SparkLEPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // Timer callback para actualizar la UI periódicamente
    void timerCallback() override;

private:
    // Referencia al procesador de audio
    SparkLEPluginAudioProcessor& audioProcessor;

    // Componentes de la UI
    juce::TextButton loadSampleButton { "Load Sample" };
    juce::TextButton playButton { "Play" };
    juce::TextButton clickButton { "Click ON" };
    juce::Slider tempoSlider;
    juce::Label tempoLabel { {}, "Tempo:" };
    std::unique_ptr<SequencerComponent> sequencerComponent;
    
    // Métodos para responder a los botones
    void loadSampleButtonClicked();
    void setupPatternSelector();
    void setupTempoControl();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SparkLEPluginAudioProcessorEditor)
};