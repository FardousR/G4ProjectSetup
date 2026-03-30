#ifndef ACTION_INITIALIZATION_HH
#define ACTION_INITIALIZATION_HH

#include "G4VUserActionInitialization.hh"
#include "SimulationParameters.hh"

class ActionInitialization : public G4VUserActionInitialization
{
  public:
    explicit ActionInitialization(SimulationParameters* simParams);
    virtual ~ActionInitialization();

    virtual void BuildForMaster() const override;
    virtual void Build() const override;

  private:
    SimulationParameters* fSimParams; 
};

#endif