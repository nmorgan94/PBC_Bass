#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_gui_basics/juce_gui_basics.h>

class PresetManager
{
public:
    PresetManager(juce::AudioProcessorValueTreeState& apvts)
        : valueTreeState(apvts)
    {
        // Get preset directory
        presetDirectory = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("SampleRealm")
            .getChildFile("Reese")
            .getChildFile("Presets");
        
        // Create directory if it doesn't exist
        if (!presetDirectory.exists())
            presetDirectory.createDirectory();
        
        loadPresetList();
    }
    
    bool savePreset(const juce::String& presetName)
    {
        if (presetName.isEmpty())
            return false;
        
        // Create JSON object
        juce::var presetData(new juce::DynamicObject());
        auto* obj = presetData.getDynamicObject();
        
        // Save all parameters
        for (auto* param : valueTreeState.processor.getParameters())
        {
            if (auto* paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
            {
                obj->setProperty(paramWithID->paramID, param->getValue());
            }
        }
        
        // Write to file
        auto presetFile = presetDirectory.getChildFile(presetName + ".json");
        juce::FileOutputStream stream(presetFile);
        
        if (stream.openedOk())
        {
            stream.writeText(juce::JSON::toString(presetData, true), false, false, nullptr);
            loadPresetList(); // Refresh list
            return true;
        }
        
        return false;
    }
    
    bool loadPreset(const juce::String& presetName)
    {
        auto presetFile = presetDirectory.getChildFile(presetName + ".json");
        
        if (!presetFile.existsAsFile())
            return false;
        
        // Read JSON file
        juce::var presetData = juce::JSON::parse(presetFile);
        
        if (!presetData.isObject())
            return false;
        
        auto* obj = presetData.getDynamicObject();
        
        // Load all parameters
        for (auto* param : valueTreeState.processor.getParameters())
        {
            if (auto* paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
            {
                if (obj->hasProperty(paramWithID->paramID))
                {
                    float value = obj->getProperty(paramWithID->paramID);
                    param->setValueNotifyingHost(value);
                }
            }
        }
        
        return true;
    }
    
    bool deletePreset(const juce::String& presetName)
    {
        auto presetFile = presetDirectory.getChildFile(presetName + ".json");
        
        if (presetFile.existsAsFile())
        {
            bool deleted = presetFile.deleteFile();
            if (deleted)
                loadPresetList(); // Refresh list
            return deleted;
        }
        
        return false;
    }
    
    juce::StringArray getPresetList() const
    {
        return presetList;
    }
    
    void loadPresetList()
    {
        presetList.clear();
        
        auto presetFiles = presetDirectory.findChildFiles(juce::File::findFiles, false, "*.json");
        
        for (const auto& file : presetFiles)
        {
            presetList.add(file.getFileNameWithoutExtension());
        }
        
        presetList.sort(true); // Sort alphabetically
    }
    
    void showSaveDialog(juce::Component*, std::function<void(bool, juce::String)> callback)
    {
        auto* window = new juce::AlertWindow("Save Preset", "Enter preset name:", juce::AlertWindow::NoIcon);
        window->addTextEditor("presetName", "", "Preset Name:");
        window->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
        window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
        
        window->enterModalState(true, juce::ModalCallbackFunction::create([this, window, callback](int result)
        {
            bool success = false;
            juce::String presetName;
            
            if (result == 1)
            {
                presetName = window->getTextEditorContents("presetName");
                
                if (presetName.isNotEmpty())
                {
                    success = savePreset(presetName);
                    
                    if (!success)
                    {
                        juce::NativeMessageBox::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                                                    "Error",
                                                                    "Failed to save preset!");
                    }
                }
            }
            
            callback(success, presetName);
        }), true);
    }
    
    void showDeleteDialog(const juce::String& presetName, juce::Component* parent, std::function<void(bool)> callback)
    {
        juce::NativeMessageBox::showOkCancelBox(juce::AlertWindow::QuestionIcon,
                                                "Delete Preset",
                                                "Are you sure you want to delete '" + presetName + "'?",
                                                parent,
                                                juce::ModalCallbackFunction::create([this, presetName, callback](int result)
        {
            bool success = false;
            
            if (result == 1) // OK was clicked
            {
                success = deletePreset(presetName);
                
                if (!success)
                {
                    juce::NativeMessageBox::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                                                "Error",
                                                                "Failed to delete preset!");
                }
            }
            
            callback(success);
        }));
    }
    
private:
    juce::AudioProcessorValueTreeState& valueTreeState;
    juce::File presetDirectory;
    juce::StringArray presetList;
};
