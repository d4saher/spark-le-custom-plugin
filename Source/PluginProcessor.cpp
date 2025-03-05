#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "MidiHandler.h"

//==============================================================================
SparkLEPluginAudioProcessor::SparkLEPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", {})
{
    // Inicialización del plugin
    juce::Logger::writeToLog("SparkLEPlugin: Inicializando el procesador de audio");
    
    // Intenta abrir un archivo de registro para DEBUG
    juce::File logFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("sparkle_plugin_log.txt");
    juce::FileLogger* fileLogger = new juce::FileLogger(logFile, "SparkLE Plugin Log");
    juce::Logger::setCurrentLogger(fileLogger);
    
    juce::Logger::writeToLog("SparkLEPlugin: Procesador creado correctamente");
}

SparkLEPluginAudioProcessor::~SparkLEPluginAudioProcessor()
{
    juce::Logger::writeToLog("SparkLEPlugin: Destruyendo el procesador de audio");
    juce::Logger::setCurrentLogger(nullptr);
}

//==============================================================================
const juce::String SparkLEPluginAudioProcessor::getName() const
{
    return "Spark LE Custom Plugin";
}

bool SparkLEPluginAudioProcessor::acceptsMidi() const
{
    return true;
}

bool SparkLEPluginAudioProcessor::producesMidi() const
{
    return true;
}

bool SparkLEPluginAudioProcessor::isMidiEffect() const
{
    return false;
}

double SparkLEPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SparkLEPluginAudioProcessor::getNumPrograms()
{
    return 1;
}

int SparkLEPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SparkLEPluginAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String SparkLEPluginAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return "Default";
}

void SparkLEPluginAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void SparkLEPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Inicializa cualquier recurso que necesites
    midiHandler.prepareToPlay(sampleRate, samplesPerBlock);
}

void SparkLEPluginAudioProcessor::releaseResources()
{
    // Libera recursos cuando dejas de reproducir
    midiHandler.releaseResources();
}

bool SparkLEPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Acepta solo configuraciones de salida estéreo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void SparkLEPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Borra canales de salida no utilizados
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Procesa el MIDI
    midiHandler.processMidi(midiMessages);
    
    // Añade un pequeño sonido para verificar que el secuenciador está funcionando
    // Solo si el click está habilitado
    if (midiHandler.isSequencerPlaying() && midiHandler.clickEnabled)
    {
        int currentStep = midiHandler.getCurrentStep();
        if (currentStep % 4 == 0)  // En cada beat principal
        {
            // Generar un pequeño click para indicar el tempo (solo durante desarrollo)
            float clickVolume = 0.1f; // Volumen bajo
            for (int i = 0; i < std::min(100, buffer.getNumSamples()); i++) {
                float sample = clickVolume * std::sin(i * 0.1f);
                if (i > 50) sample *= (1.0f - ((i - 50) / 50.0f)); // fade out
                
                for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
                    buffer.addSample(channel, i, sample);
                }
            }
        }
    }
}

//==============================================================================
bool SparkLEPluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SparkLEPluginAudioProcessor::createEditor()
{
    return new SparkLEPluginAudioProcessorEditor(*this);
}

//==============================================================================
void SparkLEPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Guarda el estado del plugin para recuperarlo después
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SparkLEPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restaura el estado previo
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SparkLEPluginAudioProcessor();
}
