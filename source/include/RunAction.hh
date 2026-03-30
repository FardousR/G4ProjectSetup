#ifndef RUN_ACTION_HH
#define RUN_ACTION_HH

#include "G4UserRunAction.hh"
#include "G4Accumulable.hh"
#include "G4Timer.hh"
#include "SimulationParameters.hh"
#include <ctime>
#include <chrono>

class RunAction : public G4UserRunAction
{
public:
    explicit RunAction(SimulationParameters* simParams);
    virtual ~RunAction();

    virtual void BeginOfRunAction(const G4Run* run) override;
    virtual void EndOfRunAction(const G4Run* run) override;

    void RecordEventProcessed();
    G4int GetTotalProcessedEvents();

private:
    void UpdateProgress();
    void PrintProgress();

private:
    SimulationParameters* fSimParams;
    G4Timer* fTimer;
    G4Accumulable<G4int> fProcessedEvents; 
    
    // Static globals for thread-safe progress printing
    static G4int fLastPrintProgress;
    static G4int fGlobalProcessedCounter;

    // Made static so worker threads can calculate ETA based on master's start time
    static std::chrono::time_point<std::chrono::high_resolution_clock> fRunStartChrono;
    
    std::chrono::time_point<std::chrono::high_resolution_clock> fRunEndChrono;
};
#endif // RUN_ACTION_HH