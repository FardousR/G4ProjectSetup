#include "SteppingAction.hh"
#include "G4Step.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include "Analysis.hh"
#include "DataType.hh" // Assuming your struct is defined here

SteppingAction::SteppingAction(SimulationParameters* simParams)
    : G4UserSteppingAction(), fSimParams(simParams) {}

SteppingAction::~SteppingAction() {}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    G4double edep = step->GetTotalEnergyDeposit();
    G4double stepLen = step->GetStepLength();

    // Skip steps with no energy OR no length (prevents division by zero for LET)
    if (edep <= 0.0 || stepLen <= 0.0) return;

    // Random point along the track for accurate spatial binning
    G4ThreeVector prePoint  = step->GetPreStepPoint()->GetPosition();
    G4ThreeVector postPoint = step->GetPostStepPoint()->GetPosition();
    G4ThreeVector position  = prePoint + G4UniformRand() * (postPoint - prePoint);

    // Get the density of the material at this specific step
    G4double density = step->GetPreStepPoint()->GetMaterial()->GetDensity();

    // Create a safe, thread-local instance of YOUR struct
    ScoredQuantity sq;
    
    // Fill the fields using your exact variable names
    sq.EnergyDeposit      = edep;
    sq.EnergyDepositPoint = position;
    sq.StepLength         = stepLen;
    sq.MaterialDensity    = density;  // Changed from previous 'Density'
    
    // (Optional) Fill other useful fields from your struct if you need them later
    // sq.TrackID          = step->GetTrack()->GetTrackID();
    // sq.ParticlePDGcode  = step->GetTrack()->GetDefinition()->GetPDGEncoding();
    // sq.CurrentStep      = step;

    // Send the data to your thread-local Analysis singleton
    auto analysis = Analysis::GetAnalysis();
    analysis->FillScoringData(sq);
}