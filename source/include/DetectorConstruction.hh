#ifndef DETECTOR_CONSTRUCTION_HH
#define DETECTOR_CONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "SimulationParameters.hh"
#include "G4UserLimits.hh"
#include "G4LogicalVolume.hh"
#include "G4Region.hh"
#include "globals.hh"
#include <G4NistManager.hh>
#include <G4Material.hh>
#include <G4ThreeVector.hh>

/**
 * @class DetectorConstruction
 * @brief Defines all materials and geometry for the simulation.
 */
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    explicit DetectorConstruction(SimulationParameters* simParams);
    virtual ~DetectorConstruction();

    virtual G4VPhysicalVolume* Construct() override;

    // Optionally: a volume for scoring
    G4LogicalVolume* GetScoringVolume() const { return fScoringVolume; }

private:
    // -- Main geometry-building methods --
    G4VPhysicalVolume* ConstructWorld();
    G4VPhysicalVolume* ConstructAir(G4LogicalVolume* motherVolume);
    G4VPhysicalVolume* ConstructPMMA(G4LogicalVolume* motherVolume, G4double thick, G4double pos);
    G4VPhysicalVolume* ConstructWaterPhantom(G4LogicalVolume* motherVolume);
    G4VPhysicalVolume* ConstructPhantomWall(G4LogicalVolume* motherVolume);
    G4VPhysicalVolume* ConstructCollimator(G4LogicalVolume* motherVolume);
    G4VPhysicalVolume* ConstructDetectorCase(G4LogicalVolume* motherVolume, const G4ThreeVector& pos);
    void               ConstructMicrodosimeter(G4LogicalVolume* motherVolume, const G4ThreeVector& pos);

    // -- Helper Methods --
    G4Material*        GetOrCreatePMMA();
    G4Material*        GetOrCreateSolidWater();
    G4Material*        GetMaterial(const G4String& name);

private:
    SimulationParameters* fSimParams;     ///< Pointer to external simulation parameters
    G4UserLimits*         fStepLimit;     ///< Step limit for user steps
    G4double              fDecDis;        ///< Derived geometry parameter (aSP->GetDis() - 17.5)

    G4LogicalVolume*      fScoringVolume; ///< Optional scoring volume

    // Cache materials
    G4Material*           fPMMAMaterial;  ///< Cache pointer to PMMA material
    G4Material*           fSolidWaterMaterial;  ///< Cache pointer to PMMA material

    G4Region* detectorRegion = new G4Region("Detector_Region");
    G4Region* worldRegion = new G4Region("World_Region");

    static constexpr G4bool CHECK_OVERLAPS = true;
};

#endif // DETECTOR_CONSTRUCTION_HH
