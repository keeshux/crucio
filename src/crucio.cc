#include "crucio.h"

using namespace crucio;
using namespace std;
using namespace TCLAP;

int main(int argc, char* argv[])
{

    // defaults to failure
    int status = -1;

    // heap objects
    Matcher* inMatcher = NULL;
    Compiler* inCpl = NULL;
    Walk* inWalk = NULL;

    try {

        // command line object
        CmdLine cmd("CRUCIO - crossword and fill-in compiler", ' ', "1.0");

        // sets constrained fill values
        vector<string> allowedFillStr;
        allowedFillStr.push_back("letter");
        allowedFillStr.push_back("word");
        ValuesConstraint<string> allowedFillVals(allowedFillStr);

        // sets constrained walk values
        vector<string> allowedWalkStr;
        allowedWalkStr.push_back("bfs");
        allowedWalkStr.push_back("dfs");
        ValuesConstraint<string> allowedWalkVals(allowedWalkStr);

        // sets default random seed to current time
        time_t nowTime;
        time(&nowTime);
        const uint32_t nowTicks = (uint32_t) nowTime;

        // switches
        SwitchArg uniqueArg("U", "unique", "Fill grid with unique words",
                            false);
        SwitchArg determArg("D", "deterministic", "Cycle until deterministic " \
                            "filling (required for fill-in puzzles)", false);
        SwitchArg verboseArg("v", "verbose", "Print out algorithmic steps",
                             false);

        // labeled arguments
        ValueArg<string> dictArg("d", "dictionary", "Dictionary file",
                                 true, "", "dictionary");
        ValueArg<string> gridArg("g", "grid", "Grid file", true,
                                 "", "grid");
        ValueArg<string> fillArg("f", "fill", "Filling strategy", false,
                                 "letter", &allowedFillVals);
        ValueArg<string> walkArg("w", "walk", "Cell graph walk for variable " \
                                 "ordering", false, "bfs", &allowedWalkVals);
        ValueArg<uint32_t> seedArg("r", "seed", "Random seed", false,
                                   nowTicks * nowTicks, "seed");

        // unlabeled arguments
        UnlabeledValueArg<string> fileArg("file", "Output file " \
                                          "for reuse in external applications (NOTE: " \
                                          "big endian byte order)", true, "", "output");

        // arguments insertion (last to first)
        cmd.add(gridArg);
        cmd.add(dictArg);
        cmd.add(fileArg);
        cmd.add(verboseArg);
        cmd.add(determArg);
        cmd.add(uniqueArg);
        cmd.add(seedArg);
        cmd.add(walkArg);
        cmd.add(fillArg);

        // command line parsing
        cmd.parse(argc, argv);

        // verbosity
        setVerbose(verboseArg.getValue());

        // allocates data structures through input arguments
        const Alphabet inAlphabet = crucio::LETTERS;
        const Grid inGrid(gridArg.getValue());

        // chooses matcher
        switch (inAlphabet) {
        case LETTERS:
            inMatcher = new LanguageMatcher(dictArg.getValue());
            break;

        case DIGITS:
            inMatcher = new SolutionMatcher();
            break;
        }

        // creates dictionary with matcher
        Dictionary inDict(inAlphabet, inMatcher);

#ifndef USE_BENCHMARK
        // binary output
        ofstream outFile(fileArg.getValue().c_str(), ios::binary);
        if (!outFile.is_open()) {
            throw CrucioException("main: unable to write output file");
        }
#endif

        // prints out input description
        printInputDescription(cout, inDict, inGrid,
                              fillArg.getValue(), walkArg.getValue(),
                              uniqueArg.getValue(), determArg.getValue(),
                              seedArg.getValue(), verboseArg.getValue());

        // model building
        Model inModel(&inDict, &inGrid);

        // if verbose prints out model description too
        if (verboseArg.getValue()) {
            cout << "[MODEL]" << endl << endl;
            printModelDescription(cout, inModel);
        }

        // compiler setup
        if (fillArg.getValue() == "letter") {
            inCpl = new LetterCompiler();
        } else if (fillArg.getValue() == "word") {
            inCpl = new WordCompiler();
        }
        inCpl->setUnique(uniqueArg.getValue());
        inCpl->setDeterministic(determArg.getValue());

        // walk selection
        if (walkArg.getValue() == "bfs") {
            inWalk = new BFSWalk();
        } else if (walkArg.getValue() == "dfs") {
            inWalk = new DFSWalk();
        } else {
            // assert(false)
        }

        // pseudorandom generator initialization
        srand(seedArg.getValue());

        if (verboseArg.getValue()) {
            cout << "[ALGORITHM]" << endl << endl;
        }

        // execution
        switch (inCpl->compile(&inModel, *inWalk)) {
        case Compiler::SUCCESS: {

                // success!
                status = 0;

                // prints out results
                cout << "[OUTPUT]" << endl << endl;
                //printModelGrid(cout, inModel);
                printOutput(cout, inModel);

#ifndef USE_BENCHMARK
                // builds result data and writes them to output file
                const Output outData(inModel);
                outData.printRaw(outFile);
                outFile.close();
#endif

                break;
            }
        case Compiler::FAILURE_IMPOSSIBLE: {
                cout << "failure: no solutions found" << endl;
                break;
            }
        case Compiler::FAILURE_OVERCONSTRAINED: {
                cout << "failure: model is overconstrained" << endl;
                break;
            }
        case Compiler::FAILURE_ND_GRID: {
                cout << "failure: grid layout forces non-determinism" << endl;
                break;
            }
        }

#ifndef USE_BENCHMARK
        // closes output file (FIXME: leaks here on CrucioException)
        outFile.close();
#endif
    } catch (ArgException& e) {
        cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    } catch (CrucioException& e) {
        cerr << e.what() << endl;
    }

    // deallocates compiler and walk
    if (inCpl) {
        delete inCpl;
    }
    if (inMatcher) {
        delete inMatcher;
    }
    if (inWalk) {
        delete inWalk;
    }

    // exit status
    return status;
}
