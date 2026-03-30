#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "SimulationParameters.hh"
#include "G4ParticleGun.hh"
#include "globals.hh"
#include "G4Threading.hh"

class G4Event;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGeneratorAction(SimulationParameters* SimParams);
    virtual ~PrimaryGeneratorAction();

    virtual void GeneratePrimaries(G4Event* anEvent) override;

    // Call this from RunAction::BeginOfRunAction to reset static counters between runs
    static void ResetGlobalCounters();

private:
    void multivariateGaussian(double mean_x, double mean_y,
                              double sigma_x, double sigma_y,
                              double cov_xy, double& x, double& y);

private:
    SimulationParameters* fSimParams;
    G4ParticleGun* fParticleGun;

    // Geometry constants (Consider moving these to SimulationParameters later)
    G4double fSADx;
    G4double fSADy;
    G4double fPosZ;

    // Thread-local variables to track the current spot being processed by THIS thread
    G4int fLocalSpotIndex;
    G4int fLocalParticlesRemaining;

    // Global tracker to synchronize across all threads
    static G4int g_NextAvailableSpotIndex;
};

#endif