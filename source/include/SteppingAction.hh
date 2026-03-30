#ifndef STEPPING_ACTION_HH
#define STEPPING_ACTION_HH

#include "G4UserSteppingAction.hh"
#include "RunAction.hh"
#include "SimulationParameters.hh"

#include <unordered_map>
#include <cstdint>

class G4VPhysicalVolume;
class G4Step;

class SteppingAction : public G4UserSteppingAction
{
public:
    SteppingAction(RunAction* runAction, SimulationParameters* simParams);
    ~SteppingAction() override;

    void UserSteppingAction(const G4Step* step) override;

private:
    // Per-track accumulation while inside the detector
    struct DetAccum {
        G4double dep   = 0.0;  // total deposited energy inside detector
        G4double len   = 0.0;  // total path length inside detector
        G4double iniKE = 0.0;  // kinetic energy at detector entrance
    };

    using Key64 = std::uint64_t;

    // Geometry / bookkeeping helpers
    bool  IsInDetector(const G4VPhysicalVolume* vol) const;
    static Key64 MakeKey(int eventID, int trackID);

    void     StartDetectorAccum(Key64 key, G4double iniKE);
    void     UpdateDetectorAccum(Key64 key, G4double stepDeposit, G4double stepLen);
    bool     HasAccum(Key64 key) const;
    DetAccum TakeAccum(Key64 key);

    // Scoring entry points for different data types
    void ScoreStepData();
    void ScoreDetectorData();
    void ScoreDosimetryData();
    void ScoreReactionData();

private:
    RunAction*            fRunAction   = nullptr;
    SimulationParameters* fSimParams   = nullptr;
    const G4Step*         fStep        = nullptr;  // current step (read-only)

    std::unordered_map<Key64, DetAccum> fDetAccums;
};

#endif // STEPPING_ACTION_HH
