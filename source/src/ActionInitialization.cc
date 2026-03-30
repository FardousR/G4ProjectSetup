#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "EventAction.hh" 
#include "RunAction.hh"
// #include "SteppingAction.hh" // Uncomment when you build this!

ActionInitialization::ActionInitialization(SimulationParameters* simParams)
 : G4VUserActionInitialization(),
   fSimParams(simParams)
{}

ActionInitialization::~ActionInitialization() {}

void ActionInitialization::BuildForMaster() const
{
    // Master thread only needs the RunAction for tracking global stats
    auto runAction = new RunAction(fSimParams);
    SetUserAction(runAction);
}

void ActionInitialization::Build() const
{
    // 1) Primary generator
    auto primaryGenAction = new PrimaryGeneratorAction(fSimParams);
    SetUserAction(primaryGenAction);

    // 2) RunAction
    auto runAction = new RunAction(fSimParams);
    SetUserAction(runAction);

    // 3) EventAction (This hooks up the progress bar!)
    auto eventAction = new EventAction(runAction);
    SetUserAction(eventAction);

    // 3) SteppingAction (pass RunAction to it so it can call RecordEventProcessed!)
    // auto steppingAction = new SteppingAction(runAction, fSimParams);
    // SetUserAction(steppingAction);
}