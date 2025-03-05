#include "MidiHandler.h"

//==============================================================================
MidiHandler::MidiHandler()
    : midiOutput(nullptr),
      isPlaying(false),
      bpm(120.0),
      sampleRate(44100.0),
      samplesPerBeat(0),
      samplePosition(0),
      currentStep(0),
      lastTimeInSamples(0)
{
    juce::Logger::writeToLog("SparkLEPlugin: Inicializando MidiHandler");
    
    // Inicializa la matriz de pasos a false
    for (int pad = 0; pad < maxPads; ++pad)
    {
        for (int step = 0; step < maxSteps; ++step)
        {
            sequencerSteps[pad][step] = false;
        }
    }
    
    // Busca el dispositivo Spark LE
    try {
        findSparkLEDevice();
        juce::Logger::writeToLog("SparkLEPlugin: MidiHandler inicializado correctamente");
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog("SparkLEPlugin ERROR en MidiHandler: " + juce::String(e.what()));
    }
}

MidiHandler::~MidiHandler()
{
    // Ya no necesitamos limpiar explícitamente el midiOutput
    // unique_ptr se encargará de la liberación automáticamente
}

void MidiHandler::prepareToPlay(double newSampleRate, int /*samplesPerBlock*/)
{
    sampleRate = newSampleRate;
    
    // Calcula muestras por beat (para un compás 4/4 a la velocidad actual)
    samplesPerBeat = static_cast<int>((60.0 / bpm) * sampleRate);
}

void MidiHandler::releaseResources()
{
    // Detiene el secuenciador
    stopSequencer();
}

void MidiHandler::processMidi(juce::MidiBuffer& midiMessages)
{
    // Procesa mensajes MIDI entrantes
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        
        // Procesa los mensajes según sea necesario
        // Por ejemplo, detectar mensajes específicos del Spark LE
        if (message.isNoteOn())
        {
            // Procesa nota activada
            int noteNumber = message.getNoteNumber();
            int velocity = message.getVelocity();
            
            // Mapea a pad si es necesario
            // Por ejemplo, si la nota está en el rango de los pads del Spark
            if (noteNumber >= SparkLEMidi::padNoteOffset && 
                noteNumber < SparkLEMidi::padNoteOffset + maxPads)
            {
                int padIndex = noteNumber - SparkLEMidi::padNoteOffset;
                // Enciende el LED correspondiente
                setLED(padIndex, true);
            }
        }
        else if (message.isNoteOff())
        {
            // Procesa nota desactivada
            int noteNumber = message.getNoteNumber();
            
            if (noteNumber >= SparkLEMidi::padNoteOffset && 
                noteNumber < SparkLEMidi::padNoteOffset + maxPads)
            {
                int padIndex = noteNumber - SparkLEMidi::padNoteOffset;
                // Apaga el LED correspondiente
                setLED(padIndex, false);
            }
        }
        else if (message.isController())
        {
            // Procesa mensajes de control change
            int controllerNumber = message.getControllerNumber();
            int value = message.getControllerValue();
            
            // Aquí podrías responder a los controles específicos del Spark LE
            // ...
        }
    }
    
    // Si el secuenciador está activo, avanza y genera eventos MIDI
    if (isPlaying)
    {
        // En modo standalone, necesitamos manejar nuestro propio timing
        if (lastTimeInSamples == 0)
        {
            // Primera vez, inicializa el tiempo
            lastTimeInSamples = juce::Time::getHighResolutionTicks();
        }
        else
        {
            // Calcula el tiempo transcurrido en samples
            juce::int64 currentTime = juce::Time::getHighResolutionTicks();
            double elapsedTimeMs = juce::Time::highResolutionTicksToSeconds(currentTime - lastTimeInSamples) * 1000.0;
            lastTimeInSamples = currentTime;
            
            // Convierte el tiempo transcurrido a samples (basado en sample rate)
            int elapsedSamples = static_cast<int>(elapsedTimeMs * sampleRate / 1000.0);
            
            // Avanza el secuenciador con el tiempo transcurrido
            advanceSequencer(elapsedSamples);
            
            // Dispara los eventos MIDI para el paso actual
            triggerCurrentStep();
        }
        
        // Genera un click si está habilitado y estamos en un beat principal
        if (clickEnabled && currentStep % 4 == 0)
        {
            // Añadir una nota MIDI para el click
            int clickNote = 37; // Generalmente un sonido de caja o palmas
            auto message = juce::MidiMessage::noteOn(10, clickNote, (juce::uint8)100);
            midiMessages.addEvent(message, 0);
            
            // Programar el note off
            auto noteOff = juce::MidiMessage::noteOff(10, clickNote);
            midiMessages.addEvent(noteOff, 10); // Después de 10 samples
        }
    }
}

void MidiHandler::startSequencer()
{
    if (!isPlaying)
    {
        isPlaying = true;
        currentStep = 0;
        samplePosition = 0;
    }
}

void MidiHandler::stopSequencer()
{
    if (isPlaying)
    {
        isPlaying = false;
        
        // Envía Note Off para todos los pads activos
        for (int pad = 0; pad < maxPads; ++pad)
        {
            sendNoteOff(SparkLEMidi::padNoteOffset + pad);
            setLED(pad, false);
        }
    }
}

void MidiHandler::setTempo(double newBpm)
{
    bpm = newBpm;
    
    // Recalcula muestras por beat
    samplesPerBeat = static_cast<int>((60.0 / bpm) * sampleRate);
}

void MidiHandler::sendNoteOn(int noteNumber, int velocity, int channel)
{
    if (midiOutput)
    {
        midiOutput->sendMessageNow(juce::MidiMessage::noteOn(channel, noteNumber, (juce::uint8) velocity));
    }
}

void MidiHandler::sendNoteOff(int noteNumber, int channel)
{
    if (midiOutput)
    {
        midiOutput->sendMessageNow(juce::MidiMessage::noteOff(channel, noteNumber));
    }
}

void MidiHandler::sendControlChange(int controllerNumber, int value, int channel)
{
    if (midiOutput)
    {
        midiOutput->sendMessageNow(juce::MidiMessage::controllerEvent(channel, controllerNumber, value));
    }
}

void MidiHandler::setLED(int padIndex, bool isOn)
{
    // Esta función envía un mensaje MIDI para controlar los LEDs del Spark LE
    if (padIndex >= 0 && padIndex < maxPads)
    {
        int controllerNumber = SparkLEMidi::ledControllerOffset + padIndex;
        int value = isOn ? 127 : 0;  // 127 para encendido, 0 para apagado
        
        sendControlChange(controllerNumber, value);
    }
}

void MidiHandler::setPadColor(int padIndex, juce::Colour color)
{
    // Nota: Esta es una función de ejemplo, ya que el Spark LE original 
    // probablemente no tiene control de color RGB. Modifícala según el protocolo real.
    
    // En algunos controladores modernos, el color se podría enviar como una serie de
    // mensajes SysEx o CC específicos.
    
    // Por ejemplo, podríamos enviar un System Exclusive para establecer el color:
    if (midiOutput && padIndex >= 0 && padIndex < maxPads)
    {
        // Convertir el color a componentes RGB (0-127)
        uint8_t r = static_cast<uint8_t>(color.getRed() >> 1);    // 0-127
        uint8_t g = static_cast<uint8_t>(color.getGreen() >> 1);  // 0-127
        uint8_t b = static_cast<uint8_t>(color.getBlue() >> 1);   // 0-127
        
        // Ejemplo de mensaje SysEx (deberías adaptar esto a tu controlador)
        // F0 (SysEx), ID del fabricante, ID del dispositivo, comando de color, pad, r, g, b, F7 (End SysEx)
        const uint8_t sysExData[] = { 0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42, static_cast<uint8_t>(padIndex), r, g, b, 0xF7 };
        
        juce::MidiMessage sysExMessage(sysExData, sizeof(sysExData));
        midiOutput->sendMessageNow(sysExMessage);
    }
}

void MidiHandler::setStepState(int padIndex, int step, bool isActive)
{
    if (padIndex >= 0 && padIndex < maxPads && step >= 0 && step < maxSteps)
    {
        sequencerSteps[padIndex][step] = isActive;
    }
}

bool MidiHandler::getStepState(int padIndex, int step) const
{
    if (padIndex >= 0 && padIndex < maxPads && step >= 0 && step < maxSteps)
    {
        return sequencerSteps[padIndex][step];
    }
    
    return false;
}

int MidiHandler::getCurrentStep() const
{
    return currentStep;
}

void MidiHandler::advanceSequencer(int numSamples)
{
    // Avanza la posición del sample
    samplePosition += numSamples;
    
    // Si llegamos a un nuevo paso (16mo de nota)
    int samplesPerStep = samplesPerBeat / 4;  // En un compás 4/4, 4 pasos por beat
    
    if (samplePosition >= samplesPerStep)
    {
        // Cuántos pasos completos han pasado
        int stepsToAdvance = samplePosition / samplesPerStep;
        
        // Avanza el paso actual
        currentStep = (currentStep + stepsToAdvance) % maxSteps;
        
        // Dispara los eventos MIDI para el paso actual
        triggerCurrentStep();
        
        // Ajusta la posición del sample
        samplePosition %= samplesPerStep;
    }
}

void MidiHandler::findSparkLEDevice()
{
    // Intenta encontrar el dispositivo Spark LE entre los dispositivos MIDI disponibles
    auto midiOutputs = juce::MidiOutput::getAvailableDevices();
    
    juce::Logger::writeToLog("SparkLEPlugin: Buscando dispositivos MIDI disponibles");
    juce::Logger::writeToLog("SparkLEPlugin: Número de dispositivos MIDI: " + juce::String(midiOutputs.size()));
    
    for (auto& device : midiOutputs)
    {
        juce::Logger::writeToLog("SparkLEPlugin: Dispositivo MIDI encontrado: " + device.name);
        
        // Busca un dispositivo que contenga "Spark" en su nombre
        if (device.name.containsIgnoreCase("Spark"))
        {
            juce::Logger::writeToLog("SparkLEPlugin: Dispositivo Spark LE encontrado: " + device.name);
            sparkLEDeviceName = device.name;
            
            try {
                // En versiones recientes de JUCE, openDevice devuelve un unique_ptr
                midiOutput = juce::MidiOutput::openDevice(device.identifier);
                
                if (midiOutput) {
                    juce::Logger::writeToLog("SparkLEPlugin: Conexión con Spark LE establecida correctamente");
                    return;
                }
            }
            catch (const std::exception& e) {
                juce::Logger::writeToLog("SparkLEPlugin ERROR: No se pudo abrir el dispositivo MIDI: " + juce::String(e.what()));
            }
        }
    }
    
    // Si no encontramos el Spark LE, muestra una alerta o registra un mensaje
    juce::Logger::writeToLog("SparkLEPlugin: No se pudo encontrar el dispositivo Arturia Spark LE");
}

void MidiHandler::triggerCurrentStep()
{
    // Envía eventos MIDI para los pads activados en el paso actual
    for (int pad = 0; pad < maxPads; ++pad)
    {
        if (sequencerSteps[pad][currentStep])
        {
            // Envía Note On para este pad
            int noteNumber = SparkLEMidi::padNoteOffset + pad;
            sendNoteOn(noteNumber, 127);  // Velocidad máxima
            
            // Enciende el LED 
            setLED(pad, true);
            
            // Programa un Note Off automático para la siguiente actualización
            // En una implementación real, podrías calcular cuándo enviar el Note Off
            // basado en la duración de la nota deseada
        }
        else
        {
            // Si el pad no está activo en este paso, asegúrate de que el LED esté apagado
            setLED(pad, false);
        }
    }
}