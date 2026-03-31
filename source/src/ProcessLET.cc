#include "ProcessLET.hh"
#include <iostream>

// ROOT Includes
#include "TFile.h"
#include "TH1D.h"

void ProcessLET::Process(const std::string& inputFile) {
    std::cout << "\n>>> Starting Post-Processing: Calculating final LET..." << std::endl;

    // 1. Open the raw Geant4 output file
    TFile* fin = TFile::Open(inputFile.c_str(), "READ");
    if (!fin || fin->IsZombie()) {
        std::cerr << ">>> Error: Could not open " << inputFile << " for post-processing." << std::endl;
        return;
    }

    // 2. Extract original histograms
    TH1D* hEne      = (TH1D*)fin->Get("Energy");
    TH1D* hFlu      = (TH1D*)fin->Get("Fluence");
    TH1D* hDose     = (TH1D*)fin->Get("Dose");
    TH1D* hLETd_num = (TH1D*)fin->Get("LET_dose");
    TH1D* hLETf_num = (TH1D*)fin->Get("LET_fluence");

    // Safety check to ensure histograms exist
    if (!hEne || !hFlu || !hLETd_num || !hLETf_num) {
        std::cerr << ">>> Error: Missing required histograms in the input file!" << std::endl;
        fin->Close();
        return;
    }

    // 3. Generate the new "_mod.root" filename
    std::string outputFile = inputFile;
    size_t pos = outputFile.find(".root");
    if (pos != std::string::npos) {
        outputFile.replace(pos, 5, "_mod.root");
    }

    TFile* fout = new TFile(outputFile.c_str(), "RECREATE");

    // 4. Clone and Divide
    TH1D* hLETd_final = (TH1D*)hLETd_num->Clone("LETd_Final");
    hLETd_final->SetTitle("Final Dose-Averaged LET;Depth (mm);LET (keV/um)");
    hLETd_final->Divide(hEne);

    TH1D* hLETf_final = (TH1D*)hLETf_num->Clone("LETf_Final");
    hLETf_final->SetTitle("Final Fluence-Averaged LET;Depth (mm);LET (keV/um)");
    hLETf_final->Divide(hFlu);

    // 5. Save everything to the new file
    hEne->Write();
    hFlu->Write();
    if (hDose) hDose->Write();
    hLETd_final->Write();
    hLETf_final->Write();

    // 6. Clean up
    fout->Close();
    fin->Close();
    std::cout << ">>> Successfully generated processed LET file: " << outputFile << "\n" << std::endl;
}
