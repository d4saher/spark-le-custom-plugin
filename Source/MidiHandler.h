#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

//==============================================================================
class MidiHandler
{
public:
    MidiHandler();
    ~MidiHandler();
    
    // Preparación para reproducción y liberación de recursos
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void releaseResources();
    
    // Procesa mensajes MIDI entrantes y genera salida
    void processMidi(juce::MidiBuffer& midiMessages);
    
    // Funciones para el secuenciador
    void startSequencer();
    void stopSequencer();
    void setTempo(double bpm);
    bool isSequencerPlaying() const { return isPlaying; }
    
    // Funciones para interactuar con el Spark LE
    void sendNoteOn(int noteNumber, int velocity, int channel = 1);
    void sendNoteOff(int noteNumber, int channel = 1);
    void sendControlChange(int controllerNumber, int value, int channel = 1);
    
    // Funciones para controlar los LEDs del Spark LE
    void setLED(int padIndex, bool isOn);
    void setPadColor(int padIndex, juce::Colour color);
    
    // Funciones para gestionar los pasos del secuenciador
    void setStepState(int padIndex, int step, bool isActive);
    bool getStepState(int padIndex, int step) const;
    int getCurrentStep() const;
    
    // Estado del click
    bool clickEnabled = true;
    
private:
    // MIDI
    std::unique_ptr<juce::MidiOutput> midiOutput;
    juce::String sparkLEDeviceName;
    
    // Secuenciador
    bool isPlaying;
    double bpm;
    double sampleRate;
    int samplesPerBeat;
    int samplePosition;
    juce::int64 lastTimeInSamples;  // Para modo standalone
    juce::AudioPlayHead::CurrentPositionInfo positionInfo;  // Info de posición desde el host
    
    // Matriz para almacenar el estado de los pasos
    static constexpr int maxPads = 8;
    static constexpr int maxSteps = 16;
    bool sequencerSteps[maxPads][maxSteps];
    int currentStep;
    
    // Métodos auxiliares
    void advanceSequencer(int numSamples);
    void findSparkLEDevice();
    void triggerCurrentStep();
    
    // Constantes MIDI específicas del Spark LE
    struct SparkLEMidi
    {
        static constexpr int padNoteOffset = 60;  // Nota MIDI para el primer pad
        static constexpr int ledControllerOffset = 20;  // CC para el primer LED
    };
};