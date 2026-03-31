#ifndef PROCESS_LET_HH
#define PROCESS_LET_HH

#include <string>

class ProcessLET {
public:
    // Takes the original output filename, processes it, and saves the _mod version
    static void Process(const std::string& inputFile);
};

#endif // PROCESS_LET_HH
