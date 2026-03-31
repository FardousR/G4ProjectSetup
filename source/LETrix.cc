#include "SimulationParameters.hh"
#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"
#include "G4PhysListFactory.hh"
#include "G4StepLimiterPhysics.hh"
#include "PhysicsList.hh"
#include "ProcessLET.hh"

#include "G4SystemOfUnits.hh"
#include "G4RunManagerFactory.hh"
#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
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
    // 1. Initialize Simulation Parameters using a raw pointer
    SimulationParameters* SP = new SimulationParameters(argc, argv);
    
    // Safely attempt to read the plan file and assign the beam
    try {
        SP->Initialize();
    } catch (const std::exception& e) {
        cerr << "\nFatal Initialization Error: " << e.what() << "\nExiting..." << endl;
        delete SP;
        return EXIT_FAILURE;
    }

    // 2. Set Random Engine
    CLHEP::RanluxEngine defaultEngine(1234567, 4);
    G4Random::setTheEngine(&defaultEngine);
    G4Random::setTheSeed(static_cast<G4int>(time(nullptr)));

    // 3. Construct the Run Manager
    #ifdef G4MULTITHREADED
        G4MTRunManager* runManager = new G4MTRunManager();
        runManager->SetNumberOfThreads(SP->GetThreads());
    #else
        G4RunManager* runManager = new G4RunManager();
    #endif
    
    // 4. Set Detector Construction
    runManager->SetUserInitialization(new DetectorConstruction(SP));

    // Decide on physics list
    G4VModularPhysicsList* physicsList = nullptr;
    G4PhysListFactory physListFactory;
    G4String physListName = SP->GetPhyList();

    if (physListName == "PHYSICS_LIST_CUSTOM") {
        G4cout << "Using custom PhysicsList..." << G4endl;
        physicsList = new PhysicsList();
    }
    else {
        physicsList = physListFactory.GetReferencePhysList(physListName);
        if (!physicsList) {
            cerr << "Error: Physics list \"" << physListName << "\" not found. Exiting." << endl;
            delete runManager;
            delete SP;
            return EXIT_FAILURE;
        }
    }

    // Register Step Limiter Physics
    physicsList->RegisterPhysics(new G4StepLimiterPhysics());
    runManager->SetUserInitialization(physicsList);

    // Set User Action Initialization
    runManager->SetUserInitialization(new ActionInitialization(SP));

    // 5. Setup UI Manager
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

    // Declare GUI variables outside the if-statement so we can delete them safely
    G4UIExecutive* ui = nullptr;
    G4VisExecutive* visManager = nullptr;

    // 6. Execute GUI or Batch Mode
    if (SP->GetUseGUI()) {
        ui = new G4UIExecutive(argc, argv);
        visManager = new G4VisExecutive();
        visManager->Initialize();
        
        uiManager->ApplyCommand("/control/execute init_vis.mac");
        ui->SessionStart();
        
        // Clean up UI when the user closes the window
        delete visManager;
        delete ui;
    } else {
        // Batch mode
        uiManager->ApplyCommand("/run/initialize");
        uiManager->ApplyCommand("/run/beamOn " + to_string(SP->GetNumParticles()));
    }

    // -------------------------------------------------------------------------
    // 7. CRITICAL TERMINATION SEQUENCE
    // -------------------------------------------------------------------------
    
    // Step A: Delete RunManager FIRST. 
    // This tells Geant4 to merge worker threads and safely close Output.root.
    delete runManager; 

    // Step B: Trigger Post-Processing.
    // Now that the file is fully closed, ROOT can safely open it and do the math.
    // (Note: Changed 'simParams' to 'SP' to match your variable declaration)
    ProcessLET::Process(SP->GetOutfile()); 

    // Step C: Delete parameters and exit
    delete SP;
    
    return EXIT_SUCCESS;
}