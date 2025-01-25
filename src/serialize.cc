// stores and retrieves DrumMachine data
#include "serialize.h"

#define SERIALIZE_ID 0xBEA106

// write a single integer to the string, followed by a comma
void addToken(File ser, int32_t value)
{
    ser.print(value);
    ser.print(",");
}

// read a single signed integer from the string, followed by a comma
int32_t getToken(File ser)
{
    int32_t value = 0;
    bool isNegative = false;

    if (ser.peek() == '-')
    {
        isNegative = true;
        ser.read(); // consume the negative sign
    }

    while (ser.peek() != ',' && ser.peek() != -1)
    {
        value = value * 10 + (ser.read() - '0');
    }

    if (ser.peek() == ',')
    {
        ser.read(); // consume the comma
    }

    return isNegative ? -value : value;
}

bool writeSampleAdjustment(sample_adjustment_t *adj, File &ser)
{
    addToken(ser, 0xf00d);
    addToken(ser, adj->detune);
    addToken(ser, adj->volume);
    addToken(ser, adj->cutoff);
    addToken(ser, adj->trimStart);
    addToken(ser, adj->trimEnd);
    addToken(ser, adj->loopStart);
    addToken(ser, adj->loopEnd);
    addToken(ser, adj->loopMode);
    addToken(ser, adj->attackTime);
    addToken(ser, adj->decayTime);
    addToken(ser, adj->sustainLevel);
    addToken(ser, adj->releaseTime);
    addToken(ser, adj->samplePtr);
    return true;
}

bool readSampleAdjustment(sample_adjustment_t *adj, File &ser)
{
    int32_t id = getToken(ser);
    if (id != 0xf00d)
    {
        Serial.println("Invalid sample adjustment ID");
        return false;
    }
    adj->detune = getToken(ser);
    adj->volume = getToken(ser);
    adj->cutoff = getToken(ser);
    adj->trimStart = getToken(ser);
    adj->trimEnd = getToken(ser);
    adj->loopStart = getToken(ser);
    adj->loopEnd = getToken(ser);
    adj->loopMode = getToken(ser);
    adj->attackTime = getToken(ser);
    adj->decayTime = getToken(ser);
    adj->sustainLevel = getToken(ser);
    adj->releaseTime = getToken(ser);
    adj->samplePtr = getToken(ser);
    return true;
}

bool readSampleAdjustments(DrumMachine &dm, File &ser)
{
    for (int i = 0; i < 26; i++)
    {
        readSampleAdjustment(&dm.drumSamples[i].adjustments, ser);
    }
    return true;
}

bool writeSampleAdjustments(DrumMachine &dm, File &ser)
{
    for (int i = 0; i < 26; i++)
    {
        writeSampleAdjustment(&dm.drumSamples[i].adjustments, ser);
    }
    return true;
}

/* read a single step of a pattern from the file */
bool readStep(step_t *step, File &ser)
{
    step->type = getToken(ser);
    step->velocity = getToken(ser);
    step->kickDelay = getToken(ser);
    step->fx = getToken(ser);
    step->detune = getToken(ser);
    step->portaTime = getToken(ser);
    return true;
}

/* write a single step of a pattern to the file */
bool writeStep(step_t *step, File &ser)
{
    addToken(ser, step->type);
    addToken(ser, step->velocity);
    addToken(ser, step->kickDelay);
    addToken(ser, step->fx);
    addToken(ser, step->detune);
    addToken(ser, step->portaTime);
    return true;
}

/* read the pattern data from the file */
bool readPatterns(DrumMachine &dm, File &ser)
{
    if (getToken(ser) != 0xf002)
    {
        Serial.println("Invalid pattern ID");
        return false;
    }
    int32_t nPatterns = getToken(ser);
    if (nPatterns != nSteps * nChans * maxPatterns)
    {
        Serial.println("Invalid number of patterns");
        return false;
    }

    for (int i = 0; i < nSteps * nChans * maxPatterns; i++)
    {
        readStep(&dm.allPatterns[i], ser);        
    }
    return true;
}

/* write out the pattern data to the file */
bool writePatterns(DrumMachine &dm, File &ser)
{
    addToken(ser, 0xf002);
    addToken(ser, nSteps * nChans * maxPatterns);
    for (int i = 0; i < nSteps * nChans * maxPatterns; i++)
    {
       writeStep(&dm.allPatterns[i], ser);
    }
    return true;
}

/* read the channel data from the file */
bool readChannels(DrumMachine &dm, File &ser)
{
    if (getToken(ser) != 0xf002)
    {
        Serial.println("Invalid channel ID");
        return false;
    }
    int32_t nChansCheck = getToken(ser);
    if (nChansCheck != nChans)
    {
        Serial.println("Invalid number of channels");
        return false;
    }

    for (int i = 0; i < nChans; i++)
    {
        dm.channels[i].volume = getToken(ser);
        dm.channels[i].mute = getToken(ser);
        dm.channels[i].solo = getToken(ser);
        dm.channels[i].filterCutoff = getToken(ser);
        dm.channels[i]._enabled = getToken(ser);
    }

    return true;
}

/* write out the channel data to the file */
bool writeChannels(DrumMachine &dm, File &ser)
{
    addToken(ser, 0xf002);
    addToken(ser, nChans);
    for (int i = 0; i < nChans; i++)
    {
        addToken(ser, dm.channels[i].volume);
        addToken(ser, dm.channels[i].mute);
        addToken(ser, dm.channels[i].solo);
        addToken(ser, dm.channels[i].filterCutoff);
        addToken(ser, dm.channels[i]._enabled);
    }
    return true;
}

bool writeConfiguration(DrumMachine &dm, File &ser)
{
    addToken(ser, 0xd00a);
    addToken(ser, dm.bpm);
    addToken(ser, dm.swing);
    addToken(ser, dm.pattern);
    addToken(ser, dm.kit);
    addToken(ser, dm.volume);
    addToken(ser, dm.patternMode);
    return true;
}

bool writePatternSequence(DrumMachine &dm, File &ser)
{
    addToken(ser, 0xbeef);
    addToken(ser, strlen(dm.patternSequence));
    for (int i = 0; i < strlen(dm.patternSequence); i++)
    {
        addToken(ser, dm.patternSequence[i]);
    }
    return true;
}

bool writeDrumMachine(DrumMachine &dm, File &ser)
{
    char *buf;
    addToken(ser, SERIALIZE_ID);
    writeConfiguration(dm, ser);
    writePatternSequence(dm, ser);       
    writeChannels(dm, ser);
    writePatterns(dm, ser);
    writeSampleAdjustments(dm, ser);
    addToken(ser, 0);
    return true;
}

/* Read the global configuration from the file */
bool readConfiguration(DrumMachine &dm, File &ser)
{
    int32_t id = getToken(ser);
    if (id != 0xd00a)
    {
        Serial.println("Invalid configuration ID");
        return false;
    }
    dm.bpm = getToken(ser);
    dm.swing = getToken(ser);
    dm.pattern = getToken(ser);
    dm.kit = getToken(ser);
    dm.volume = getToken(ser);
    dm.patternMode = getToken(ser);
    return true;
}

bool readPatternSequence(DrumMachine &dm, File &ser)
{
    int32_t id = getToken(ser);
    if (id != 0xbeef)
    {
        Serial.println("Invalid pattern sequence ID");
        return false;
    }
    int len = getToken(ser);
    char patternSequence[128];
    for (int i = 0; i < len; i++)
    {
        patternSequence[i] = getToken(ser);
    }
    patternSequence[len] = '\0';
    strcpy(dm.patternSequence, patternSequence);
    return true;
}

bool readDrumMachine(DrumMachine &dm, File &ser)
{

    int pos = 0;
    int32_t id;
    id = getToken(ser);

    // verify magic number
    if (id != SERIALIZE_ID)
    {
        Serial.println("Invalid serialize ID");
        return false;
    }    
    readConfiguration(dm, ser);
    readPatternSequence(dm, ser);    
    readChannels(dm, ser);    
    readPatterns(dm, ser);    
    readSampleAdjustments(dm, ser);    

    int end = getToken(ser);
    if (end != 0)
    {
        Serial.println("Invalid end token");
        return false;
    }

    return true;
}

void setKit(DrumMachine &dm, int kit);

bool serialiseKit(DrumMachine &dm, File &ser)
{
   
    addToken(ser, SERIALIZE_ID);
    addToken(ser, 0xad00d);
    addToken(ser, dm.kit);
    writeSampleAdjustments(dm, ser);
    addToken(ser, 0);
    return true;
}

bool deserialiseKit(DrumMachine &dm, File &ser)
{
    int32_t id = getToken(ser);
    if (id != SERIALIZE_ID)
    {
        Serial.println("Invalid serialize ID");
        return false;
    }

    int32_t check = getToken(ser);
    if (check != 0xad00d)
    {
        Serial.println("Invalid sample adjustment ID");
        return false;
    }

    int32_t kit = getToken(ser);
    readSampleAdjustments(dm, ser);

    int32_t end = getToken(ser);
    if (end != 0)
    {
        Serial.println("Invalid end token");
        return false;
    }

    return true;
}

