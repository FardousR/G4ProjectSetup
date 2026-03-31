//============================= Analysis.cc =============================
#include "Analysis.hh"

// Geant4 includes
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "G4AutoDelete.hh"

// C/C++ STL
#include <iostream>

// Thread-local pointer to Analysis instance
G4ThreadLocal Analysis* gThreadLocalAnalysis = nullptr;

//------------------------------------------------------------------------------
// Singleton Access
//------------------------------------------------------------------------------
Analysis* Analysis::GetAnalysis()
{
    if (!gThreadLocalAnalysis) {
        gThreadLocalAnalysis = new Analysis();
        G4AutoDelete::Register(gThreadLocalAnalysis);
    }
    return gThreadLocalAnalysis;
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
Analysis::Analysis()
  : fSimParams(nullptr),
    fPlanDoseFactor(1.0),
    fHistIdEne(-1),   // <-- Added initialization
    fHistIdFlu(-1),   // <-- Added initialization
    fHistIdDose(-1),
    fHistIdLETd(-1),
    fHistIdLETf(-1)
{}

//------------------------------------------------------------------------------
// File I/O Management
//------------------------------------------------------------------------------
void Analysis::Open()
{
    auto analysisManager = G4AnalysisManager::Instance();
    
    if (!fSimParams) {
        G4Exception("Analysis::Open()", "NoSimParams", FatalException,
                    "fSimParams is null. Please call SetSimulationParameters first.");
        return;
    }
    
    // Open the file using the name provided in the CLI/Macro
    analysisManager->OpenFile(fSimParams->GetOutfile());
}

void Analysis::Save()
{
    auto analysisManager = G4AnalysisManager::Instance();
    if (fSimParams) {
        analysisManager->Write();
    }
}

void Analysis::Close()
{
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->CloseFile();
}

//------------------------------------------------------------------------------
// Histogram Booking
//------------------------------------------------------------------------------
void Analysis::BookHistograms()
{
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->SetVerboseLevel(1);

    if (fSimParams) {
        fPlanDoseFactor = fSimParams->GetPlanDoseFactor();
    }

    // Book histograms and store their integer IDs for fast access later.
    // Ensure the limits (0 to 200) match your physical phantom dimensions (e.g., mm)
    fHistIdEne  = analysisManager->CreateH1("Energy", "Energy Deposit", 400, 0, 400);      // <-- Added
    fHistIdFlu  = analysisManager->CreateH1("Fluence", "Particle Fluence", 400, 0, 400);   // <-- Added
    fHistIdDose = analysisManager->CreateH1("Dose", "Depth Dose Profile", 400, 0, 400);
    fHistIdLETd = analysisManager->CreateH1("LET_dose", "Dose-Averaged LET", 400, 0, 400);
    fHistIdLETf = analysisManager->CreateH1("LET_fluence", "Fluence-Averaged LET", 400, 0, 400);
}

//------------------------------------------------------------------------------
// Data Recording (Called per step/event)
//------------------------------------------------------------------------------
void Analysis::FillScoringData(const ScoredQuantity& SDL)
{
    G4double x = SDL.EnergyDepositPoint.x();
    G4double y = SDL.EnergyDepositPoint.y();
    G4double z = SDL.EnergyDepositPoint.z();

    // ----------------------------------------------------------------
    // 1. Spatial Cut (Define your scoring field size here)
    // ----------------------------------------------------------------
    const G4double limitX = 50.0 * mm; // Scores within +/- 10mm
    const G4double limitY = 50.0 * mm; // Scores within +/- 10mm
    
    if (std::abs(x) > limitX || std::abs(y) > limitY) {
        return; // Exits immediately if the particle is outside the box
    }

    // ----------------------------------------------------------------
    // 2. Extract Step Data using YOUR struct's variable names
    // ----------------------------------------------------------------
    G4double edep    = SDL.EnergyDeposit;
    G4double stepLen = SDL.StepLength;
    G4double density = SDL.MaterialDensity; 

    // ----------------------------------------------------------------
    // 3. Dose & Fluence Calculation
    // ----------------------------------------------------------------
    // Assuming your Z-axis histogram has a 1mm bin width (e.g., 200 bins for 200mm)
    const G4double binWidthZ = 1.0 * mm; 
    
    // Calculate Mass = Volume * Density
    G4double volume = (2.0 * limitX) * (2.0 * limitY) * binWidthZ;
    G4double mass   = density * volume;
    
    // Dose = Energy / Mass. (Dividing by 'gray' converts Geant4 units to Gy)
    G4double dose = (edep / mass) / gray; 

    // ----------------------------------------------------------------
    // 4. LET Calculation (in standard keV/um)
    // ----------------------------------------------------------------
    G4double let      = (edep / stepLen) / (keV / um);
    G4double fluence  = stepLen / volume;

    auto analysisManager = G4AnalysisManager::Instance();

    // ----------------------------------------------------------------
    // 5. Fill Histograms
    // ----------------------------------------------------------------
    // Score Raw Energy
    analysisManager->FillH1(fHistIdEne, z, edep);

    // Score Fluence (Fixed the typo here to use fHistIdFlu)
    analysisManager->FillH1(fHistIdFlu, z, fluence);

    // Score Absolute Dose (Gy)
    analysisManager->FillH1(fHistIdDose, z, dose);

    // Score Dose-Averaged LET Numerator
    analysisManager->FillH1(fHistIdLETd, z, let * edep);

    // Score Fluence-Averaged LET Numerator
    analysisManager->FillH1(fHistIdLETf, z, let * fluence);
}