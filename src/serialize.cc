// stores and retrieves DrumMachine data
#include "serialize.h"

#define SERIALIZE_ID 0xBEA102

// write a single integer to the string, followed by a comma
void addToken(File ser, int32_t value)
{
    ser.print(value);
    ser.print(",");
}


// read a single integer from the string, followed by a comma
int32_t getToken(File ser)
{
    int32_t value = 0;    
    while(ser.peek() != ',' && ser.peek() != -1)
    {
        value = value * 10 + (ser.read() - '0');
    }
    if(ser.peek() == ',')
    {
        ser.read();
    }
    return value;
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
    addToken(ser, adj->attackTime);
    addToken(ser, adj->decayTime);
    addToken(ser, adj->sustainLevel);
    addToken(ser, adj->releaseTime);
    return true;
}

bool readSampleAdjustment(sample_adjustment_t *adj, File &ser)
{
    int32_t id = getToken(ser);
    if(id != 0xf00d)
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
    adj->attackTime = getToken(ser);
    adj->decayTime = getToken(ser);
    adj->sustainLevel = getToken(ser);
    adj->releaseTime = getToken(ser);
    return true;
}

bool readSampleAdjustments(DrumMachine &dm, File &ser)
{
    for(int i=0;i<26;i++)
    {
        readSampleAdjustment(&dm.drumSamples[i].adjustments, ser);
    }
    return true;
}

bool writeSampleAdjustments(DrumMachine &dm, File &ser)
{
    for(int i=0;i<26;i++)
    {
        writeSampleAdjustment(&dm.drumSamples[i].adjustments, ser);
    }
    return true;
}

/* read the pattern data from the file */
bool readPatterns(DrumMachine &dm, File &ser)
{
    if(getToken(ser)!=0xf002)
    {
        Serial.println("Invalid pattern ID");
        return false;
    }
    int32_t nPatterns = getToken(ser);
    if(nPatterns != nSteps * nChans * maxPatterns)
    {
        Serial.println("Invalid number of patterns");
        return false;
    }

    for(int i=0;i<nSteps * nChans * maxPatterns;i++)
    {
        dm.allPatterns[i].type = getToken(ser);
        dm.allPatterns[i].velocity = getToken(ser);
        dm.allPatterns[i].kickDelay = getToken(ser);
    }

    return true;
}

/* write out the pattern data to the file */
bool writePatterns(DrumMachine &dm, File &ser)
{
    addToken(ser, 0xf002);
    addToken(ser, nSteps * nChans * maxPatterns);
    for(int i=0;i<nSteps * nChans * maxPatterns;i++)
    {
        addToken(ser, dm.allPatterns[i].type);
        addToken(ser, dm.allPatterns[i].velocity);
        addToken(ser, dm.allPatterns[i].kickDelay);
    }
    return true;
}

/* read the channel data from the file */
bool readChannels(DrumMachine &dm, File &ser)
{
    if(getToken(ser)!=0xf002)
    {
        Serial.println("Invalid channel ID");
        return false;
    }
    int32_t nChansCheck = getToken(ser);
    if(nChansCheck != nChans)
    {
        Serial.println("Invalid number of channels");
        return false;
    }

    for(int i=0;i<nChans;i++)
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
    for(int i=0;i<nChans;i++)
    {
        addToken(ser, dm.channels[i].volume);
        addToken(ser, dm.channels[i].mute);
        addToken(ser, dm.channels[i].solo);
        addToken(ser, dm.channels[i].filterCutoff);
        addToken(ser, dm.channels[i]._enabled);
    }
    return true;
}


bool writeDrumMachine(DrumMachine &dm, File &ser)
{
    char *buf;    
    addToken(ser, SERIALIZE_ID);
    addToken(ser, dm.bpm);
    addToken(ser, dm.swing);
    addToken(ser, dm.pattern);
    addToken(ser, dm.kit);
    addToken(ser, dm.volume);
    addToken(ser, dm.patternMode);

    addToken(ser, strlen(dm.patternSequence));

    for(int i=0;i<strlen(dm.patternSequence);i++)
    {
        addToken(ser, dm.patternSequence[i]);
    }

    writeChannels(dm, ser);
    writePatterns(dm, ser);
    writeSampleAdjustments(dm, ser);


    addToken(ser, 0);    
    
    return true;

}


bool readDrumMachine(DrumMachine &dm, File &ser)
{

    int pos = 0;
    int32_t id;
    id = getToken(ser);
        
    if(id != SERIALIZE_ID)
    {
        Serial.println("Invalid serialize ID");
        return false;
    }
    dm.bpm = getToken(ser);
    dm.swing = getToken(ser);
    dm.pattern = getToken(ser);
    dm.kit = getToken(ser);
    dm.volume = getToken(ser);
    dm.patternMode = getToken(ser);


    int len = getToken(ser);
    char patternSequence[len+1];
    for(int i=0;i<len;i++)
    {
        patternSequence[i] = getToken(ser);
    }
    patternSequence[len] = '\0';
    strcpy(dm.patternSequence, patternSequence);
    
    readChannels(dm, ser);
    readPatterns(dm, ser);        
    readSampleAdjustments(dm, ser);

    int end = getToken(ser);
    if(end != 0)
    {
        Serial.println("Invalid end token");
        return false;
    }

    return true;

}
