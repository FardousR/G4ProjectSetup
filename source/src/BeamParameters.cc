#include "SimulationParameters.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <numeric>
#include <algorithm>

// -----------------------------------------------------------------------------
// Beam & Plan Logic
// -----------------------------------------------------------------------------
void SimulationParameters::AssignBeam() {
    this->AssignSpots(this->GetPlanfile());
    if (fSpots.empty() || fMode == kModeUndefined) return;

    this->AssignSpotParticleNumber(fMode);

    // Update master count to match strictly assigned distribution
    this->SetNumParticles(fTotalAssignedParticles);

    // Ratio of total plan weight/number of particles to the total simulated particles
    if (fTotalAssignedParticles > 0) {
        fPlanDoseFactor = fTotalWeightSpots / static_cast<double>(fTotalAssignedParticles);
    } else {
        fPlanDoseFactor = 0.0; 
    }
}

void SimulationParameters::AssignSpots(const G4String& planFile) {
    std::ifstream inputFile(planFile);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Error: Unable to open plan file: " + planFile);
    }

    fSpots.clear();
    fEnergyLayers.clear();
    fTotalWeightSpots = 0.0;
    fMinWeight = DBL_MAX;

    std::string line;
    while (std::getline(inputFile, line)) {
        if (line.empty() || line[0] == '[') continue; // Skip headers

        std::stringstream ss(line);
        SpotParameter s;

        if (!(ss >> s.energy >> s.e_spread >> s.spotx >> s.spoty >> s.sig_x >> s.sig_y
                 >> s.div_x >> s.div_y >> s.cov_x >> s.cov_y >> s.weight)) {
            continue;
        }

        // Unit conversions to Geant4 standard
        s.energy *= 1000.0; s.e_spread *= 1000.0;
        s.spotx *= 10.0; s.spoty *= 10.0;
        s.sig_x = (s.sig_x * 10.0) / 2.355; s.sig_y = (s.sig_y * 10.0) / 2.355;
        s.cov_x *= 10.0; s.cov_y *= 10.0;

        s.spot_num = fSpots.size();
        s.num_par = 0;
        s.num_current_par = 0;

        if (s.weight < fMinWeight) fMinWeight = s.weight;
        fTotalWeightSpots += s.weight;
        fSpots.push_back(s);

        // --- Energy Layer Tracking ---
        // New layer if list is empty or energy differs by more than 1 keV
        if (fEnergyLayers.empty() || std::abs(fEnergyLayers.back().energy - s.energy) > 1e-3) {
            EnergyLayer newLayer;
            newLayer.energy = s.energy;
            newLayer.num_spots = 1;
            fEnergyLayers.push_back(newLayer);
        } else {
            fEnergyLayers.back().num_spots++;
        }
    }
    fTotalNumberSpots = fSpots.size();
    inputFile.close();
}

void SimulationParameters::AssignSpotParticleNumber(BeamMode mode) {
    if (fSpots.empty()) return;

    G4int requested = GetNumParticles();

    if (mode == kSequentialMuRand) {
        double accumulated = 0.0;
        G4int assigned = 0;
        for (auto& s : fSpots) {
            accumulated += (double)requested * (s.weight / fTotalWeightSpots);
            G4int count = (G4int)std::floor(accumulated + 0.5) - assigned;
            s.num_par = count;
            assigned += count;
        }
    }
    else if (mode == kSequentialMuRatio) {
        G4int assignedSoFar = 0;
        for (auto& s : fSpots) {
            s.num_par = static_cast<G4int>(std::floor(requested * (s.weight / fTotalWeightSpots)));
            assignedSoFar += s.num_par;
        }
        int remainder = requested - assignedSoFar;
        for (int i = 0; i < remainder; ++i) {
            fSpots[i % fSpots.size()].num_par += 1;
        }
    }
    else if (mode == kSequentialMinPar) {
        G4int minP = GetMinParPerSpot();
        for (auto& s : fSpots) {
            s.num_par = static_cast<G4int>(std::round(minP * (s.weight / fMinWeight)));
        }
    }

    fTotalAssignedParticles = 0;
    for (auto& s : fSpots) {
        s.num_current_par = 0; 
        fTotalAssignedParticles += s.num_par;
    }
}

G4double SimulationParameters::GetSmallestSpotWeight() const {
    return fSpots.empty() ? 0.0 : fMinWeight;
}