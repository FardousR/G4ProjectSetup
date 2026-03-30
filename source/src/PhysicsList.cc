#include "PhysicsList.hh"


// Geant4 includes for building the physics
#include "G4SystemOfUnits.hh"
#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"

// EM physics
#include "G4EmStandardPhysics_option3.hh"  // EM "EMY"-like

// Decays
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"

// Extra EM
#include "G4EmExtraPhysics.hh"

// Hadronic physics
#include "G4HadronElasticPhysics.hh"
#include "G4HadronPhysicsQGSP_BIC.hh"
#include "G4HadronPhysicsQGSP_BIC_HP.hh"

#include "G4StoppingPhysics.hh"
#include "G4IonPhysics.hh"
#include "G4NeutronTrackingCut.hh"

PhysicsList::PhysicsList()
{
  // Default cut value (1 mm)
  defaultCutValue = 1.0 * mm;

  // Increase or decrease as needed
  SetVerboseLevel(1);

  // -- Register the various physics builders in the desired order --

  // 1) EM physics: "option3" is historically akin to "EMY"
  RegisterPhysics(new G4EmStandardPhysics_option3());

  // 2) Decays
  RegisterPhysics(new G4DecayPhysics());
  RegisterPhysics(new G4RadioactiveDecayPhysics());

  // 3) Extra EM (synchrotron radiation, gamma-nuclear, etc. if relevant)
  RegisterPhysics(new G4EmExtraPhysics());

  // 4) Hadronic elastic scattering
  RegisterPhysics(new G4HadronElasticPhysics());

  // 5) QGSP with BIC + HP for hadronic inelastic
  RegisterPhysics(new G4HadronPhysicsQGSP_BIC_HP());

  // 6) Stopping physics (e.g. for stopping pions)
  RegisterPhysics(new G4StoppingPhysics());

  // 7) Ion physics
  RegisterPhysics(new G4IonPhysics());

  // 8) Neutron tracking cut
  RegisterPhysics(new G4NeutronTrackingCut());
}

PhysicsList::~PhysicsList() = default;

void PhysicsList::ConstructParticle()
{
  // This ensures all registered physics builders can create particles
  G4VModularPhysicsList::ConstructParticle();
}

void PhysicsList::ConstructProcess()
{
  // Let the registered physics builders construct processes
  G4VModularPhysicsList::ConstructProcess();
}

void PhysicsList::SetCuts()
{
  // Sets the default cut values for all particles
  SetCutsWithDefault();
}
