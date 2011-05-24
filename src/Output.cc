#include "Output.h"

using namespace crucio;
using namespace std;

/* <namespace> */

void crucio::printInputDescription(ostream& out, const Dictionary& d,
        const Grid& g, const string& fillType, const string& walkType,
        const bool unique, const bool deterministic,
        const uint32_t seed, const bool verbose) {

    // grid indexes
    uint32_t i, j;

    // random seed
    out << "random seed = " << seed << endl;

    // input files
    out << "dictionary = '" << d.getFilename() << "', " <<
            d.getSize() << " valid words" << endl;
    out << "grid = '"  << g.getFilename() << "', " <<
            g.getRows() << "x" << g.getColumns() << ", " <<
            "interlock degree " << g.getInterlockDegree() << "%" << endl;

    out << endl;

    // unfilled grid output
    for (i = 0; i < g.getRows(); ++i) {
        for (j = 0; j < g.getColumns(); ++j) {
            const Cell* const cl = g.getCell(i, j);
            out << cl->getValue();
        }
        out << endl;
    }

    out << endl;

    // parameters
    out << "filling strategy = " << fillType << endl;
    out << "walk heuristic = " << walkType << endl;
    out << "unique words = " << (unique ? "yes" : "no") << endl;
    out << "deterministic filling = " << (deterministic ? "yes" : "no") <<
            endl;
    out << "verbose = " << (verbose ? "yes" : "no") << endl;

    out << endl;
}

void crucio::printModelDescription(ostream& out, const Model& m) {

    // model letters/words shortcuts
    const uint32_t lettersNum = m.getLettersNum();
    const uint32_t wordsNum = m.getWordsNum();
    uint32_t li, wi;

    out << "letters = " << lettersNum << endl;
    out << "words = " << wordsNum << endl;
    out << endl;

    // letters
    for (li = 0; li < lettersNum; ++li) {

        // current letter and cell reference
        const Letter* const l = m.getLetter(li);
        const Cell* const lCl = l->getCell();

        // position in the grid
        out << "l" << li << " = (" << lCl->getRow() << "," <<
                lCl->getColumn() << ")" << endl;

        // words it belongs to
        const list<LetterPosition>& letterWords = m.getLetterWords(li);
        out << "\tbelongs to: ";
        list<LetterPosition>::const_iterator lpIt;
        for (lpIt = letterWords.begin(); lpIt != letterWords.end(); ++lpIt) {
            const uint32_t wi = lpIt->getWordIndex();
            const uint32_t pos = lpIt->getPosition();
            out << "w" << wi << "[" << pos << "] ";
        }
        out << endl;

        out << endl;
    }

    // words
    for (wi = 0; wi < wordsNum; ++wi) {

        // current word definition reference
        const Definition* const def = m.getWord(wi)->getDefinition();

        // position in the grid
        out << "w" << wi << " = " << def->getNumber() << " ";
        out << ((def->getDirection() == Definition::ACROSS) ?
                "ACROSS" : "DOWN");
        out << endl;

        // length and starting coordinates
        const Cell* const defCl = def->getStartCell();
        out << "\tlength: " << def->getLength() << endl;
        out << "\tstarts at: " << "(" << defCl->getRow() << "," <<
                defCl->getColumn() << ")" << endl;

        // letters
        const vector<uint32_t>& wordLetters = m.getWordLetters(wi);
        out << "\tcontains: ";
        vector<uint32_t>::const_iterator lIt;
        for (lIt = wordLetters.begin(); lIt != wordLetters.end(); ++lIt) {
            const int li = *lIt;
            out << "l" << li;
            out << " ";
        }
        out << endl;

        // crossings (vector is never empty if no isolated cells in the grid)
        const list<pair<uint32_t, WordCrossing> >& neighbours =
                m.getWordNeighbours(wi);
        out << "\tcrossings: ";
        list<pair<uint32_t, WordCrossing> >::const_iterator nbIt;
        for (nbIt = neighbours.begin(); nbIt != neighbours.end(); ++nbIt) {
            const uint32_t cwi = nbIt->first;
            const uint32_t pos = nbIt->second.getPosition();
            const uint32_t cwPos = nbIt->second.getCPosition();
            out << "w" << wi << "[" << pos << "]" << "=" <<
                    "w" << cwi << "[" << cwPos << "]";
            out << " ";
        }
        out << endl;

        out << endl;
    }
}

void crucio::printModelGrid(ostream& out, const Model& m) {
    const Grid* const g = m.getGrid();
    const vector<Word*>& words = m.getWords();

    // chars matrix for output
    vector<vector<char> > gChars(g->getRows(), vector<char>(g->getColumns()));

    // grid and letters traversing
    uint32_t i, j;
    uint32_t wi;

    // starting grid
    for (i = 0; i < g->getRows(); ++i) {
        for (j = 0; j < g->getColumns(); ++j) {
            const Cell* const cl = g->getCell(i, j);
            gChars[i][j] = cl->getValue();
        }
    }

    // puts words into the grid
    for (wi = 0; wi < words.size(); ++wi) {
        const Word* const w = words[wi];
        const Definition* const def = w->getDefinition();
        const Cell* const defCl = def->getStartCell();

        // starting location
        uint32_t si = defCl->getRow();
        uint32_t sj = defCl->getColumn();

        // fills by character
        const string& value = w->get();
        string::const_iterator vIt;
        for (vIt = value.begin(); vIt != value.end(); ++vIt) {
            gChars[si][sj] = *vIt;

            // chooses filling direction
            if (def->getDirection() == Definition::ACROSS) {
                ++sj;
            } else {
                ++si;
            }
        }
    }

    // filled grid output
    for (i = 0; i < g->getRows(); ++i) {
        for (j = 0; j < g->getColumns(); ++j) {
            out << gChars[i][j];
        }
        out << endl;
    }
}

void crucio::printOutput(ostream& out, const Model& m) {
    const vector<Word*>& words = m.getWords();

    // words traversing
    uint32_t wi;

    // prints filled grid
    printModelGrid(out, m);

    out << endl;

    // across words by number
    for (wi = 0; wi < words.size(); ++wi) {
        const Word* const w = words[wi];
        const Definition* const def = w->getDefinition();
        const Cell* const defCl = def->getStartCell();

        if (def->getDirection() == Definition::ACROSS) {
            out << def->getNumber() << " ACROSS (" <<
                    defCl->getRow() << "," << defCl->getColumn() << ") = ";
            out << "'" << w->get() << "'" << endl;
        }
    }

    // down words by number
    for (wi = 0; wi < words.size(); ++wi) {
        const Word* const w = words[wi];
        const Definition* const def = w->getDefinition();
        const Cell* const defCl = def->getStartCell();

        if (def->getDirection() == Definition::DOWN) {
            out << def->getNumber() << " DOWN (" <<
                    defCl->getRow() << "," << defCl->getColumn() << ") = ";
            out << "'" << w->get() << "'" << endl;
        }
    }

    out << endl;

    // words by length
    //map<uint32_t, set<string> > wordsMap;
    map<uint32_t, multiset<string> > wordsMap;

    // fills map
    for (wi = 0; wi < words.size(); ++wi) {
        const Word* const w = words[wi];
        const uint32_t len = w->getLength();
        //const pair<map<uint32_t, set<string> >::iterator, bool> res =
        //      wordsMap.insert(make_pair(len, set<string>()));
        const pair<map<uint32_t, multiset<string> >::iterator, bool> res =
                wordsMap.insert(make_pair(len, multiset<string>()));
        res.first->second.insert(w->get());
    }

    // prints words ordered by length
    //map<uint32_t, set<string> >::const_iterator wmIt;
    map<uint32_t, multiset<string> >::const_iterator wmIt;
    for (wmIt = wordsMap.begin(); wmIt != wordsMap.end(); ++wmIt) {
        out << wmIt->first << " LETTERS" << endl;
        out << "------------------" << endl;

        //const set<string>& subset = wmIt->second;
        //set<string>::const_iterator wIt;
        const multiset<string>& subset = wmIt->second;
        multiset<string>::const_iterator wIt;
        for (wIt = subset.begin(); wIt != subset.end(); ++wIt) {
            out << *wIt << endl;
        }

        out << endl;
    }
}

/* Output */

Output::Output(const Model& m) :
        m_rows(0),
        m_columns(0),
        m_cellsData(),
        m_defsData() {
    uint32_t i, j;
    uint32_t wi;

    // grid data
    const Grid* const g = m.getGrid();
    m_rows = g->getRows();
    m_columns = g->getColumns();

    // resizes vectors accordingly
    m_cellsData.resize(m_rows);
    for (i = 0; i < m_rows; ++i) {
        m_cellsData[i].resize(m_columns);
    }

    // loads grid original and final content
    for (i = 0; i < m_rows; ++i) {
        for (j = 0; j < m_columns; ++j) {
            const Cell* const cl = g->getCell(i, j);
            const char ch = cl->getValue();

            // unfilled cell
            m_cellsData[i][j].m_unfilled = ch;

            // mapped letter if any, default unfilled
            const int li = m.getLetterIndexByPos(i, j);
            if (li != -1) {
                const Letter* const l = m.getLetter(li);
                m_cellsData[i][j].m_filled = l->get();
            } else {
                m_cellsData[i][j].m_filled = ch;
            }
        }
    }

    // loads final words
    for (wi = 0; wi < m.getWordsNum(); ++wi) {
        const Word* const w = m.getWord(wi);

        // data creation
        const Definition* const def = w->getDefinition();
        const Cell* const defCl = def->getStartCell();
        i = defCl->getRow();
        j = defCl->getColumn();
        const DefinitionData dData(def->getDirection(), def->getNumber(),
                i, j, w->get());

        // insertion
        m_defsData.insert(dData);

        // puts cell number
        m_cellsData[i][j].m_number = dData.getNumber();
    }

/*  set<DefinitionData>::const_iterator d;
    for (d = m_defsData.begin(); d != m_defsData.end(); ++d) {
        cout << d->getNumber() << " " << (d->getDirection() == Definition::ACROSS ? "ORIZZONTALE" : "VERTICALE") <<
                " = \'" << d->getString() << "\' starts at (" << d->getStartRow() << "," << d->getStartColumn() << ")" <<
                endl;
    }*/
}

Output::Output(istream& in) :
        m_rows(0),
        m_columns(0),
        m_cellsData(),
        m_defsData() {

    // 32-bit integer buffer (big endian)
    char numBuf[4];

    // grid and words indexes
    uint32_t i, j;
    uint32_t wi;

    // grid size and content
    in.read(numBuf, 4);
    m_rows = bytesToLong(numBuf);
    in.read(numBuf, 4);
    m_columns = bytesToLong(numBuf);
    const unsigned long gridSize = m_rows * m_columns;
    char* const gridBuf = new char[2 * gridSize];
    in.read(gridBuf, 2 * gridSize);

    // loads grid original and final content
    const char* gridPtr = gridBuf;
    m_cellsData.resize(m_rows);
    for (i = 0; i < m_rows; ++i) {
        m_cellsData[i].resize(m_columns);

        for (j = 0; j < m_columns; ++j) {
            const char ch = *gridPtr;
            const char filledCh = gridPtr[gridSize];

            // stores grid starting and final elements
            m_cellsData[i][j].m_unfilled = ch;
            m_cellsData[i][j].m_filled = filledCh;

            // next cell
            ++gridPtr;
        }
    }

    // loads final words
    char wordBuf[Dictionary::MAX_LENGTH + 1];
    in.read(numBuf, 4);
    const unsigned long wordsNum = bytesToLong(numBuf);
    for (wi = 0; wi < wordsNum; ++wi) {
        in.read(numBuf, 4);
        const unsigned short dir = bytesToShort(numBuf);
        const unsigned short number = bytesToShort(numBuf + 2);
        in.read(numBuf, 4);
        i = bytesToLong(numBuf);
        in.read(numBuf, 4);
        j = bytesToLong(numBuf);
        in.read(numBuf, 4);
        const unsigned long length = bytesToLong(numBuf);
        in.read(wordBuf, length);
        wordBuf[length] = '\0';

        // data creation
        const DefinitionData dData((Definition::Direction) dir, (uint32_t) number,
                i, j, wordBuf);

        // insertion
        m_defsData.insert(dData);

        // puts cell number
        m_cellsData[i][j].m_number = dData.getNumber();
    }

    // deallocates temporary buffers
    delete gridBuf;

/*  set<DefinitionData>::const_iterator d;
    for (d = m_defsData.begin(); d != m_defsData.end(); ++d) {
        cout << d->getNumber() << " " << (d->getDirection() == Definition::ACROSS ? "ORIZZONTALE" : "VERTICALE") <<
                " = \'" << d->getString() << "\' starts at (" << d->getStartRow() << "," << d->getStartColumn() << ")" <<
                endl;
    }*/
}

void Output::printRaw(ostream& out) const {

    // 32-bit integer buffer (big endian)
    char numBuf[4];

    // grid size
    const unsigned long gridSize = (unsigned long)(m_rows * m_columns);

    // buffer to store unfilled and filled grid content
    char* const gridBuf = new char[2 * gridSize];
    char* gridPtr = gridBuf;

    // grid indexes
    uint32_t i, j;

    // stores starting content and filled content at gridSize offset
    for (i = 0; i < m_rows; ++i) {
        for (j = 0; j < m_columns; ++j) {
            *gridPtr = m_cellsData[i][j].m_unfilled;
            gridPtr[gridSize] = m_cellsData[i][j].m_filled;
            ++gridPtr;
        }
    }

    // writes rows/columns count
    longToBytes((unsigned long) m_rows, numBuf);
    out.write(numBuf, 4);
    longToBytes((unsigned long) m_columns, numBuf);
    out.write(numBuf, 4);

    // writes content buffer
    out.write(gridBuf, 2 * gridSize);

    // writes words count
    longToBytes((unsigned long) m_defsData.size(), numBuf);
    out.write(numBuf, 4);

    // writes words and related info
    set<DefinitionData>::const_iterator dIt;
    for (dIt = m_defsData.begin(); dIt != m_defsData.end(); ++dIt) {
        const DefinitionData& dData = *dIt;

        // 0 for ACROSS, 1 for DOWN (high byte)
        shortToBytes((unsigned short) dData.getDirection(), numBuf);

        // definition number (low byte)
        shortToBytes((unsigned short) dData.getNumber(), numBuf + 2);

        // writes <direction, number> pair
        out.write(numBuf, 4);

        // start position
        longToBytes((unsigned long) dData.getStartRow(), numBuf);
        out.write(numBuf, 4);
        longToBytes((unsigned long) dData.getStartColumn(), numBuf);
        out.write(numBuf, 4);

        // word string preceeded by its length
        const string& str = dData.getString();
        longToBytes((unsigned long) str.length(), numBuf);
        out.write(numBuf, 4);
        out.write(str.c_str(), (unsigned long) str.length());
    }

    // deallocates temporary buffers
    delete gridBuf;
}

void Output::printLatex(ostream& out, const bool solution,
        const bool fillIn) const {
    uint32_t i, j;

    // string buffers
    ostringstream puzzleOut;
    ostringstream cluesOut;

    // puzzle
    puzzleOut << "\t\\begin{Puzzle}{" << m_columns << "}{" <<
            m_rows << "}" << endl;
    for (i = 0; i < m_rows; ++i) {
        puzzleOut << "\t\t";
        for (j = 0; j < m_columns; ++j) {
            const CellData& cData = m_cellsData[i][j];

            // delimiter
            puzzleOut << "|";

            // cell number
            if (!fillIn && (cData.m_number > 0)) {
                puzzleOut << "[" << cData.m_number << "]";
            }

            // black cells conversion
            const char filledCh = cData.m_filled;
            puzzleOut << ((filledCh == Cell::BLACK) ? '*' : filledCh);
        }
        puzzleOut << "|." << endl;
    }
    puzzleOut << "\t\\end{Puzzle}%" << endl;

    // words
    set<DefinitionData>::const_iterator defIt;
    if (fillIn) {

        // split by length
        map<uint32_t, multiset<string> > wordsByLength;
        for (defIt = m_defsData.begin(); defIt != m_defsData.end(); ++defIt) {
            const string& str = defIt->getString();
            const uint32_t len = str.length();

            // inserts word in subset
            if (wordsByLength.find(len) == wordsByLength.end()) {
                wordsByLength.insert(make_pair(len, multiset<string>()));
            }
            multiset<string>& subset = wordsByLength.find(len)->second;
            subset.insert(str);
        }

        // prints by length
        map<uint32_t, multiset<string> >::const_iterator wblIt;
        for (wblIt = wordsByLength.begin(); wblIt !=
                wordsByLength.end(); ++wblIt) {
            const uint32_t len = wblIt->first;
            const multiset<string>& subset = wblIt->second;

            cluesOut << "\t\\begin{PuzzleWords}{" << len << "}%" << endl;
            multiset<string>::const_iterator ssIt;
            for (ssIt = subset.begin(); ssIt != subset.end(); ++ssIt) {
                cluesOut << "\t\t\\Word{" << *ssIt << "}" << endl;
            }
            cluesOut << "\t\\end{PuzzleWords}" << endl;
        }
    } else {
        const string changeMe = "*";

        // split by direction
        const set<DefinitionData>::const_iterator dDefIt =
                find_if(m_defsData.begin(), m_defsData.end(), IsDown());

        // prints across
        cluesOut << "\t\\begin{PuzzleClues}{\\textbf{Orizzontali --}}" << endl;
        for (defIt = m_defsData.begin(); defIt != dDefIt; ++defIt) {
            const DefinitionData& dData = *defIt;
            cluesOut << "\t\t\\Clue{" << dData.getNumber() << "}{" <<
                    dData.getString() << "}{" << changeMe << "}" << endl;
        }
        cluesOut << "\t\\end{PuzzleClues}" << endl;

        // prints down
        cluesOut << "\t\\begin{PuzzleClues}{\\textbf{Verticali --}}" << endl;
        for (; defIt != m_defsData.end(); ++defIt) {
            const DefinitionData& dData = *defIt;
            cluesOut << "\t\t\\Clue{" << dData.getNumber() << "}{" <<
                    dData.getString() << "}{" << changeMe << "}" << endl;
        }
        cluesOut << "\t\\end{PuzzleClues}" << endl;
    }

    // real output

    // document settings
    //if (m_rows < m_columns) {
    //  out << "\\documentclass[landscape]{article}" << endl;
    //} else {
        out << "\\documentclass{article}" << endl;
    //}
    out << "\\pagestyle{empty}" << endl;
    out << "\\pagenumbering{none}" << endl;
    out << "\\linespread{1.6}" << endl;

    // puzzle settings
    out << "\\usepackage[small]{cwpuzzle}" << endl;
    out << "\\renewcommand{\\PuzzleBlackBox}{" \
            "\\rule{.75\\PuzzleUnitlength}{.75\\PuzzleUnitlength}}" << endl;
    out << "\\renewcommand{\\PuzzleWordsText}[1]{\\textbf{#1 lettere} -- }" <<
            endl;

    // begin
    out << "\\begin{document}" << endl;

    // prints unsolved puzzle
    out << "\\begin{figure}[htb!]" << endl;
    out << puzzleOut.str();
    out << cluesOut.str();
    out << "\\end{figure}" << endl;

    // solved puzzle if requested
    if (solution) {

        // starts a new page
        out << "\\newpage" << endl;

        // prints solved puzzle
        out << "\\PuzzleSolution" << endl;
        out << "\\begin{figure}[htb!]" << endl;
        out << puzzleOut.str();
        out << "\\end{figure}" << endl;
    }

    // end
    out << "\\end{document}" << endl;
}
