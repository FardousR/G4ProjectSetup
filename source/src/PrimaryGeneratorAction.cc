#include "PrimaryGeneratorAction.hh"
#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4AutoLock.hh"
#include <cmath>
#include <algorithm> // For std::max

// Initialize the global spot index
G4int PrimaryGeneratorAction::g_NextAvailableSpotIndex = 0;

namespace {
    G4Mutex spotSelectionMutex = G4MUTEX_INITIALIZER;
}

PrimaryGeneratorAction::PrimaryGeneratorAction(SimulationParameters* SimParams)
    : G4VUserPrimaryGeneratorAction(),
      fSimParams(SimParams),
      fLocalSpotIndex(-1),
      fLocalParticlesRemaining(0)
{
    fParticleGun = new G4ParticleGun(1);
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    fParticleGun->SetParticleDefinition(particleTable->FindParticle("proton"));

    // Set fixed geometry constants
    fSADx = 2560.0 * mm;
    fSADy = 2000.0 * mm;
    fPosZ = 500.0 * mm;
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
}

void PrimaryGeneratorAction::ResetGlobalCounters()
{
    G4AutoLock lk(&spotSelectionMutex);
    g_NextAvailableSpotIndex = 0;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    const std::vector<SpotParameter>& spots = fSimParams->GetSpots();
    if (spots.empty()) return;

    // --- OPTIMIZED THREAD-SAFE SELECTION (Spot-Claiming) ---
    // If this thread has finished its current spot, fetch the next available spot
    if (fLocalParticlesRemaining <= 0) {
        G4AutoLock lk(&spotSelectionMutex);
        if (g_NextAvailableSpotIndex < (G4int)spots.size()) {
            fLocalSpotIndex = g_NextAvailableSpotIndex;
            fLocalParticlesRemaining = spots[fLocalSpotIndex].num_par;
            g_NextAvailableSpotIndex++; // Advance global index for the next thread
        } else {
            fLocalSpotIndex = -1; // No more spots available
        }
    }

    // If all spots across all threads have been processed, exit
    if (fLocalSpotIndex == -1 || fLocalSpotIndex >= (G4int)spots.size()) {
        return;
    }

    // Get the spot this thread is currently responsible for
    const SpotParameter& selectedSpot = spots[fLocalSpotIndex];
    fLocalParticlesRemaining--; // Consume one particle from this thread's quota

    // --- KINEMATICS CALCULATION ---
    G4double spot_x_pos = selectedSpot.spotx * ((fSADx - fPosZ) / fSADx);
    G4double spot_y_pos = selectedSpot.spoty * ((fSADy - fPosZ) / fSADy);
    G4double spot_x_dir = std::atan(selectedSpot.spotx / fSADx);
    G4double spot_y_dir = std::atan(selectedSpot.spoty / fSADy);

    G4double energy = G4RandGauss::shoot(selectedSpot.energy, selectedSpot.e_spread);

    // Properly scoped local variables for the output of the Cholesky Decomposition
    double particlePosX, particleDirX, particlePosY, particleDirY;

    multivariateGaussian(spot_x_pos, spot_x_dir, selectedSpot.sig_x,
                         selectedSpot.div_x, -selectedSpot.cov_x, particlePosX, particleDirX);

    multivariateGaussian(spot_y_pos, spot_y_dir, selectedSpot.sig_y,
                         selectedSpot.div_y, -selectedSpot.cov_y, particlePosY, particleDirY);

    // Apply to Gun
    fParticleGun->SetParticleEnergy(energy * MeV);
    fParticleGun->SetParticlePosition(G4ThreeVector(particlePosX, particlePosY, -fPosZ));
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(particleDirX, particleDirY, 1.0));
    fParticleGun->GeneratePrimaryVertex(anEvent);
}

void PrimaryGeneratorAction::multivariateGaussian(double mean_x, double mean_y,
                                                  double sigma_x, double sigma_y,
                                                  double cov_xy, double& x, double& y)
{
    double z1 = G4RandGauss::shoot(0.0, 1.0);
    double z2 = G4RandGauss::shoot(0.0, 1.0);

    double L11 = sigma_x;
    double L21 = cov_xy / sigma_x;
    double L22 = std::sqrt(std::max(0.0, (sigma_y * sigma_y) - (L21 * L21)));

    x = mean_x + L11 * z1;
    y = mean_y + L21 * z1 + L22 * z2;
}