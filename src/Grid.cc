/*
 * Grid.cc
 * crucio
 *
 * Copyright 2007 Davide De Rosa
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "Grid.h"

using namespace crucio;
using namespace std;

/* Cell */

const char Cell::sm_legalValues[] = "-#ABCDEFGHIJKLMNOPQRSTUVWXYZ";

const Cell* Cell::getNorth() const {
    if (m_row == 0) {
        return 0;
    }
    return m_parent->getCell(m_row - 1, m_column);
}

const Cell* Cell::getWest() const {
    if (m_column == 0) {
        return 0;
    }
    return m_parent->getCell(m_row, m_column - 1);
}

const Cell* Cell::getSouth() const {
    if (m_row == m_parent->getRows() - 1) {
        return 0;
    }
    return m_parent->getCell(m_row + 1, m_column);
}

const Cell* Cell::getEast() const {
    if (m_column == m_parent->getColumns() - 1) {
        return 0;
    }
    return m_parent->getCell(m_row, m_column + 1);
}

void Cell::calculateNearCells() {

    // help variable
    const Cell* cl = 0;

    // adjacent non-black cells
    cl = getEast();
    if (cl && !cl->isBlack()) {
        m_nearCells.push_back(cl);
    }
    cl = getWest();
    if (cl && !cl->isBlack()) {
        m_nearCells.push_back(cl);
    }
    cl = getNorth();
    if (cl && !cl->isBlack()) {
        m_nearCells.push_back(cl);
    }
    cl = getSouth();
    if (cl && !cl->isBlack()) {
        m_nearCells.push_back(cl);
    }
}

/* Definition */

void Definition::calculateCrossingDefinitions() {

    // definition starting cell
    const Cell* cl = m_startCell;

    // help variables
    const Cell* outCl = 0;
    const Cell* crsCl = 0;
    const Definition* def = 0;
    uint32_t pos, crsPos;

    // current position within definition
    pos = 0;

    switch (m_direction) {
    case ACROSS:
        while (cl && !cl->isBlack()) {

            // finds definition and crossing position
            crsPos = 0;
            outCl = cl;
            while (outCl && !outCl->isBlack()) {
                crsCl = outCl;
                outCl = outCl->getNorth();
                ++crsPos;
            }

            // adds new crossing information
            def = crsCl->getDownDef();
            if (def) {
                m_crossingDefinitions.push_back(make_pair(pos,
                        make_pair(def, crsPos)));
            }

            // next definition cell
            cl = cl->getEast();
            ++pos;
        }
        break;

    case DOWN:
        while (cl && !cl->isBlack()) {

            // finds definition and crossing position
            crsPos = 0;
            outCl = cl;
            while (outCl && !outCl->isBlack()) {
                crsCl = outCl;
                outCl = outCl->getWest();
                ++crsPos;
            }

            // adds new crossing information
            def = crsCl->getAcrossDef();
            if (def) {
                m_crossingDefinitions.push_back(make_pair(pos,
                        make_pair(def, crsPos)));
            }

            // next definition cell
            cl = cl->getSouth();
            ++pos;
        }
        break;
    }
}

/* Grid */

Grid::Grid(const char** charsGrid,
           const uint32_t rows, const uint32_t columns) :
        m_filename(""),
        m_rows(0),
        m_columns(0),
        m_whiteCells(0),
        m_blackCells(0),
        m_fixedCells(0),
        m_words(0),
        m_crossings(0),
        m_cells(0),
        m_acrossDefinitions(),
        m_downDefinitions() {

    initGrid(charsGrid, rows, columns);
}

Grid::Grid(const string& filename) :
        m_filename(filename),
        m_rows(0),
        m_columns(0),
        m_whiteCells(0),
        m_blackCells(0),
        m_fixedCells(0),
        m_words(0),
        m_crossings(0),
        m_cells(0),
        m_acrossDefinitions(),
        m_downDefinitions() {

    // file reading
    ifstream gridIn;
    string rowLine, columnLine;
    string line;
    istringstream* dataIn = 0;
    uint32_t rows, columns;
    
    // temporary characters matrix
    char** charsGrid;
    
    // cells indexes
    uint32_t i, j;
    
    // opens grid file
    gridIn.open(filename.c_str());
    if (!gridIn.is_open()) {
        throw GridException("grid: unable to open grid file");
    }
    
    // reads rows and columns count
    if (gridIn.eof()) {
        throw GridException("grid: unexpected EOF");
    }
    getline(gridIn, rowLine);
    if (gridIn.eof()) {
        throw GridException("grid: unexpected EOF");
    }
    getline(gridIn, columnLine);
    
    // converts strings to integers
    dataIn = new istringstream(rowLine);
    *dataIn >> rows;
    delete dataIn;
    dataIn = new istringstream(columnLine);
    *dataIn >> columns;
    delete dataIn;
    
    // size range check
    if ((rows < MIN_SIZE) || (rows > MAX_SIZE) ||
        (columns < MIN_SIZE) || (columns > MAX_SIZE)) {
        throw GridException("grid: size out of [MIN_SIZE, MAX_SIZE]");
    }
    
    // allocates a (rows, columns) chars matrix
    charsGrid = (char**) malloc(rows * sizeof(char*));
    for (i = 0; i < rows; ++i) {
        charsGrid[i] = (char*) malloc(columns * sizeof(char));
        for (j = 0; j < columns; ++j) {
            charsGrid[i][j] = '\0';
        }
    }
    
    // reads by row
    i = 0;
    while (getline(gridIn, line)) {
        if (i == rows) {
            throw GridException("grid: bad rows count");
        }
        if (line.length() != columns) {
            throw GridException("grid: bad row length");
        }
        
        // current row
        j = 0;
        string::const_iterator rowIt;
        for (rowIt = line.begin(); rowIt != line.end(); ++rowIt) {
            const char ch = *rowIt;
            
            // checks format
            if (!Cell::isLegal(ch)) {
                throw GridException("grid: illegal character");
            }
            
            // stores character
            charsGrid[i][j] = ch;
            
            // next column
            ++j;
        }
        
        // next row
        ++i;
    }
    if (i != rows) {
        throw GridException("grid: bad rows count");
    }
    
    // closes file
    gridIn.close();
    
    // complete initialization
    initGrid((const char**) charsGrid, rows, columns);
    
    // deallocate temporary matrix
    for (i = 0; i < rows; ++i) {
        free(charsGrid[i]);
    }
    free(charsGrid);
}

void Grid::initGrid(const char** charsGrid,
                    const uint32_t rows, const uint32_t columns) {

    // save size
    m_rows = rows;
    m_columns = columns;

    // cells indexes and definitions iterator
    uint32_t i, j;
    vector<Definition*>::iterator defIt;

    try {

        // allocates a matrix to contain detailed info on each cell
        m_cells.resize(m_rows);
        for (i = 0; i < m_rows; ++i) {
            m_cells[i].resize(m_columns);
            for (j = 0; j < m_columns; ++j) {
                m_cells[i][j] = 0;
            }
        }

        // creates cells
        unsigned int defNumber = 1;
        for (i = 0; i < m_rows; ++i) {
            for (j = 0; j < m_columns; ++j) {
                const char ch = charsGrid[i][j];

                // checks for this cell to be a crossing
                bool crossing = false;

                // new cell
                Cell* cl = 0;

                // definitions, if existing
                Definition* acrossDef = 0;
                Definition* downDef = 0;

                // total white/black/fixed cells
                if (ch == Cell::WHITE) {
                    ++m_whiteCells;
                } else if (ch == Cell::BLACK) {
                    ++m_blackCells;
                } else {
                    ++m_fixedCells;
                }

                // non-black cell
                if (ch != Cell::BLACK) {

                    // checks for definition start
                    const bool leftWall = ((j == 0) ||
                            (charsGrid[i][j - 1] == Cell::BLACK));
                    const bool rightWall = ((j == m_columns - 1) ||
                            (charsGrid[i][j + 1] == Cell::BLACK));
                    const bool topWall = ((i == 0) ||
                            (charsGrid[i - 1][j] == Cell::BLACK));
                    const bool bottomWall = ((i == m_rows - 1) ||
                            (charsGrid[i + 1][j] == Cell::BLACK));

                    // unreachable cell
                    if (leftWall && rightWall &&
                            topWall && bottomWall) {
                        throw GridException("grid: isolated cells");
                    }

                    // creates across definition
                    if (leftWall && !rightWall) {

                        // increases total words
                        ++m_words;

                        // measures word length
                        uint32_t k = j;
                        while ((k < m_columns) &&
                                (charsGrid[i][k] != Cell::BLACK)) {
                            ++k;
                        }
                        const uint32_t defLength = k - j;

                        // creates definition details
                        acrossDef = new Definition(this,
                                m_acrossDefinitions.size(),
                                Definition::ACROSS, defNumber, defLength);

                        // puts definitions into vector
                        m_acrossDefinitions.push_back(acrossDef);
                    }

                    // creates down definition
                    if (topWall && !bottomWall) {

                        // increases total words
                        ++m_words;

                        // measures word length
                        uint32_t k = i;
                        while ((k < m_rows) &&
                                (charsGrid[k][j] != Cell::BLACK)) {
                            ++k;
                        }
                        const uint32_t defLength = k - i;

                        // creates definition details
                        downDef = new Definition(this,
                                m_downDefinitions.size(),
                                Definition::DOWN, defNumber, defLength);

                        // puts definitions into vector
                        m_downDefinitions.push_back(downDef);
                    }

                    // crossing condition
                    crossing = ((!leftWall || !rightWall) &&
                            (!topWall || !bottomWall));

                    // increases crossings
                    if (crossing) {
                        ++m_crossings;
                    }
                }

                // creates cell and puts it into matrix
                cl = new Cell(this, i, j, ch, crossing);
                m_cells[i][j] = cl;

                // IMPORTANT: adds further links
                cl->setAcrossDef(acrossDef);
                cl->setDownDef(downDef);
                if (acrossDef) {
                    acrossDef->setStartCell(cl);
                }
                if (downDef) {
                    downDef->setStartCell(cl);
                }

                // if current cell contains at least a definition, increases
                // definitions number
                if (acrossDef || downDef) {
                    ++defNumber;
                }
            }
        }

        // no words
        if (m_words == 0) {
            throw GridException("grid: no words");
        }

        // IMPORTANT: computes near cells (cells graph)
        for (i = 0; i < m_rows; ++i) {
            for (j = 0; j < m_columns; ++j) {
                Cell* const cl = m_cells[i][j];
                cl->calculateNearCells();
            }
        }

        // unconnected cell graph (IMPORTANT: AFTER calculateNearCells())
        if (!isConnected()) {
            throw GridException("grid: unreachable cells");
        }

        // IMPORTANT: computes crossing definitions (definitions graph)
        for (defIt = m_acrossDefinitions.begin(); defIt !=
                m_acrossDefinitions.end(); ++defIt) {
            Definition* const def = *defIt;
            def->calculateCrossingDefinitions();
        }
        for (defIt = m_downDefinitions.begin(); defIt !=
                m_downDefinitions.end(); ++defIt) {
            Definition* const def = *defIt;
            def->calculateCrossingDefinitions();
        }
    } catch (GridException&) {

        // on error deallocates cell matrix and definitions vectors
        for (i = 0; i < m_rows; ++i) {
            for (j = 0; j < m_columns; ++j) {
                if (m_cells[i][j]) {
                    delete m_cells[i][j];
                }
            }
        }
        for (defIt = m_acrossDefinitions.begin(); defIt !=
                m_acrossDefinitions.end(); ++defIt) {
            delete *defIt;
        }
        for (defIt = m_downDefinitions.begin(); defIt !=
                m_downDefinitions.end(); ++defIt) {
            delete *defIt;
        }

        // rethrows exception
        throw;
    }
}

Grid::~Grid() {

    // cells
    for (uint32_t i = 0; i < m_rows; ++i) {
        for (uint32_t j = 0; j < m_columns; ++j) {
            delete m_cells[i][j];
        }
    }

    // definitions
    vector<Definition*>::iterator dIt;
    for (dIt = m_acrossDefinitions.begin(); dIt !=
            m_acrossDefinitions.end(); ++dIt) {
        delete *dIt;
    }
    for (dIt = m_downDefinitions.begin(); dIt !=
            m_downDefinitions.end(); ++dIt) {
        delete *dIt;
    }
}

bool Grid::isConnected() const {
    const Cell* const firstCl = getFirstNonBlackCell();
    set<const Cell*, CellCompare> visited;

    // connected if search reaches all white or fixed cells
    return (getReachableFrom(firstCl, &visited) == m_whiteCells + m_fixedCells);
}

uint32_t Grid::getReachableFrom(const Cell* const cl,
        set<const Cell*, CellCompare>* const visited) {

    uint32_t reachable = 0;

    // skips visited cells
    const pair<set<const Cell*,
            CellCompare>::iterator, bool> res = visited->insert(cl);
    if (!res.second) {
        return 0;
    }

    // adds current
    ++reachable;

    // goes down recursively
    const list<const Cell*>& nearCells = cl->getNearCells();
    list<const Cell*>::const_iterator nClIt;
    for (nClIt = nearCells.begin(); nClIt != nearCells.end(); ++nClIt) {
        const Cell* const nCl = *nClIt;
        reachable += getReachableFrom(nCl, visited);
    }

    return reachable;
}

/* <global> */

ostream& operator<<(ostream& out, const Grid& g) {
    for (uint32_t i = 0; i < g.getRows(); ++i) {
        for (uint32_t j = 0; j < g.getColumns(); ++j) {
            const Cell* const cl = g.getCell(i, j);
            out << cl->getValue();
        }
        out << endl;
    }

    return out;
}
