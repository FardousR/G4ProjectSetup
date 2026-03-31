#include "RunAction.hh"
#include "PrimaryGeneratorAction.hh" 
#include "Analysis.hh" // Make sure this is included!
#include "G4RunManager.hh"
#include "G4AccumulableManager.hh"
#include "G4Run.hh"
#include "G4AutoLock.hh"
#include <iostream>
#include <iomanip>

// Static initializations for live progress tracking
G4int RunAction::fLastPrintProgress = 0;
G4int RunAction::fGlobalProcessedCounter = 0;
std::chrono::time_point<std::chrono::high_resolution_clock> RunAction::fRunStartChrono;

namespace {
    G4Mutex progressMutex = G4MUTEX_INITIALIZER;
}

RunAction::RunAction(SimulationParameters* simParams)
  : G4UserRunAction(),
    fSimParams(simParams),
    fTimer(new G4Timer()),
    fProcessedEvents("ProcessedEvents", 0)
{
    // 1. MUST Link the parameters to the singleton BEFORE booking!
    auto analysis = Analysis::GetAnalysis();
    analysis->SetSimulationParameters(fSimParams); 
    analysis->BookHistograms();

    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Register(fProcessedEvents);
}

RunAction::~RunAction()
{
    delete fTimer;
}

void RunAction::BeginOfRunAction(const G4Run* run)
{
    G4AccumulableManager::Instance()->Reset();

    // 2. ALL threads (Master AND Workers) must open the analysis file
    auto analysis = Analysis::GetAnalysis();
    analysis->Open();

    if (IsMaster()) {
        PrimaryGeneratorAction::ResetGlobalCounters();
        
        fLastPrintProgress = 0;
        fGlobalProcessedCounter = 0;
        fRunStartChrono = std::chrono::high_resolution_clock::now();
        
        G4cout << "\n=================================================" << G4endl;
        G4cout << ">>> Run " << run->GetRunID() << " starting..." << G4endl;
        G4cout << "=================================================" << G4endl;
    }
}

void RunAction::EndOfRunAction(const G4Run* run)
{
    // ALL threads (Master AND Workers) must save and close their data.
    // Geant4 automatically merges the worker data into the master file here.
    auto analysis = Analysis::GetAnalysis();
    analysis->Save();
    analysis->Close();

    G4AccumulableManager::Instance()->Merge();

    if (IsMaster()) {
        fRunEndChrono = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(fRunEndChrono - fRunStartChrono).count();
        
        // Calculate the time breakdowns
        auto days    = duration / (24 * 3600);
        auto hours   = (duration % (24 * 3600)) / 3600;
        auto minutes = (duration % 3600) / 60;
        auto seconds = duration % 60;
        
        G4cout << "\n=============================================" << G4endl;
        G4cout << " Run " << run->GetRunID() << " completed!" << G4endl;
        G4cout << " Total Events Simulated : " << fProcessedEvents.GetValue() << G4endl;
        G4cout << "=============================================" << G4endl;
        G4cout << std::left << std::setw(15) << " Days"    << ": " << days << G4endl;
        G4cout << std::left << std::setw(15) << " Hours"   << ": " << hours << G4endl;
        G4cout << std::left << std::setw(15) << " Minutes" << ": " << minutes << G4endl;
        G4cout << std::left << std::setw(15) << " Seconds" << ": " << seconds << G4endl;
        G4cout << "=============================================\n" << G4endl;
    }
}

void RunAction::RecordEventProcessed()
{
    fProcessedEvents += 1;
    
    // Lock mutex before accessing the global counter for console printing
    G4AutoLock lock(&progressMutex);
    fGlobalProcessedCounter++;
    UpdateProgress();
}

G4int RunAction::GetTotalProcessedEvents()
{
    return fProcessedEvents.GetValue();
}

void RunAction::UpdateProgress()
{
    G4int total = fSimParams->GetNumParticles();
    if (total <= 0) return;

    G4int percent = (fGlobalProcessedCounter * 100) / total;
    
    // Print a progress update every 10%
    if (percent >= fLastPrintProgress + 10) { 
        fLastPrintProgress = (percent / 10) * 10; 
        PrintProgress();
    }
}

void RunAction::PrintProgress()
{
    G4int total = fSimParams->GetNumParticles();
    if (total <= 0 || fGlobalProcessedCounter == 0) return;

    // Calculate elapsed time in seconds
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - fRunStartChrono;
    double elapsedSec = elapsed.count();

    // Avoid division by zero in the very first fraction of a second
    if (elapsedSec <= 0.0) elapsedSec = 0.001; 

    // Calculate Rate and ETA
    double eventsPerSecond = fGlobalProcessedCounter / elapsedSec;
    double remainingSec = (total - fGlobalProcessedCounter) / eventsPerSecond;

    // Format ETA into Hours, Minutes, Seconds
    int remHours = static_cast<int>(remainingSec) / 3600;
    int remMins = (static_cast<int>(remainingSec) % 3600) / 60;
    int remSecs = static_cast<int>(remainingSec) % 60;

    // Calculate absolute projected End Time
    auto projectedEndTimeChrono = std::chrono::system_clock::now() + std::chrono::seconds(static_cast<long long>(remainingSec));
    std::time_t projectedEndTimeT = std::chrono::system_clock::to_time_t(projectedEndTimeChrono);
    
    // Format the absolute time nicely (e.g., "14:35:22")
    std::tm* endTm = std::localtime(&projectedEndTimeT);
    char timeBuffer[64];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", endTm);

    // Print the beautifully formatted output
    G4cout << " [Progress] " << std::setw(3) << fLastPrintProgress << "% "
           << "(" << fGlobalProcessedCounter << " / " << total << ") "
           << "| ETA: " << remHours << "h " << remMins << "m " << remSecs << "s "
           << "| Est. Finish: " << timeBuffer << G4endl;
}