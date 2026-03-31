#ifndef STEPPING_ACTION_HH
#define STEPPING_ACTION_HH

#include "G4UserSteppingAction.hh"
#include "SimulationParameters.hh"

class G4Step;

class SteppingAction : public G4UserSteppingAction
{
public:
    // Notice: RunAction is removed here, as we no longer need it!
    explicit SteppingAction(SimulationParameters* simParams);
    ~SteppingAction() override;

    void UserSteppingAction(const G4Step* step) override;

private:
    SimulationParameters* fSimParams;
};

#endif // STEPPING_ACTION_HH