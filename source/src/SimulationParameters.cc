#include "SimulationParameters.hh"
#include <getopt.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iomanip>

// -----------------------------------------------------------------------------
// Constructor & Destructor
// -----------------------------------------------------------------------------
SimulationParameters::SimulationParameters(int argc, char** argv) {
    AssignDefaultParameters();
    ParseCommandLine(argc, argv);
    // Note: File loading and printing are deferred to Initialize() for safety
}

SimulationParameters::~SimulationParameters() {}

void SimulationParameters::Initialize() {
    AssignBeam();
    PrintParameters();
}

// -----------------------------------------------------------------------------
// Setup & Command Line Parsing
// -----------------------------------------------------------------------------
void SimulationParameters::AssignDefaultParameters() {
    fNumParticles = 100000;
    fMinParPerSpot = 10;
    fThreads = 1;
    fOutfile = "Output.root";
    fPlanfile = "Plan.dat";
    fTargetMat = "G4_WATER";
    fUseGUI = false;

    // Default starting state
    fMode = kSequentialMuRand;

    fStepLimit = 0.1 * mm;
    fProdCut = 0.1 * mm;
    fPhyList = "QGSP_BIC_HP_EMZ";
    fPlanDoseFactor = 1.0;
}

void SimulationParameters::ParseCommandLine(int argc, char** argv) {
    static struct option long_options[] = {
        {"num_particles", required_argument, nullptr, 'n'},
        {"parperspot",    required_argument, nullptr, 'N'},
        {"threads",       required_argument, nullptr, 't'},
        {"outfile",       required_argument, nullptr, 'o'},
        {"planfile",      required_argument, nullptr, 'L'},
        {"gui",           no_argument,       nullptr, 'g'},
        {"ratio",         no_argument,       nullptr, 'r'},
        {"minpar",        no_argument,       nullptr, 'm'},
        {"steplimit",     required_argument, nullptr, 's'},
        {"phylist",       required_argument, nullptr, 'p'},
        {"targetmat",     required_argument, nullptr, 'T'},
        {"prodcut",       required_argument, nullptr, 'c'},
        {"planfactor",    required_argument, nullptr, 'f'},
        {"help",          no_argument,       nullptr, 'h'},
        {0, 0, 0, 0}
    };

    int opt, option_index = 0;
    while ((opt = getopt_long(argc, argv, "n:N:t:o:L:grmp:s:T:c:f:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'n': SetNumParticles(ParseInt(optarg, "num_particles")); break;
            case 'N': 
                SetMinParPerSpot(ParseInt(optarg, "parperspot"));
                fMode = kSequentialMinPar; 
                break;
            case 't': SetThreads(ParseInt(optarg, "threads")); break;
            case 'o': SetOutfile(optarg); break;
            case 'L': SetPlanfile(optarg); break;
            case 'g': SetUseGUI(true); break;
            case 'r': fMode = kSequentialMuRatio; break;
            case 'm': fMode = kSequentialMinPar; break;
            case 's': SetStepLimit(ParseDouble(optarg, "steplimit") * mm); break;
            case 'p': SetPhyList(optarg); break;
            case 'T': SetTargetMat(optarg); break;
            case 'c': SetProdCut(ParseDouble(optarg, "prodcut") * mm); break;
            case 'f': SetPlanDoseFactor(ParseDouble(optarg, "planfactor")); break;
            case 'h': PrintUsageAndExit(); break;
            default: 
                throw std::invalid_argument("Unknown command line option provided. Use -h for help.");
        }
    }
}

// -----------------------------------------------------------------------------
// Helpers & Printing
// -----------------------------------------------------------------------------
G4double SimulationParameters::ParseDouble(const char* arg, const std::string& name) const {
    try { 
        return std::stod(arg); 
    } catch(...) { 
        throw std::invalid_argument("Failed to parse double argument for: " + name); 
    }
}

G4int SimulationParameters::ParseInt(const char* arg, const std::string& name) const {
    try { 
        return std::stoi(arg); 
    } catch(...) { 
        throw std::invalid_argument("Failed to parse integer argument for: " + name); 
    }
}

void SimulationParameters::PrintUsageAndExit() const {
    std::cout << "Usage: ./LETrix -n [particles] -N [min_par] -L [plan] [options]\n";
    exit(0);
}

void SimulationParameters::PrintParameters() const {
    const int w = 30;

    std::cout << "\n" << std::string(55, '=') << "\n";
    std::cout << "           GEANT4 SIMULATION PARAMETERS\n";
    std::cout << std::string(55, '=') << "\n";

    // Beam and Particle Logic
    std::cout << std::left << std::setw(w) << "Primary Particles"     << ": " << GetNumParticles() << "\n";
    std::cout << std::left << std::setw(w) << "Min Particles / Spot"  << ": " << GetMinParPerSpot() << "\n";
    std::cout << std::left << std::setw(w) << "Plan File"             << ": " << GetPlanfile() << "\n";

    std::cout << std::left << std::setw(w) << "Assignment Mode"       << ": ";
    switch (fMode) {
        case kSequentialMinPar:  std::cout << "Sequential MinPar\n"; break;
        case kSequentialMuRatio: std::cout << "Sequential Ratio\n"; break;
        case kSequentialMuRand:  std::cout << "Sequential MuRand\n"; break;
        default:                 std::cout << "Undefined\n"; break;
    }

    std::cout << std::string(55, '-') << "\n";

    // Hardware and Physics
    std::cout << std::left << std::setw(w) << "Execution Threads"     << ": " << GetThreads() << "\n";
    std::cout << std::left << std::setw(w) << "Physics List"          << ": " << GetPhyList() << "\n";
    std::cout << std::left << std::setw(w) << "Target Material"       << ": " << GetTargetMat() << "\n";
    std::cout << std::left << std::setw(w) << "Output File"           << ": " << GetOutfile() << "\n";

    std::cout << std::string(55, '-') << "\n";

    // Precision and Limits
    std::cout << std::left << std::setw(w) << "Step Limit"            << ": " << GetStepLimit() / mm << " mm\n";
    std::cout << std::left << std::setw(w) << "Production Cut"        << ": " << GetProdCut() / mm << " mm\n";
    std::cout << std::left << std::setw(w) << "Plan Dose Factor"      << ": " << GetPlanDoseFactor() << "\n";
    std::cout << std::left << std::setw(w) << "GUI Enabled"           << ": " << (GetUseGUI() ? "Yes" : "No") << "\n";

    std::cout << std::string(55, '=') << "\n";

    // --- Print Energy Layer Summary ---
    if (!fEnergyLayers.empty()) {
        std::cout << "\n--- Energy Layer Summary ---\n"
                  << " Total Spots    : " << fTotalNumberSpots << "\n"
                  << " Total Layers   : " << fEnergyLayers.size() << "\n\n";
                  
        for (size_t i = 0; i < fEnergyLayers.size(); ++i) {
            std::cout << " Layer " << std::setw(2) << i + 1 << "     : " 
                      << std::fixed << std::setprecision(2) << std::setw(7) << fEnergyLayers[i].energy << " MeV | " 
                      << std::setw(4) << fEnergyLayers[i].num_spots << " spots\n";
        }
        std::cout << std::string(28, '-') << "\n";
    }
    std::cout << std::endl;
}