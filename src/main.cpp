#include <iostream>
#include <fstream>
#include <vector>
#include "place.h"
using namespace std;

int main(int argc, char** argv)
{
    fstream input;
    string inputFileName = argv[1];

    if (argc == 2) {
        input.open(argv[1], ios::in);
        if (!input) {
            cerr << "Cannot open the input file \"" << argv[1]
                 << "\". The program will be terminated..." << endl;
            exit(1);
        }
    }
    else {
        cerr << "Usage: ./pl <input file>" << endl;
        exit(1);
    }

    Placer* placer = new Placer(input);
    placer -> initialPartition();
    placer -> outputPartitionResult(inputFileName);
    placer -> viaDecision(inputFileName);
    placer -> outputViaDecisionResult(inputFileName);

    return 0;
}
