#ifndef SIMULATION_PARAMETERS_HH
#define SIMULATION_PARAMETERS_HH

#include "G4SystemOfUnits.hh"
#include "G4String.hh"
#include "DataType.hh" // Assumes SpotParameter is defined here
#include <string>
#include <vector>
#include <cfloat>
#include <stdexcept>

/**
 * @enum BeamMode
 * @brief Strictly sequential distribution logic.
 */
enum BeamMode {
    kModeUndefined = 0,
    kSequentialMuRand = 1,
    kSequentialMuRatio = 2,
    kSequentialMinPar = 3
};

/**
 * @struct EnergyLayer
 * @brief Holds summary data for each distinct energy layer parsed from the plan.
 */
struct EnergyLayer {
    G4double energy;
    G4int num_spots;
};

class SimulationParameters {
public:
    SimulationParameters(int argc, char** argv);
    ~SimulationParameters();

    // Safely loads files and prints info (called in main.cc after instantiation)
    void Initialize();

    // ----------------------------------------------------------------------
    // Accessors & Mutators
    // ----------------------------------------------------------------------
    G4int GetNumParticles() const { return fNumParticles; }
    void  SetNumParticles(G4int v) { fNumParticles = v; }

    G4int GetMinParPerSpot() const { return fMinParPerSpot; }
    void  SetMinParPerSpot(G4int v) { fMinParPerSpot = v; }

    G4int GetThreads() const { return fThreads; }
    void  SetThreads(G4int v) { fThreads = v; }

    G4String GetOutfile() const { return fOutfile; }
    void     SetOutfile(const G4String& v) { fOutfile = v; }

    G4String GetPlanfile() const { return fPlanfile; }
    void     SetPlanfile(const G4String& v) { fPlanfile = v; }

    G4String GetTargetMat() const { return fTargetMat; }
    void     SetTargetMat(const G4String& v) { fTargetMat = v; }

    G4bool GetUseGUI() const { return fUseGUI; }
    void   SetUseGUI(G4bool v) { fUseGUI = v; }

    BeamMode GetBeamMode() const { return fMode; }
    void     SetBeamMode(BeamMode mode) { fMode = mode; }

    G4double GetStepLimit() const { return fStepLimit; }
    void     SetStepLimit(G4double v) { fStepLimit = v; }

    G4String GetPhyList() const { return fPhyList; }
    void     SetPhyList(const G4String& v) { fPhyList = v; }

    G4double GetProdCut() const { return fProdCut; }
    void     SetProdCut(G4double v) { fProdCut = v; }

    G4double GetPlanDoseFactor() const { return fPlanDoseFactor; }
    void     SetPlanDoseFactor(G4double v) { fPlanDoseFactor = v; }

    // ----------------------------------------------------------------------
    // Beam Logic & Storage
    // ----------------------------------------------------------------------
    void AssignBeam();
    void AssignSpots(const G4String& planFile);
    void AssignSpotParticleNumber(BeamMode mode);
    G4double GetSmallestSpotWeight() const;

    const std::vector<SpotParameter>& GetSpots() const { return fSpots; }

    // --- Energy Layer Trackers ---
    G4int GetTotalEnergyLayers() const { return fEnergyLayers.size(); }
    const std::vector<EnergyLayer>& GetEnergyLayers() const { return fEnergyLayers; }

private:
    void AssignDefaultParameters();
    void ParseCommandLine(int argc, char** argv);
    void PrintUsageAndExit() const;
    void PrintParameters() const;
    G4double ParseDouble(const char* arg, const std::string& parameterName) const;
    G4int    ParseInt(const char* arg, const std::string& parameterName) const;

private:
    // Core Simulation Settings
    G4int    fNumParticles;
    G4int    fMinParPerSpot;
    G4int    fThreads;
    G4String fOutfile;
    G4String fPlanfile;
    G4String fTargetMat;
    G4String fPhyList;
    G4bool   fUseGUI;

    // Beam Assignment State
    BeamMode fMode;

    // Physics/Scoring Tuning
    G4double fPlanDoseFactor;
    G4double fStepLimit;
    G4double fProdCut;

    // Spot and Layer Storage
    std::vector<SpotParameter> fSpots;
    std::vector<EnergyLayer>   fEnergyLayers;
    
    // Cached Totals for fast lookup
    G4double fTotalWeightSpots = 0.0;
    G4double fMinWeight = DBL_MAX;
    G4int    fTotalNumberSpots = 0;
    G4int    fTotalAssignedParticles = 0;
};

#endif // SIMULATION_PARAMETERS_HH