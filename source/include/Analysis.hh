//============================= Analysis.hh =============================
#ifndef ANALYSIS_HH
#define ANALYSIS_HH

#include "SimulationParameters.hh"
#include "DataType.hh"  // ScoredQuantity, ReactionFinal

#include "G4String.hh"
#include "G4SystemOfUnits.hh"
#include "G4AutoDelete.hh"
#include "G4ThreeVector.hh"
#include "G4AnalysisManager.hh"

#include <vector>
#include <string>
#include <array>

// Forward declarations to avoid heavy includes
class G4Step;
class G4EventManager;
class NeutronKerma;

/**
 * @class Analysis
 * @brief Thread-local singleton managing histograms, data collection, and I/O.
 */
class Analysis {
 public:
  // Singleton access (thread-local)
  static Analysis* GetAnalysis();

  // -- Constructors / Destructor --
  explicit Analysis(SimulationParameters* simParams);
  Analysis();
  ~Analysis() = default;

  // Prevent copy/assign
  Analysis(const Analysis&) = delete;
  Analysis& operator=(const Analysis&) = delete;

  // -- Core interface --
  void SetSimulationParameters(SimulationParameters* simParams) { fSimParams = simParams; }
  SimulationParameters* GetSimulationParameters() const { return fSimParams; }

  void BookHistograms();
  void Open();
  void Save();   // does Write()
  void Close();  // just closes file

  // Fill data methods
  void FillScoringData(ScoredQuantity aSDL);
  void FillDetectorData(ScoredQuantity aSDL);
  void FillStepData(ScoredQuantity aSDL);
  void FillDosimetryData(ScoredQuantity aSDL);

  // Experimental histograms (IDD / XY / YZ) – called from SteppingAction
  void FillExptHistograms(ScoredQuantity exptData);

  // Nuclear reaction (one-shot) writer (currently stub)
  void FillReactionFinal(const ReactionData& R);

  // Utility (stub kept from your file)
  G4double GetNeuKermaFactor(G4double energy);

 private:
  // -- Private helpers for experimental histos --
  void BookExptHistograms();
  // Helper functions for experimental histograms
  std::vector<G4int> FindCollimatorIndex(G4double xmm, G4double ymm) const;
  std::vector<G4int> FindXYDepthIndex(G4double zmm) const;
  std::vector<G4int> FindYZPlaneIndex(G4double xmm) const;


  G4double fPlanDoseFactor;

  // -- Private data members --

  // Simulation parameters
  SimulationParameters* fSimParams = nullptr;

  // Max KE per particle group
  static constexpr G4double maxKE[8] = {
    200.0, 200.0, 200.0, 10.0, 10.0, 2.0, 10.0, 20.0
  };

  // 1D histogram IDs for chip spectra (each array has 4 elements: one per chip)
  G4int fH1ID_enespec[8][4];

  // Experimental 1D vs Z (IDD, energy, LET) for collimator sizes
  G4int fH1ID_IDD[8];
  G4int fH1ID_energyZ[8];
  G4int fH1ID_letDataZ[8];

  // Experimental 2D XY maps at fixed depths (dose / energy / LET)
  G4int fH2ID_doseXY2D[5];
  G4int fH2ID_energyXY2D[5];
  G4int fH2ID_letDataXY2D[5];

  // Experimental 2D YZ maps at fixed X planes (dose / energy / LET)
  G4int fH2ID_doseYZ2D[3];
  G4int fH2ID_energyYZ2D[3];
  G4int fH2ID_letDataYZ2D[3];

  // 1D KE spectra per group
  G4int fH1ID_ke[8];

  // Total energy deposition per group
  G4int fH1ID_AllDetEng[8];

  // 1D energy profile histograms (8 groups)
  G4int fH1ID_enedep1D[8];

  // 2D energy profile histograms (8 groups)
  G4int fH2ID_enedep2D[8];

  // 1D energy profiles for each detector index (0 to 599)
  G4int fH1ID_DetEng[8][600];

  // Current step copy (if you use it elsewhere)
  ScoredQuantity fCurrentScoredQuantity;

  // Ntuple IDs for (future) reaction output
  int fNtupleRxnId  = -1; // "rxn"           (one row per interaction)
  int fNtupleProdId = -1; // "rxn_products"  (one row per unique product PDG with multiplicity)
};

#endif // ANALYSIS_HH
