#ifndef PHYSICS_LIST_HH
#define PHYSICS_LIST_HH

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

/**
 * @class PhysicsList
 * @brief Custom physics list replicating the QGSP_BIC_EMY reference list:
 *        - QGSP + Binary Cascade for hadronic physics
 *        - EM Standard Option 3 (historical "EMY") for EM physics
 */
class PhysicsList : public G4VModularPhysicsList
{
public:
  PhysicsList();
  virtual ~PhysicsList();

  // Mandatory overrides
  virtual void ConstructParticle() override;
  virtual void ConstructProcess()  override;
  virtual void SetCuts()          override;
};

#endif // PHYSICS_LIST_HH
