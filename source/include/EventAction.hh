#ifndef EVENT_ACTION_HH
#define EVENT_ACTION_HH

#include "G4UserEventAction.hh"
#include "RunAction.hh"

class EventAction : public G4UserEventAction
{
public:
    explicit EventAction(RunAction* runAction);
    virtual ~EventAction();

    // This is called automatically by Geant4 at the end of every event/particle
    virtual void EndOfEventAction(const G4Event* event) override;

private:
    RunAction* fRunAction;
};

#endif