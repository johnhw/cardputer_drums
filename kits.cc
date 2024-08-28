#include "synth.h"
// Standard kit

constexpr synth_t fmBass = {
    .startFreq = 0,
    .freqDecay = 0,
    .ampAttack = 15,
    .ampDecay = 80,    
    .modDecay = 50,
    .overdrive = 1.0    
  };

constexpr synth_t kickDrum = {
.startFreq = 450,
.freqDecay = 35,
.ampAttack = 0.1,
.ampDecay = 50,    
.overdrive = 2.5,
.lowpassStart = 2000,
};

constexpr synth_t hihatDrum = {
.ampAttack = 0.1,
.ampDecay = 8,
.noise = 1.0,
.overdrive = 0.25
};

constexpr synth_t snareDrum = {
.startFreq=300,
.freqDecay = 100,
.ampAttack = 0.5,
.ampDecay = 18,
.noise = 1.0,
.overdrive = 3.6,
.lowpassStart = 5000,        
.lowpassDecay = 50,
};

constexpr synth_t clickDrum = {
.startFreq=600,
.freqDecay = 0,
.ampAttack = 0.1,    
.ampDecay = 3,
.overdrive = 1.0,  
};

constexpr synth_t tomDrum = {
.startFreq=400,
.freqDecay = 1000,
.ampAttack = 2,    
.ampDecay = 15,
.overdrive = 0.25
};

constexpr synth_t sweepDrum = {
.startFreq=2000,
.freqDecay = 200,
.ampAttack = 3,
.ampDecay = 200,
.overdrive = 0.2
};

constexpr synth_t teekDrum = {
    .startFreq=1000,
    .freqDecay = 200,
    .ampAttack = 0.2,        
    .ampDecay = 3,
    .overdrive = 1.7
};

constexpr synth_t openHat = {                
    .ampAttack = 0,
    .ampDecay = 300,
    .noise=1.0,
    .overdrive = 0.1
};

constexpr synth_t clapDrum = {
    .startFreq=0,
    .freqDecay = 100,
    .ampAttack = 0.1,
    .ampDecay = 19,
    .noise = 1,
    .overdrive = 2.4
};

constexpr synth_t dirtyBass = {
    .startFreq=40,
    .ampAttack=10,
    .ampDecay=60,
    .modRatio = 2.0,
    .modStart = 4.0,
    .modDecay = 120,
    .overdrive = 8.0
};

constexpr synth_t cowbell = {
    .startFreq=800,
    .ampAttack=10,
    .ampDecay=90,
    .modRatio=6.321,
    .modStart = 2.0,        
    .modDecay = 90,
    .overdrive = 0.25,
};

constexpr synth_t closedCowbell = {
    .startFreq=800,
    .ampAttack=10,
    .ampDecay=30,
    .modRatio=6.321,
    .modStart = 0.1,        
    .modDecay = 10,
    .overdrive = 0.2,
};

constexpr synth_t crash = {
    .startFreq=500,
    .ampAttack=2,
    .ampDecay=110,
    .noise=0.1,
    .modRatio=6.321,
    .modStart = 5000.0,        
    .modDecay = 50,
    .overdrive = 0.5,
    .lowpassStart=3000,
    .lowpassDecay=90,        
};

constexpr synth_t empty = {
    .ampAttack = 0,
    .ampDecay = 1,
    
};


// todo: fix clap
constexpr synth_t standardKitDrums[] = {fmBass, kickDrum, hihatDrum, snareDrum, clickDrum, tomDrum, sweepDrum, teekDrum, openHat, clapDrum, dirtyBass, cowbell, closedCowbell, crash, empty, empty, empty};

constexpr kit_t standardKit = {
  .synths = standardKitDrums,
  .nSynths = 17,
  .name = "Standard"
};


// House kit

constexpr synth_t house_fmBass = {
    .startFreq = 0,
    .freqDecay = 0,
    .ampAttack = 5,
    .ampDecay = 70,    
    .modRatio = 3.0,
    .modStart = 10.0,
    .modEnd = 3.0,
    .modDecay = 70,
    .overdrive = 1.0    
  };

constexpr synth_t house_kickDrum = {
.startFreq = 250,
.freqDecay = 35,
.ampAttack = 0.1,
.ampDecay = 30,    
.overdrive = 2.0,
.lowpassStart = 2000,
};

constexpr synth_t house_hihatDrum = {
.ampAttack = 0.1,
.ampDecay = 4,
.noise = 1.0,
.overdrive = 0.25
};

constexpr synth_t house_snareDrum = {
.startFreq=400,
.freqDecay = 100,
.ampAttack = 0.5,
.ampDecay = 10,
.noise = 0.5,
.overdrive = 3.6,
.lowpassStart = 3000,        
.lowpassDecay = 50,
};

constexpr synth_t house_clickDrum = {
.startFreq=4000,
.freqDecay = 0,
.ampAttack = 0.1,    
.ampDecay = 2,
.noise = 1.0,
.overdrive = 0.5,  
};

constexpr synth_t house_tomDrum = {
.startFreq=500,
.freqDecay = 300,
.ampAttack = 2,    
.ampDecay = 20,
.overdrive = 0.6
};

constexpr synth_t house_sweepDrum = {
.startFreq=1000,
.freqDecay = 50,
.ampAttack = 3,
.ampDecay = 200,
.noise = 0.5,
.overdrive = 0.2
};

constexpr synth_t house_teekDrum = {
    .startFreq=1000,
    .freqDecay = 200,
    .ampAttack = 0.2,        
    .ampDecay = 3,
    .overdrive = 1.7
};

constexpr synth_t house_openHat = {                
    .ampAttack = 5,
    .ampDecay = 29,
    .noise=1.0,
    .overdrive = 0.1
};

constexpr synth_t house_clapDrum = {
    .startFreq=0,
    .freqDecay = 100,
    .ampAttack = 0.1,
    .ampDecay = 19,
    .noise = 1,
    .overdrive = 4.4
};

constexpr synth_t house_dirtyBass = {
    .startFreq=400,
    .ampAttack=10,
    .ampDecay=60,
    .modRatio = 2.0,
    .modStart = 0.2,
    .modDecay = 120,
    .overdrive = 1.0
};

constexpr synth_t house_cowbell = {
    .startFreq=1200,
    .ampAttack=10,
    .ampDecay=40,
    .modRatio=6.321,
    .modStart = 2.0,        
    .modDecay = 90,
    .overdrive = 0.25,
};

constexpr synth_t house_closedCowbell = {
    .startFreq=1200,
    .ampAttack=10,
    .ampDecay=30,
    .modRatio=6.321,
    .modStart = 0.1,        
    .modDecay = 10,
    .overdrive = 0.2,
};

constexpr synth_t house_crash = {
    .startFreq=500,
    .ampAttack=2,
    .ampDecay=190,
    .noise=0.1,
    .modRatio=6.321,
    .modStart = 5000.0,        
    .modDecay = 50,
    .overdrive = 0.5,
    .lowpassStart=3000,
    .lowpassDecay=90,        
};

// todo: fix clap
constexpr synth_t houseKitDrums[] = {house_fmBass, house_kickDrum, house_hihatDrum, house_snareDrum, house_clickDrum, house_tomDrum, house_sweepDrum, house_teekDrum, house_openHat, house_clapDrum, house_dirtyBass, house_cowbell, house_closedCowbell, house_crash, empty, empty, empty};

constexpr kit_t houseKit = {
  .synths = houseKitDrums,
  .nSynths = 17,
  .name = "House"
};



// Minimal techno kit

// Gabber kit

// HipHop kit


// Definition

kit_t drumKits[] = {standardKit, houseKit};
int nDrumKits = sizeof(drumKits) / sizeof(kit_t);