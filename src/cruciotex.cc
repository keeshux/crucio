#include "cruciotex.h"

using namespace crucio;
using namespace std;
using namespace TCLAP;

int main(int argc, char* argv[]) {
	try {

		// command line object
		CmdLine cmd("CRUCIOTEX - LaTeX formatter for CRUCIO", ' ', "1.0");

		// unlabeled arguments
		UnlabeledValueArg<string> fileArg("file", "Input file as " \
				"written by CRUCIO", true, "", "string", cmd);

        // switches
        SwitchArg solArg("s", "solution", "Include solution", cmd, false);
        SwitchArg fillArg("f", "fill_in", "Fill-in puzzle style", cmd, false);

		// command line parsing
		cmd.parse(argc, argv);

		// opens binary input file
		ifstream inFile(fileArg.getValue().c_str(), ios::binary);
		if (!inFile.is_open()) {
			cerr << "error: unable to open \'" << fileArg.getValue() <<
					"\'" << endl;
			return -1;
		}

		// loads result object from file
		const Output outData(inFile);

		// closes file
		inFile.close();

		// prints LaTeX output on standard output
		outData.printLatex(cout, solArg.getValue(), fillArg.getValue());
	} catch (ArgException& e) {
		cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
	}

	return 0;
}
