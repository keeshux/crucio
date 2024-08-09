//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#include "Grid.h"
#include "FillIn.h"

using namespace crucio;
using namespace std;

int main(int argc, char *argv[])
{
    unsigned rows;
    unsigned columns;
    unsigned minLength;
    unsigned maxLength;

    //crucio::setVerbose(false);

#if 1
    rows = atoi(argv[1]);
    columns = atoi(argv[2]);
    minLength = atoi(argv[3]);
    maxLength = atoi(argv[4]);
#else
//    rows = 5; columns = 9; minLength = 2; maxLength = 6;
    rows = 9; columns = 13; minLength = 2; maxLength = 7;
//    rows = 21; columns = 31; minLength = 2; maxLength = 8;
//    rows = 31; columns = 51; minLength = 2; maxLength = 8;
#endif
    
    unsigned seed = 0;
    if (argc > 5) {
        seed = atoi(argv[5]);
    } else {
        seed = (unsigned)time(NULL);
    }
    srand(seed);
    
    GridStructure structure;
    structure.m_rows = rows;
    structure.m_columns = columns;
    structure.m_minLength = minLength;
    structure.m_maxLength = maxLength;
    
    FillIn fillIn(structure, (unsigned (*)())&rand);
    cerr << "grid is " << rows << "x" << columns << endl;
    cerr << "random seed = " << seed << endl;
    
    // fill-in creation algorithm
    fillIn.layout();
    cerr << fillIn << endl;

    // produce grid object
    Grid *grid = fillIn.createGrid();

    // grid output in crucio format to stdout
    cout << grid->getRows() << endl;
    cout << grid->getColumns() << endl;
    cout << *grid;

    // words count by length
    unsigned *defCount = new unsigned[maxLength + 1];
    fill(defCount, defCount + maxLength + 1, 0);

    // across + down words
    const vector<Definition *> &across = grid->getAcrossDefinitions();
    const vector<Definition *> &down = grid->getDownDefinitions();
    vector<Definition *>::const_iterator di;
    for (di = across.begin(); di != across.end(); ++di) {
        ++defCount[(*di)->getLength()];
    }
    for (di = down.begin(); di != down.end(); ++di) {
        ++defCount[(*di)->getLength()];
    }

    // print count by length
    cerr << "lengths" << endl;
    for (unsigned len = minLength; len <= maxLength; ++len) {
        cerr << "\t" << len << " = " << defCount[len] << " words" << endl;
    }

    // deallocation
    delete[] defCount;
    delete grid;
    
    return 0;
}
