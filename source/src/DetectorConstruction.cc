#include "DetectorConstruction.hh"

// Geant4 headers
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4NistManager.hh"
#include "G4SubtractionSolid.hh"
#include "G4MultiUnion.hh"
#include "G4Region.hh"
#include "G4ProductionCuts.hh"
#include "G4RegionStore.hh"

// Local / project headers
#include "G4AssemblyVolume.hh"
#include "G4SDManager.hh"

//------------------------------------------------------------------------------
// Constructor / Destructor
//------------------------------------------------------------------------------
DetectorConstruction::DetectorConstruction(SimulationParameters* simParams)
 : G4VUserDetectorConstruction(),
   fSimParams(simParams)
{
    // Prepare user limits
    fStepLimit = new G4UserLimits(1.0*mm);//fSimParams->GetStepLimit());

    // Compute decdis once
}

DetectorConstruction::~DetectorConstruction()
{
    delete fStepLimit;
}

//------------------------------------------------------------------------------
// Construct() - Main entry point called by Geant4 run manager
//------------------------------------------------------------------------------
G4VPhysicalVolume* DetectorConstruction::Construct()
{
    // 1) Define the world
    auto worldPV = ConstructWorld();

    // 2) Place air
    auto airPV = ConstructAir(worldPV->GetLogicalVolume());

    auto phantomPV = ConstructWaterPhantom(airPV->GetLogicalVolume());
    auto phantomWallPV = ConstructPhantomWall(phantomPV->GetLogicalVolume());
       // G4ProductionCuts* cuts = new G4ProductionCuts;
       //   cuts->SetProductionCut(5.0*um,    G4ProductionCuts::GetIndex("gamma"));
       //   cuts->SetProductionCut(5.0*um,    G4ProductionCuts::GetIndex("e-"));
       //   cuts->SetProductionCut(5.0*um,    G4ProductionCuts::GetIndex("e+"));
       //   cuts->SetProductionCut(5.0*um,    G4ProductionCuts::GetIndex("proton"));
       //   cuts->SetProductionCut(5.0*um,    G4ProductionCuts::GetIndex("neutron"));
       // detectorRegion->SetProductionCuts(cuts);

       // worldRegion->SetUserLimits(new G4UserLimits(1.0*mm));
       // detectorRegion->SetUserLimits(new G4UserLimits(5.0*um));
  return worldPV;
}

//------------------------------------------------------------------------------
// Helper: Construct the world volume
//------------------------------------------------------------------------------
G4VPhysicalVolume* DetectorConstruction::ConstructWorld()
{
    // Use vacuum
    auto vacuum = GetMaterial("G4_Galactic");
    if (!vacuum) {
        G4Exception("DetectorConstruction::ConstructWorld()",
                    "MaterialNotFound", FatalException,
                    "G4_Galactic not found in NIST.");
    }

    // Dimensions
    const G4double worldX = 620.0 * CLHEP::mm;
    const G4double worldY = 620.0 * CLHEP::mm;
    const G4double worldZ = 1220.0 * CLHEP::mm;

    // Build the solid and logical volume
    auto solidWorld   = new G4Box("World", worldX/2, worldY/2, worldZ/2);
    auto logicWorld   = new G4LogicalVolume(solidWorld, vacuum, "World");

    // Visualization
    auto worldVis = new G4VisAttributes(G4Colour(0.0, 1.0, 1.0));
    worldVis->SetForceWireframe(true);
    logicWorld->SetVisAttributes(worldVis);

    // Set the step limit for the world volume
    // logicWorld->SetUserLimits(new G4UserLimits(10.0));

    // worldRegion->AddRootLogicalVolume(logicWorld);

    // Place physically (no mother => 0)
    auto physWorld = new G4PVPlacement(nullptr,
                                       G4ThreeVector(),
                                       logicWorld,
                                       "World",
                                       nullptr,
                                       false,
                                       0,
                                       CHECK_OVERLAPS);

    return physWorld;
}


//------------------------------------------------------------------------------
// Helper: Construct an air box inside the world
//------------------------------------------------------------------------------
G4VPhysicalVolume* DetectorConstruction::ConstructAir(G4LogicalVolume* motherVolume)
{
    auto air = GetMaterial("G4_AIR");
    const G4double airX = 600.0 * CLHEP::mm;
    const G4double airY = 600.0 * CLHEP::mm;
    const G4double airZ = 1200.0 * CLHEP::mm;

    auto solidAir   = new G4Box("AirSolid", airX/2, airY/2, airZ/2);
    auto logicAir   = new G4LogicalVolume(solidAir, air, "AirLogical");

    auto airVis = new G4VisAttributes(G4Colour(0.0, 1.0, 1.0));
    airVis->SetForceWireframe(true);
    logicAir->SetVisAttributes(airVis);

    worldRegion->AddRootLogicalVolume(logicAir);


    // Place
    return new G4PVPlacement(nullptr,
                             G4ThreeVector(),
                             logicAir,
                             "Air",
                             motherVolume,
                             false,
                             0,
                             CHECK_OVERLAPS);
}



//------------------------------------------------------------------------------
// Construct the large Water Phantom for dosimetry
//------------------------------------------------------------------------------

G4VPhysicalVolume* DetectorConstruction::ConstructWaterPhantom(G4LogicalVolume* motherVolume)
{
  auto water = GetMaterial("G4_WATER");
  if (!water) {
      G4Exception("DetectorConstruction::ConstructSolidWater()",
                  "MaterialNotFound", FatalException,
                  "G4_WATER not found in NIST.");
  }
    const G4double WaterPhantomX = 500.0 * CLHEP::mm;
    const G4double WaterPhantomY = 500.0 * CLHEP::mm;
    const G4double WaterPhantomZ = 500.0 * CLHEP::mm;

    auto solidWaterPhantom   = new G4Box("WaterPhantomSolid", WaterPhantomX/2, WaterPhantomY/2, WaterPhantomZ/2);
    auto logicWaterPhantom   = new G4LogicalVolume(solidWaterPhantom, water, "WaterPhantomLogical");

    auto WaterPhantomVis = new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.5));
    // WaterPhantomVis->SetForceWireframe(true);
    logicWaterPhantom->SetVisAttributes(WaterPhantomVis);

    worldRegion->AddRootLogicalVolume(logicWaterPhantom);


    // Place
    return new G4PVPlacement(nullptr,
                             G4ThreeVector(0, 0, WaterPhantomZ/2),
                             logicWaterPhantom,
                             "WaterPhantom",
                             motherVolume,
                             false,
                             0,
                             CHECK_OVERLAPS);
}


G4VPhysicalVolume* DetectorConstruction::ConstructPhantomWall(G4LogicalVolume* motherVolume)
{
  auto water = GetMaterial("G4_PLEXIGLASS");
  if (!water) {
      G4Exception("DetectorConstruction::ConstructSolidWater()",
                  "MaterialNotFound", FatalException,
                  "G4_WATER not found in NIST.");
  }
    const G4double WaterPhantomX = 500.0 * CLHEP::mm;
    const G4double WaterPhantomY = 500.0 * CLHEP::mm;
    const G4double WaterPhantomZ = 10.0 * CLHEP::mm;

    auto solidWaterPhantom   = new G4Box("WaterPhantomSolid", WaterPhantomX/2, WaterPhantomY/2, WaterPhantomZ/2);
    auto logicWaterPhantom   = new G4LogicalVolume(solidWaterPhantom, water, "WaterPhantomLogical");

    auto WaterPhantomVis = new G4VisAttributes(G4Colour(1.0, 1.0, 0.0));
    // WaterPhantomVis->SetForceWireframe(true);
    logicWaterPhantom->SetVisAttributes(WaterPhantomVis);

    worldRegion->AddRootLogicalVolume(logicWaterPhantom);


    // Place
    return new G4PVPlacement(nullptr,
                             G4ThreeVector(0, 0, -500/2 + WaterPhantomZ/2),
                             logicWaterPhantom,
                             "WaterPhantom",
                             motherVolume,
                             false,
                             0,
                             CHECK_OVERLAPS);
}

//------------------------------------------------------------------------------
// GetOrCreateSolidWater - create Solid Water material only once
//------------------------------------------------------------------------------
G4Material* DetectorConstruction::GetOrCreateSolidWater()
{
    if (fSolidWaterMaterial) {
        return fSolidWaterMaterial;
    }

    // Create Solid Water (epoxy-based radiotherapy phantom material)
    auto nist = G4NistManager::Instance();
    G4double density = 1.04 * CLHEP::g / CLHEP::cm3;
    fSolidWaterMaterial = new G4Material("SolidWater", density, 4);

    fSolidWaterMaterial->AddElement(nist->FindOrBuildElement("H"), 0.09);  // 9%
    fSolidWaterMaterial->AddElement(nist->FindOrBuildElement("C"), 0.66);  // 66%
    fSolidWaterMaterial->AddElement(nist->FindOrBuildElement("N"), 0.02);  // 2%
    fSolidWaterMaterial->AddElement(nist->FindOrBuildElement("O"), 0.19);  // 19%

    return fSolidWaterMaterial;
}

//------------------------------------------------------------------------------
// Helper to get a material from NIST by name
//------------------------------------------------------------------------------
G4Material* DetectorConstruction::GetMaterial(const G4String& name)
{
    return G4NistManager::Instance()->FindOrBuildMaterial(name);
}
