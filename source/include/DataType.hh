#ifndef DataType_h
#define DataType_h 1
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include "G4SteppingManager.hh"
#include "G4VTouchable.hh"
#include "G4VPhysicalVolume.hh"
#include "G4TrackVector.hh"
#include "G4ProcessType.hh"
#include "G4Region.hh"
#include "G4ThreeVector.hh"

// Avoid using namespace std in headers.

extern struct interlacing_beam {
    int n_beam;
    int beam_direction;
    double y_pos;
    double weight;
} interlacing_beam_;

extern struct SetupParameter {
    int n_particle;
    int n_progress;
    int n_thread;
    std::string out_file;
    bool place_phantom;
    bool place_mouse;
    bool use_gui;
    bool use_coll;
    bool use_shield;
} SetupParameter_;

extern struct PhysicsParameter {
    std::string phy_list;
    double step_length;
    bool use_sl_neutral;
    std::array<double, 4> production_cut;
} PhysicsParameter_;

extern struct SpotParameter {
    int spot_num;
    double energy;
    double e_spread;
    double spotx;
    double spoty;
    double sig_x;
    double sig_y;
    double div_x;
    double div_y;
    double cov_x;
    double cov_y;
    double weight;
    int    num_par;
    int    num_current_par;
} SpotParameter_;

extern struct BeamParameter {
    std::string particle;
    std::string planFile;
    double position;
    std::array<double, 2> sad;
    std::vector<SpotParameter> spots;
    double total_weight;
    int num_spots;
} BeamParameter_;

extern struct PtvParameter {
    std::array<double, 2> xRange;
    std::array<double, 2> yRange;
    std::array<double, 2> zRange;
} PtvParameter_;

extern struct CollimatorParameter {
    std::string mat;
    std::array<double, 3> len;
    std::array<double, 3> pos;
    std::array<double, 3> angle;
    std::array<double, 2> maxSlitExtX;
    std::array<double, 2> maxSlitExtY;
    double ctc;
    double throughput;
    double slitWidth;
    std::vector<double> peakCenters;
    std::vector<double> valleyCenters;
} CollimatorParameter_;

extern struct ShieldParameter {
    std::string mat;
    double pos[3];
    double size[3];
} ShieldParameter_;

extern struct VoxelSegment {
    G4ThreeVector center;    // Center of the voxel
    G4double stepLength;     // Length of the step within the voxel
    G4double energyDeposit;  // Energy deposit in the voxel
} VoxelSegment_;

extern struct HistogramParameter {
    std::array<double, 2> xRange;
    std::array<double, 2> yRange;
    std::array<double, 2> zRange;
    std::array<double, 2> xBinNum;
    std::array<double, 2> yBinNum;
    std::array<double, 2> zBinNum;
    std::array<double, 3> xBinWidth;
    std::array<double, 3> yBinWidth;
    std::array<double, 3> zBinWidth;
} HistogramParameter_;

extern struct HistogramSize {
    double xBinNum;
    double yBinNum;
    double zBinNum;
    std::array<double, 2> xRange;
    std::array<double, 2> yRange;
    std::array<double, 2> zRange;
} HistogramSize_;

extern struct HistogramScoringParameter {
    std::array<double, 3> xProfile;
    std::array<double, 3> yProfile;
    std::array<double, 3> zProfile;
} HistogramScoringParameter_;

// Step / detector scored quantity (existing)
extern struct ScoredQuantity {
  int EventID;
  int TrackID;
  int ParentID;
  int ProcessSubType;
  int ParticleGenType;
  int ParticlePDGcode;
  double ParticleCharge;
  double StepInitialKE;
  double StepFinalKE;
  double StepMeanKE;
  double TotalSecondaryEnergy;
  double EnergyDeposit;
  double DetEnergyDeposit;
  double DetIniKE;
  double DetFinKE;
  double DetMeanKE;
  double DetSL;
  double StepLength;
  double MaterialDensity;
  double WatDoseFactor;
  double WatSPFactor;
  double WatEnergyFactor;
  double StoppingPower;
  double WatStoppingPower;
  double GeometryVolume;
  int det_no;
  double det_x;
  double det_y;
  double det_z;
  int det_i;
  int det_j;
  std::string ParticleName;
  std::string ProcessName;
  std::string GeometryComponent;
  const G4Step* CurrentStep;
  G4ThreeVector PreStepPosition;
  G4ThreeVector PostStepPosition;
  G4ThreeVector EnergyDepositPoint;
  G4ProcessType ProcessType;
} ScoredQuantity_;

// -----------------------------------------------------------------------------
// 1. Step-level data (one entry per step)
// -----------------------------------------------------------------------------
extern struct StepData
{
  // --- Event / track identifiers ---
  int EventID        = 0;
  int TrackID        = 0;
  int ParentID       = 0;

  // --- Particle info ---
  int    ParticlePDGcode  = 0;
  double ParticleCharge   = 0.0;
  int    ParticleGenType  = 0;    // 1 = primary, 2 = secondary (your convention)
  std::string ParticleName;

  // --- Process info (for the step that ended at PostStepPoint) ---
  std::string  ProcessName;
  G4ProcessType ProcessType = G4ProcessType::fNotDefined;
  int           ProcessSubType = -1;

  // --- Geometry / detector position for this step ---
  std::string GeometryComponent;  // volume name
  double      GeometryVolume = 0.0;

  int    det_no = 0;     // detector copy number
  double det_x  = 0.0;   // detector volume centre (world or mother frame)
  double det_y  = 0.0;
  double det_z  = 0.0;
  int    det_i  = 0;     // row index, if using matrix layout
  int    det_j  = 0;     // column index

  // --- Kinematics and energy at step level ---
  double StepInitialKE        = 0.0;
  double StepFinalKE          = 0.0;
  double StepMeanKE           = 0.0;
  double TotalSecondaryEnergy = 0.0;  // KE_pre - KE_post - EnergyDeposit

  double EnergyDeposit        = 0.0;  // dE in this step
  double StepLength           = 0.0;  // step length
  double MaterialDensity      = 0.0;  // g/cm^3 or kg/m^3 converted

  // --- Dosimetry-related factors at step level ---
  double WatDoseFactor    = 0.0;  // water dose conversion factor, if used
  double WatSPFactor      = 0.0;  // water stopping power factor
  double WatEnergyFactor  = 0.0;  // water energy scaling factor
  double StoppingPower    = 0.0;  // stopping power in material
  double WatStoppingPower = 0.0;  // stopping power in water

  // --- Positions along the step ---
  G4ThreeVector PreStepPosition;
  G4ThreeVector PostStepPosition;
  G4ThreeVector EnergyDepositPoint;  // sampled point along step for scoring

  // --- Optional: pointer back to the Geant4 step (if you really need it) ---
  const G4Step* CurrentStep = nullptr;
}StepData_;

extern struct DetectorData
{
  // Full step context for that track in that detector (you can keep or trim)
  StepData Step;  // provides EventID, TrackID, particle info, geometry, etc.

  // --- Detector-integrated quantities ---
  double DetEnergyDeposit = 0.0; // total dE in this detector (sum of steps)
  double DetIniKE         = 0.0; // KE at detector entrance
  double DetFinKE         = 0.0; // KE at detector exit (or stop)
  double DetMeanKE        = 0.0; // average (DetIniKE + DetFinKE)/2
  double DetSL            = 0.0; // total step length in detector

  // For convenience, detector-integrated “missing energy” to secondaries
  double TotalSecondaryEnergy = 0.0; // DetIniKE - DetFinKE - DetEnergyDeposit;
}DetectorData_;


// -----------------------------------------------------------------------------
// 3. Reaction-level data (per nuclear-like interaction)
// -----------------------------------------------------------------------------
extern struct ReactionData
{
  int EventID           = 0;
  int ProjectileTrackID = 0;

  int PDG_P             = 0;   // projectile PDG
  int PDG_T             = 0;   // target PDG (0 if unknown)

  G4ThreeVector Vertex;        // interaction vertex in world coordinates

  std::string ProcessName;     // e.g. "protonInelastic", "photoneutron"

  // Products, aggregated by PDG code
  std::vector<int> ProductPDG;   // unique PDGs of outgoing particles / fragments
  std::vector<int> ProductMult;  // multiplicities aligned with ProductPDG
}ReactionData_;

extern struct SourcePoint {
  double energy;
  double posx;
  double posy;
  double divx;
  double divy;
} SourcePoint_;

#endif
