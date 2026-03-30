#include "SimulationParameters.hh"
#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"
#include "G4PhysListFactory.hh"
#include "G4StepLimiterPhysics.hh"
#include "PhysicsList.hh"

#include "G4SystemOfUnits.hh"
#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "QBBC.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4UserSpecialCuts.hh"

#include <iostream>
#include <memory>
#include <algorithm> // For std::max

using namespace std;

int main(int argc, char** argv)
{
    // Initialize Simulation Parameters
    auto SP = make_unique<SimulationParameters>(argc, argv);
    
    // Safely attempt to read the plan file and assign the beam
    try {
        SP->Initialize();
    } catch (const std::exception& e) {
        cerr << "\nFatal Initialization Error: " << e.what() << "\nExiting..." << endl;
        return EXIT_FAILURE;
    }

    // Set Random Engine
    CLHEP::RanluxEngine defaultEngine(1234567, 4);
    G4Random::setTheEngine(&defaultEngine);
    G4Random::setTheSeed(static_cast<G4int>(time(nullptr)));

    // Construct the Run Manager
    #ifdef G4MULTITHREADED
        auto runManager = make_unique<G4MTRunManager>();
        runManager->SetNumberOfThreads(SP->GetThreads());
    #else
        auto runManager = make_unique<G4RunManager>();
    #endif
    
    // Set Detector Construction
    runManager->SetUserInitialization(new DetectorConstruction(SP.get()));

    // Decide on physics list
    G4VModularPhysicsList* physicsList = nullptr;
    G4PhysListFactory physListFactory;
    G4String physListName = SP->GetPhyList();

    // Example: if user passes "--phylist PHYSICS_LIST_CUSTOM" (or similar),
    // we use our custom PhysicsList class.
    if (physListName == "PHYSICS_LIST_CUSTOM") {
        G4cout << "Using custom PhysicsList..." << G4endl;
        physicsList = new PhysicsList();
    }
    else {
        // Otherwise, try the built-in reference list
        physicsList = physListFactory.GetReferencePhysList(physListName);

        if (!physicsList) {
            cerr << "Error: Physics list \"" << physListName << "\" not found. Exiting." << endl;
            return EXIT_FAILURE;
        }
    }

    // Register Step Limiter Physics if desired
    physicsList->RegisterPhysics(new G4StepLimiterPhysics());
    runManager->SetUserInitialization(physicsList);

    // Set User Action Initialization
    runManager->SetUserInitialization(new ActionInitialization(SP.get()));

    // Get UI Manager
    G4UImanager* uiManager = G4UImanager::GetUIpointer();
    const vector<string> uiCommands = {
        "/control/verbose 0",
        "/run/verbose 0",
        "/event/verbose 0",
        "/tracking/verbose 0",
        "/geometry/verbose 0",
        "/vis/verbose 0",
        "/geometry/navigator/check_overlap 0",
        "/process/verbose 0",
        "/process/had/verbose 0",
        "/process/em/verbose 0"
    };
    for (const auto& cmd : uiCommands) {
        uiManager->ApplyCommand(cmd);
    }

    // If user wants GUI mode
    if (SP->GetUseGUI()) {
        auto ui = make_unique<G4UIExecutive>(argc, argv);
        auto visManager = make_unique<G4VisExecutive>();
        visManager->Initialize();
        // Open the ToolsSG driver (TSG) directly from code
        // uiManager->ApplyCommand("/vis/open TSG");
        uiManager->ApplyCommand("/control/execute init_vis.mac");
        ui->SessionStart();
    } else {
        // Batch mode
        uiManager->ApplyCommand("/run/initialize");
        
        // Ensure progress interval is at least 1 to avoid Geant4 UI warnings
        int progressInterval = std::max(1, SP->GetNumParticles() / 20);
        // uiManager->ApplyCommand("/run/printProgress " + to_string(progressInterval));
        
        uiManager->ApplyCommand("/run/beamOn " + to_string(SP->GetNumParticles()));
    }
    return EXIT_SUCCESS;
}