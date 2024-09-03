// stores and retrieves DrumMachine data
#include "serialize.h"

#define SERIALIZE_ID 0xBEA101

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

bool writeDrumMachine(DrumMachine &dm, File &ser)
{
    char *buf;    
    addToken(ser, SERIALIZE_ID);
    addToken(ser, dm.bpm);
    addToken(ser, dm.swing);
    addToken(ser, dm.pattern);
    addToken(ser, dm.kit);
    addToken(ser, dm.volume);

    addToken(ser, strlen(dm.patternSequence));

    for(int i=0;i<strlen(dm.patternSequence);i++)
    {
        addToken(ser, dm.patternSequence[i]);
    }

    addToken(ser, nChans);

    for(int i=0;i<nChans;i++)
    {
        addToken(ser, dm.channels[i].volume);
        addToken(ser, dm.channels[i].mute);
        addToken(ser, dm.channels[i].solo);
        addToken(ser, dm.channels[i].filterCutoff);
        addToken(ser, dm.channels[i]._enabled);
    }

    addToken(ser, nSteps * nChans * maxPatterns);
    for(int i=0;i<nSteps * nChans * maxPatterns;i++)
    {
        addToken(ser, dm.allPatterns[i].type);
        addToken(ser, dm.allPatterns[i].velocity);
        addToken(ser, dm.allPatterns[i].kickDelay);
    }

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


    int len = getToken(ser);
    char patternSequence[len+1];
    for(int i=0;i<len;i++)
    {
        patternSequence[i] = getToken(ser);
    }
    patternSequence[len] = '\0';
    strcpy(dm.patternSequence, patternSequence);
    
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

    int patLen = getToken(ser);
    if(patLen != nSteps * nChans * maxPatterns)
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

    int end = getToken(ser);
    if(end != 0)
    {
        Serial.println("Invalid end token");
        return false;
    }

    return true;

}
