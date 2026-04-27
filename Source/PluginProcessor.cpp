#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       apvts (*this, nullptr, "Parameters", Parameters::createLayout())
{
    synth.addVoice (new ReeseVoice (*this));
    
    synth.addSound (new ReeseSound());

    synth.setNoteStealingEnabled (true);
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() = default;


float AudioPluginAudioProcessor::getFloatParam (const juce::StringRef& paramID) const
{
    if (auto* value = apvts.getRawParameterValue (paramID))
        return value->load();

    jassertfalse;
    return 0.0f;
}

int AudioPluginAudioProcessor::getIntParam (const juce::StringRef& paramID) const
{
    if (auto* param = dynamic_cast<juce::AudioParameterInt*>(apvts.getParameter (paramID)))
        return param->get();

    jassertfalse;
    return 0;
}

bool AudioPluginAudioProcessor::getBoolParam (const juce::StringRef& paramID) const
{
    if (auto* param = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter (paramID)))
        return param->get();

    jassertfalse;
    return false;
}

int AudioPluginAudioProcessor::getChoiceParam (const juce::StringRef& paramID) const
{
    if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter (paramID)))
        return param->getIndex();

    jassertfalse;
    return 0;
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<ReeseVoice*> (synth.getVoice (i)))
            voice->prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
}

void AudioPluginAudioProcessor::releaseResources()
{
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();
    
    // Update transport info from host
    if (auto* playHead = getPlayHead())
    {
        if (auto positionInfo = playHead->getPosition())
        {
            if (positionInfo->getBpm().hasValue())
                currentBPM.store(*positionInfo->getBpm());
        }
    }

    // Track note overlaps to enable legato mode
    // Set legato mode BEFORE processing each note based on whether notes were already held
    for (const auto metadata : midiMessages)
    {
        const auto& msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            // Enable legato only if there were already notes held before this one
            if (auto* voice = dynamic_cast<ReeseVoice*> (synth.getVoice (0)))
                voice->setLegatoMode (numNotesHeld > 0);
            
            ++numNotesHeld;
        }
        else if (msg.isNoteOff())
        {
            --numNotesHeld;
        }
    }

    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    const auto outputGain = getFloatParam ("output");
    const auto driveTrim = juce::jmap (getFloatParam ("drive"), 1.0f, 0.75f);
    buffer.applyGain (outputGain * driveTrim);
    
    // Track peak level and clipping
    float currentPeak = 0.0f;
    bool clipping = false;
    constexpr float clipThreshold = 1.0f;
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const auto magnitude = buffer.getMagnitude (channel, 0, buffer.getNumSamples());
        currentPeak = juce::jmax (currentPeak, magnitude);
        
        if (magnitude >= clipThreshold)
            clipping = true;
    }
    
    // Update atomic values for UI
    peakLevel.store (currentPeak);
    isCurrentlyClipping.store (clipping);
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
