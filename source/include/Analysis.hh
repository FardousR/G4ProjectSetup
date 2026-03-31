//============================= Analysis.hh =============================
#ifndef ANALYSIS_HH
#define ANALYSIS_HH

#include "SimulationParameters.hh"
#include "DataType.hh"  // ScoredQuantity, ReactionFinal

#include "G4String.hh"
#include "G4Types.hh"

/**
 * @class Analysis
 * @brief Thread-local singleton managing histograms, data collection, and I/O.
 */
class Analysis {
public:
    // Singleton access (thread-local for Geant4 multithreading)
    static Analysis* GetAnalysis();

    // -- THIS MOVED TO PUBLIC SO G4AutoDelete CAN ACCESS IT --
    ~Analysis() = default; 

    // Prevent copy/assign
    Analysis(const Analysis&) = delete;
    Analysis& operator=(const Analysis&) = delete;

    // -- Configuration Interface --
    void SetSimulationParameters(SimulationParameters* simParams) { fSimParams = simParams; }
    SimulationParameters* GetSimulationParameters() const { return fSimParams; }

    // -- Lifecycle & I/O --
    void BookHistograms();
    void Open();
    void Save();   // Writes data to file
    void Close();  // Closes the file

    // -- Data Recording --
    void FillScoringData(const ScoredQuantity& aSDL);

private:
    // Private constructor for Singleton pattern (prevents manual instantiation)
    Analysis();

private:
    SimulationParameters* fSimParams;
    G4double              fPlanDoseFactor;

    // Fast-lookup Histogram IDs
    G4int fHistIdEne;
    G4int fHistIdFlu;
    G4int fHistIdDose;
    G4int fHistIdLETd;
    G4int fHistIdLETf;
};

#endif // ANALYSIS_HH