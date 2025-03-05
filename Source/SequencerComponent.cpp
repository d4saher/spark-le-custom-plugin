#include "SequencerComponent.h"
#include "PluginProcessor.h"

//==============================================================================
SequencerComponent::SequencerComponent(SparkLEPluginAudioProcessor& p)
    : audioProcessor(p)
{
    // Inicializa algunos pasos para visualización
    sequencerGrid[0][0] = true;
    sequencerGrid[0][4] = true;
    sequencerGrid[0][8] = true;
    sequencerGrid[0][12] = true;
    sequencerGrid[2][2] = true;
    sequencerGrid[2][6] = true;
    sequencerGrid[2][10] = true;
    sequencerGrid[2][14] = true;
    
    // Crea los botones de la cuadrícula
    createGridButtons();
    
    debug();
}

SequencerComponent::~SequencerComponent()
{
    // Los objetos en OwnedArray se eliminan automáticamente
}

void SequencerComponent::debug()
{
    juce::Logger::writeToLog("SequencerComponent::debug()");
    juce::Logger::writeToLog("- numPads: " + juce::String(numPads));
    juce::Logger::writeToLog("- numSteps: " + juce::String(numSteps));
    juce::Logger::writeToLog("- currentStep: " + juce::String(currentStep));
    juce::Logger::writeToLog("- padButtons size: " + juce::String(padButtons.size()));
    juce::Logger::writeToLog("- Step 0,0: " + juce::String(sequencerGrid[0][0] ? "ON" : "OFF"));
}

void SequencerComponent::paint(juce::Graphics& g)
{
    // Fondo del secuenciador
    g.fillAll(juce::Colour(0xff222233));  // Azul oscuro más sutil
    
    // Marco alrededor del secuenciador
    g.setColour(juce::Colours::white);
    g.drawRect(getLocalBounds(), 1);  // Borde más fino
    
    // Dibuja las líneas de la cuadrícula
    g.setColour(juce::Colours::darkgrey);
    
    float cellWidth = getWidth() / (float)numSteps;
    float cellHeight = getHeight() / (float)numPads;
    
    // Líneas verticales
    for (int i = 0; i <= numSteps; ++i)
    {
        float x = i * cellWidth;
        g.drawLine(x, 0, x, getHeight(), 0.5f);
    }
    
    // Líneas horizontales
    for (int i = 0; i <= numPads; ++i)
    {
        float y = i * cellHeight;
        g.drawLine(0, y, getWidth(), y, 0.5f);
    }
    
    // Destaca el paso actual con un color brillante
    g.setColour(juce::Colours::yellow.withAlpha(0.3f));
    g.fillRect(getStepRect(0, currentStep).withHeight(getHeight()));
    
    // Destaca los pasos activos con colores muy claros y brillantes
    for (int row = 0; row < numPads; ++row)
    {
        for (int col = 0; col < numSteps; ++col)
        {
            if (sequencerGrid[row][col])
            {
                auto rect = getStepRect(row, col).reduced(2);
                
                // Colores más brillantes y altamente visibles para cada fila
                juce::Colour rowColor;
                switch (row % 4) {
                    case 0: rowColor = juce::Colours::orange.brighter(0.8f); break;
                    case 1: rowColor = juce::Colours::cyan.brighter(0.8f); break;
                    case 2: rowColor = juce::Colours::lime.brighter(0.8f); break;
                    case 3: rowColor = juce::Colours::magenta.brighter(0.8f); break;
                }
                
                // Contorno grueso para mayor visibilidad
                g.setColour(rowColor.darker());
                g.drawRoundedRectangle(rect.toFloat(), 4.0f, 2.0f);
                
                // Relleno brillante
                g.setColour(rowColor);
                g.fillRoundedRectangle(rect.reduced(2).toFloat(), 3.0f);
            }
        }
    }
}

void SequencerComponent::resized()
{
    // Actualiza las posiciones de los botones
    float cellWidth = getWidth() / (float)numSteps;
    float cellHeight = getHeight() / (float)numPads;
    
    for (int row = 0; row < numPads; ++row)
    {
        for (int col = 0; col < numSteps; ++col)
        {
            int index = row * numSteps + col;
            if (index < padButtons.size() && padButtons[index] != nullptr)
            {
                padButtons[index]->setBounds(getStepRect(row, col));
            }
        }
    }
}

void SequencerComponent::updateDisplay()
{
    // Actualiza el paso actual desde el MidiHandler
    currentStep = audioProcessor.getMidiHandler()->getCurrentStep();
    
    // Repinta el componente
    repaint();
}

// void SequencerComponent::mouseDown(const juce::MouseEvent& e)
// {
//     // Identifica qué paso se ha pulsado
//     float cellWidth = getWidth() / (float)numSteps;
//     float cellHeight = getHeight() / (float)numPads;
    
//     int col = static_cast<int>(e.x / cellWidth);
//     int row = static_cast<int>(e.y / cellHeight);
    
//     // Si está dentro de los límites
//     if (row >= 0 && row < numPads && col >= 0 && col < numSteps)
//     {
//         toggleStep(row, col);
//     }
// }

void SequencerComponent::mouseDrag(const juce::MouseEvent& e)
{
    // Similar a mouseDown, permite arrastrar para activar/desactivar múltiples casillas
    mouseDown(e);
}

void SequencerComponent::createGridButtons()
{
    // Limpia los botones existentes
    padButtons.clear();
    
    // Crea los botones para cada paso
    for (int row = 0; row < numPads; ++row)
    {
        for (int col = 0; col < numSteps; ++col)
        {
            auto* button = new juce::DrawableButton(
                "Step " + juce::String(row) + "-" + juce::String(col),
                juce::DrawableButton::ImageOnButtonBackground);
                
            padButtons.add(button);
            addAndMakeVisible(button);
            
            // Configura la acción al hacer clic
            button->onClick = [this, row, col] { toggleStep(row, col); };
        }
    }
}

juce::Rectangle<int> SequencerComponent::getStepRect(int row, int col)
{
    float cellWidth = getWidth() / (float)numSteps;
    float cellHeight = getHeight() / (float)numPads;
    
    return juce::Rectangle<int>(
        static_cast<int>(col * cellWidth), 
        static_cast<int>(row * cellHeight), 
        static_cast<int>(cellWidth), 
        static_cast<int>(cellHeight)
    );
}

void SequencerComponent::toggleStep(int row, int col)
{
    // Invierte el estado del paso
    sequencerGrid[row][col] = !sequencerGrid[row][col];
    
    // Llama al procesador para actualizar el patrón
    // Envía la información al MidiHandler
    audioProcessor.getMidiHandler()->setStepState(row, col, sequencerGrid[row][col]);

    // Cambia el color del botón
    int index = row * numSteps + col;
    if (index < padButtons.size() && padButtons[index] != nullptr)
    {
        padButtons[index]->setToggleState(sequencerGrid[row][col], juce::dontSendNotification);
    }
    
    // Produce un sonido inmediato cuando se activa un paso
    if (sequencerGrid[row][col]) {
        // Note on para previsualización
        audioProcessor.getMidiHandler()->sendNoteOn(60 + row, 127);
        
        // Note off programado (crea un pequeño efecto de sonido)
        juce::Timer::callAfterDelay(100, [this, row]() {
            audioProcessor.getMidiHandler()->sendNoteOff(60 + row);
        });
    }
    
    repaint();
}