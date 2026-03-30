#include "EventAction.hh"

EventAction::EventAction(RunAction* runAction)
 : G4UserEventAction(),
   fRunAction(runAction)
{}

EventAction::~EventAction() {}

void EventAction::EndOfEventAction(const G4Event*)
{
    // Tell the RunAction that one more particle has finished!
    if (fRunAction) {
        fRunAction->RecordEventProcessed();
    }
}