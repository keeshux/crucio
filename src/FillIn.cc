/*
 * FillIn.cc
 * crucio
 *
 * Copyright 2012 Davide De Rosa
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

#include "FillIn.h"

using namespace crucio;
using namespace std;

char FillIn::Entry::getDirectionChar(const EntryDirection direction)
{
    // TODO: shorten &&
    if ((direction & ENTRY_DIR_ACROSS) && (direction & ENTRY_DIR_DOWN)) {
        return 'C';
    } else if (direction & ENTRY_DIR_ACROSS) {
        return 'A';
    } else if (direction & ENTRY_DIR_DOWN) {
        return 'D';
    } else {
        return '-';
    }
}

const char *FillIn::Entry::getDirectionString(const EntryDirection direction)
{
    switch (direction) {
        case ENTRY_DIR_ACROSS:
            return "across";
            
        case ENTRY_DIR_DOWN:
            return "down";
            
        default:
            assert(false);
            return "";
    }
}

char FillIn::Entry::getDirectionChar() const
{
    return getDirectionChar(m_direction);
}

const char *FillIn::Entry::getDirectionString() const
{
    return getDirectionString(m_direction);
}

FillIn::FillIn(const GridStructure &structure, unsigned (*randomizer)()) :
        m_structure(structure),
        m_randomizer(randomizer)
{
    unsigned i, j;
    
    m_entries = new Entry*[m_structure.m_rows];
    for (i = 0; i < m_structure.m_rows; ++i) {
        m_entries[i] = new Entry[m_structure.m_columns];
        for (j = 0; j < m_structure.m_columns; ++j) {
            Entry *entry = &m_entries[i][j];

            entry->m_value = ENTRY_VAL_NONE;
            entry->m_direction = ENTRY_DIR_NONE;
        }
    }

    // prepare words distribution
    createDistribution(m_structure.m_minLength, m_structure.m_maxLength, &m_distribution);
//    cerr << "distribution: { ";
//    for (i = 0; i < m_distribution.size(); ++i) {
//        cerr << m_distribution[i] << ", ";
//    }
//    cerr << " }" << endl;
}

FillIn::~FillIn()
{
    unsigned i;
    
    for (i = 0; i < m_structure.m_rows; ++i) {
        delete[] m_entries[i];
    }
    delete[] m_entries;
}

void FillIn::layout()
{
    list<Step> crossable;
    list<Step>::iterator currentStep;
    CellAddress lower, upper;
    Word word;
    CellAddress stepCell;
    unsigned x, di, dj;

    // base step
    crossable.push_back(Step(this, randomCellAddress(), randomEntryDirection()));

    while (!crossable.empty()) {

        // 1) pick step from crossable
        
        cerr << endl;
        cerr << "steps count = " << crossable.size() << endl;

        // pick random step
//        currentStep = crossable.begin();
        currentStep = randomElement(crossable.begin(), crossable.end());
        cerr << "current step: " << *currentStep << endl;

        // related entry
        const Entry &currentEntry = currentStep->getEntry();
        const unsigned maxLength = currentStep->getBoundaries(&lower, &upper);

        // skip step in black cell
        if (currentEntry.m_value == ENTRY_VAL_BLACK) {
            cerr << "\tskipping (black cell)" << endl;
            crossable.erase(currentStep);
            continue;
        }
        
        // skip step under min length
        if (maxLength < m_structure.m_minLength) {
            cerr << "\tskipping (under min length)" << endl;
            crossable.erase(currentStep);
            continue;
        }
        
        // skip step overlapping existing word
        if (currentEntry.m_direction & currentStep->getDirection()) {
            cerr << "\tskipping (overlapping existing word)" << endl;
            crossable.erase(currentStep);
            continue;
        }

        // 2) place word
        
        // get a random word given step direction and boundaries
        currentStep->getRandomWord(&word, &lower, &upper);
        
        // place word into grid
        placeWord(&word);

        // block surrounding cells
        blockSurroundingCells(&word);

        // print grid with new word
        cerr << *this << endl;
        
        // 3) add word cells as new steps (invert direction)
        
        switch (word.m_direction) {
            case ENTRY_DIR_ACROSS: {
                di = 0;
                dj = 1;
                break;
            }
            case ENTRY_DIR_DOWN: {
                di = 1;
                dj = 0;
                break;
            }
            default: {
                assert(false);
                break;
            }
        }
        
        // new steps
        for (x = 0; x < word.m_length; ++x) {
            stepCell.m_row = word.m_origin.m_row + x * di;
            stepCell.m_column = word.m_origin.m_column + x * dj;
            
            const Step step(this, stepCell, Entry::getOppositeDirection(word.m_direction));
            cerr << "\tadding step: " << step << endl;

            crossable.push_back(step);
        }
        //unique(crossable.begin(), crossable.end());
        
        // 4) remove analyzed step

        crossable.erase(currentStep);
    }

    // 5) commit remaining empty cells

    finishFilling();
}

Grid *FillIn::createGrid() const
{
    unsigned i, j;

    char **chars = new char*[m_structure.m_rows];
    for (i = 0; i < m_structure.m_rows; ++i) {
        chars[i] = new char[m_structure.m_columns];
        for (j = 0; j < m_structure.m_columns; ++j) {
            chars[i][j] = (char)m_entries[i][j].m_value;
        }
    }

    Grid *grid = new Grid((const char **)chars, m_structure.m_rows, m_structure.m_columns);

    for (i = 0; i < m_structure.m_rows; ++i) {
        delete[] chars[i];
    }
    delete[] chars;

    return grid;
}

#pragma mark - Subproblems

FillIn::Step::Step(const FillIn *fillIn, const CellAddress &cell, const EntryDirection direction) :
        m_fillIn(fillIn),
        m_cell(cell),
        m_direction(direction)
{
}

unsigned FillIn::Step::getBoundaries(CellAddress *lower, CellAddress *upper) const
{
    unsigned x1, x2;
    
    switch (m_direction) {
        case ENTRY_DIR_ACROSS: {
            const bool has_top = (m_cell.m_row > 0);
            const bool has_bottom = (m_cell.m_row < m_fillIn->m_structure.m_rows - 1);
            bool tw, bw, ntw, nbw;
            
            x1 = x2 = m_cell.m_column;
            
            tw = bw = ntw = nbw = false;
            while (x1 > 0) {
                if (m_fillIn->getEntryAt(m_cell.m_row, x1 - 1).m_value == ENTRY_VAL_BLACK) {
                    break;
                }
                
                // top adjacency
                if (has_top) {
                    ntw = (m_fillIn->getEntryAt(m_cell.m_row - 1, x1 - 1).m_value == ENTRY_VAL_WHITE);
                    if (tw && ntw) {
                        break;
                    }
                    tw = ntw;
                }
                
                // bottom adjacency
                if (has_bottom) {
                    nbw = (m_fillIn->getEntryAt(m_cell.m_row + 1, x1 - 1).m_value == ENTRY_VAL_WHITE);
                    if (bw && nbw) {
                        break;
                    }
                    bw = nbw;
                }
                
                --x1;
            }
            
            tw = bw = ntw = nbw = false;
            while (x2 < m_fillIn->m_structure.m_columns - 1) {
                if (m_fillIn->getEntryAt(m_cell.m_row, x2 + 1).m_value == ENTRY_VAL_BLACK) {
                    break;
                }
                
                // top adjacency
                if (has_top) {
                    ntw = (m_fillIn->getEntryAt(m_cell.m_row - 1, x2 + 1).m_value == ENTRY_VAL_WHITE);
                    if (tw && ntw) {
                        break;
                    }
                    tw = ntw;
                }
                
                // bottom adjacency
                if (has_bottom) {
                    nbw = (m_fillIn->getEntryAt(m_cell.m_row + 1, x2 + 1).m_value == ENTRY_VAL_WHITE);
                    if (bw && nbw) {
                        break;
                    }
                    bw = nbw;
                }
                
                ++x2;
            }
            
            lower->m_row = m_cell.m_row;
            lower->m_column = x1;
            upper->m_row = m_cell.m_row;
            upper->m_column = x2;
            
            break;
        }
        case ENTRY_DIR_DOWN: {
            const bool has_left = (m_cell.m_column > 0);
            const bool has_right = (m_cell.m_column < m_fillIn->m_structure.m_columns - 1);
            bool lw, rw, nlw, nrw;
            
            x1 = x2 = m_cell.m_row;
            
            lw = rw = nlw = nrw = false;
            while (x1 > 0) {
                if (m_fillIn->getEntryAt(x1 - 1, m_cell.m_column).m_value == ENTRY_VAL_BLACK) {
                    break;
                }
                
                // left adjacency
                if (has_left) {
                    nlw = (m_fillIn->getEntryAt(x1 - 1, m_cell.m_column - 1).m_value == ENTRY_VAL_WHITE);
                    if (lw && nlw) {
                        break;
                    }
                    lw = nlw;
                }
                
                // right adjacency
                if (has_right) {
                    nrw = (m_fillIn->getEntryAt(x1 - 1, m_cell.m_column + 1).m_value == ENTRY_VAL_WHITE);
                    if (rw && nrw) {
                        break;
                    }
                    rw = nrw;
                }
                
                --x1;
            }
            
            lw = rw = nlw = nrw = false;
            while (x2 < m_fillIn->m_structure.m_rows - 1) {
                if (m_fillIn->getEntryAt(x2 + 1, m_cell.m_column).m_value == ENTRY_VAL_BLACK) {
                    break;
                }
                
                // left adjacency
                if (has_left) {
                    nlw = (m_fillIn->getEntryAt(x2 + 1, m_cell.m_column - 1).m_value == ENTRY_VAL_WHITE);
                    if (lw && nlw) {
                        break;
                    }
                    lw = nlw;
                }
                
                // right adjacency
                if (has_right) {
                    nrw = (m_fillIn->getEntryAt(x2 + 1, m_cell.m_column + 1).m_value == ENTRY_VAL_WHITE);
                    if (rw && nrw) {
                        break;
                    }
                    rw = nrw;
                }
                
                ++x2;
            }
            
            lower->m_row = x1;
            lower->m_column = m_cell.m_column;
            upper->m_row = x2;
            upper->m_column = m_cell.m_column;
            
            break;
        }
        default: {
            assert(false);
            break;
        }
    }

    // max length from distance
    const unsigned maxLength = x2 - x1 + 1;
    const unsigned globalMaxLength = m_fillIn->m_structure.m_maxLength;
    unsigned effectiveMaxLength = 0;

    // global max length at most
    effectiveMaxLength = min(maxLength, globalMaxLength);

    cerr << "\tmax word length is " << maxLength << endl;
    cerr << "\tmax effective word length is " << effectiveMaxLength << endl;

    return effectiveMaxLength;
}

void FillIn::Step::getRandomWord(Word *word, CellAddress *lower, CellAddress *upper) const
{
    // prefer word length from distribution
    unsigned preferredLength;
    vector<CellAddress> preferredBegin;

    // randomic choices
    vector<CellAddress> possibleBegin;
    vector<CellAddress> possibleEnd;
    vector<CellAddress>::const_iterator defBegin;
    vector<CellAddress>::const_iterator defEnd;
    unsigned distance = 0;

    // IMPORTANT: definition MUST include m_cell in order to guarantee connection
    //
    // ACROSS
    //
    //      lower->m_row         = m_cell.m_row     = upper->m_row
    //      defBegin->m_column  <= m_cell.m_column  < defEnd->m_column
    //
    // DOWN
    //
    //      defBegin->m_row     <= m_cell.m_row     < defEnd->m_row
    //      lower->m_column      = m_cell.m_column  = upper->m_column
    //
    
    cerr << "\twords can span from " << *lower << " to " << *upper << endl;
    assert(*lower != *upper);
    
    preferredLength = m_fillIn->randomWordLengthFromDistribution();
    cerr << "\tpreferred length is " << preferredLength << endl;
    
    // word direction is step direction
    word->m_direction = m_direction;
    
    // cache limits
    const GridStructure &structure = m_fillIn->m_structure;
    const unsigned minLength = structure.m_minLength;
    const unsigned maxLength = structure.m_maxLength;
    
    // step cell is lower bound, start here
    if (m_cell == *lower) {
        possibleBegin.push_back(m_cell);
    }
    // multiple
    else {
        CellAddress begin = *lower;
        CellAddress end = m_cell;
        unsigned possibleLength = 0;
        unsigned position = 0;
        
        switch (word->m_direction) {
            case ENTRY_DIR_ACROSS: {

                // admitted word lengths
                for (possibleLength = maxLength; possibleLength >= minLength; --possibleLength) {
                    
                    // begin in [lower, step]
                    for (position = lower->m_column; position <= m_cell.m_column; ++position) {
                        begin.m_column = position;

                        // not viable
                        if (!isAcrossBegin(begin)) {
                            continue;
                        }

                        // start if end in [step, upper]
                        end.m_column = position + possibleLength - 1;

                        if (isAcrossEnd(end) &&
                            (end.m_column >= m_cell.m_column) &&
                            (end.m_column <= upper->m_column)) {

//                            cerr << "\t\tmay begin at " << begin << " (" << possibleLength << ")" << endl;

                            // unique (TODO: set?)
                            if (find(possibleBegin.begin(), possibleBegin.end(), begin) == possibleBegin.end()) {
                                possibleBegin.push_back(begin);
                            }

                            // separate preferred begin cells
                            if (possibleLength == preferredLength) {
//                                cerr << "\t\tpreferred may begin at " << begin << endl;
                                preferredBegin.push_back(begin);
                            }
                        }
                    }
                }
            
                break;
            }
            case ENTRY_DIR_DOWN: {
                
                // admitted word lengths
                for (possibleLength = maxLength; possibleLength >= minLength; --possibleLength) {
                    
                    // begin in [lower, step]
                    for (position = lower->m_row; position <= m_cell.m_row; ++position) {
                        begin.m_row = position;
                        
                        // not viable
                        if (!isDownBegin(begin)) {
                            continue;
                        }

                        // start if end in [step, upper]
                        end.m_row = position + possibleLength - 1;
                        
                        if (isDownEnd(end) &&
                            (end.m_row >= m_cell.m_row) &&
                            (end.m_row <= upper->m_row)) {
                            
//                            cerr << "\t\tmay begin at " << begin << " (" << possibleLength << ")" << endl;

                            // unique (TODO: set?)
                            if (find(possibleBegin.begin(), possibleBegin.end(), begin) == possibleBegin.end()) {
                                possibleBegin.push_back(begin);
                            }
                            
                            // separate preferred begin cells
                            if (possibleLength == preferredLength) {
//                                cerr << "\t\tpreferred may begin at " << begin << endl;
                                preferredBegin.push_back(begin);
                            }
                        }
                    }
                }

                break;
            }
            default: {
                assert(false);
                break;
            }
        }
    }
    
    // preferred length fits, stop here
    if (!preferredBegin.empty()) {
        defBegin = m_fillIn->randomElement(preferredBegin.begin(), preferredBegin.end());
        distance = preferredLength - 1;

        cerr << "\tapplying preferred word length " << preferredLength << endl;
        cerr << "\tchosen begin: " << *defBegin << endl;
    }
    // otherwise choose begin and find random end in possible
    else {
        cerr << "\tcomputing random word length " << preferredLength << endl;

        defBegin = m_fillIn->randomElement(possibleBegin.begin(), possibleBegin.end());
        cerr << "\tchosen begin: " << *defBegin << endl;
        
        // step cell is upper bound, end here
        if (m_cell == *upper) {
            possibleEnd.push_back(m_cell);
        }
        // multiple
        else {
            CellAddress end = m_cell;
            CellAddress min_end = m_cell;
            CellAddress max_end = m_cell;
            
            switch (word->m_direction) {
                case ENTRY_DIR_ACROSS: {
                    
                    min_end.m_column = max(defBegin->m_column, m_cell.m_column);
                    if (min_end.m_column == defBegin->m_column) {
                        ++min_end.m_column;
                    }
                    max_end.m_column = min(defBegin->m_column + maxLength - 1, upper->m_column);
                    
                    cerr << "\t\ttrying end range: " << min_end << " to " << max_end << endl;
                    
                    for (end.m_column = min_end.m_column; end.m_column <= max_end.m_column; ++end.m_column) {
                        if (isAcrossEnd(end)) {
                            cerr << "\t\tmay end at " << end << endl;

                            // unique (TODO: set?)
                            if (find(possibleEnd.begin(), possibleEnd.end(), end) == possibleEnd.end()) {
                                possibleEnd.push_back(end);
                            }
                        }
                    }
                        
                    break;
                }
                case ENTRY_DIR_DOWN: {
                    
                    min_end.m_row = max(defBegin->m_row, m_cell.m_row);
                    if (min_end.m_row == defBegin->m_row) {
                        ++min_end.m_row;
                    }
                    max_end.m_row = min(defBegin->m_row + maxLength - 1, upper->m_row);
                    
                    cerr << "\t\ttrying end range: " << min_end << " to " << max_end << endl;
                    
                    for (end.m_row = min_end.m_row; end.m_row <= max_end.m_row; ++end.m_row) {
                        if (isDownEnd(end)) {
                            cerr << "\t\tmay end at " << end << endl;
                            
                            // unique (TODO: set?)
                            if (find(possibleEnd.begin(), possibleEnd.end(), end) == possibleEnd.end()) {
                                possibleEnd.push_back(end);
                            }
                        }
                    }
                    
                    break;
                }
                default: {
                    assert(false);
                    break;
                }
            }
        }
        
        // choose one randomly
        defEnd = m_fillIn->randomElement(possibleEnd.begin(), possibleEnd.end());
        cerr << "\tchosen end: " << *defEnd << endl;

        // length by begin/end distance
        switch (word->m_direction) {
            case ENTRY_DIR_ACROSS:
                distance = defEnd->m_column - defBegin->m_column;
                break;
                
            case ENTRY_DIR_DOWN:
                distance = defEnd->m_row - defBegin->m_row;
                break;

            default:
                assert(false);
                break;
        }
    }
    
    // complete word structure
    word->m_origin = *defBegin;
    word->m_length = distance + 1;
    
    // ensure word is long enough
    assert(word->m_length >= minLength);

    cerr << "\tchosen random word: " << *word << endl;
}

void FillIn::placeWord(const Word *word)
{
    const CellAddress *origin = &word->m_origin;
    CellAddress whiteCell, blackCell;
    unsigned x, di, dj;
    
    cerr << "placing word: " << *word << endl;
    
    switch (word->m_direction) {
        case ENTRY_DIR_ACROSS: {
            di = 0;
            dj = 1;
            break;
        }
        case ENTRY_DIR_DOWN: {
            di = 1;
            dj = 0;
            break;
        }
        default: {
            assert(false);
            break;
        }
    }
    
    for (x = 0; x < word->m_length; ++x) {
        whiteCell.m_row = origin->m_row + x * di;
        whiteCell.m_column = origin->m_column + x * dj;
        
        cerr << "\tputting white cell in " << whiteCell << endl;

        Entry &whiteEntry = getEntryAt(whiteCell);
        whiteEntry.m_value = ENTRY_VAL_WHITE;
        whiteEntry.m_direction = (EntryDirection)((unsigned)whiteEntry.m_direction | (unsigned)word->m_direction);
    }
    
    // black cells
    switch (word->m_direction) {
        case ENTRY_DIR_ACROSS: {

            if (origin->m_column > 0) {
                blackCell.m_row = origin->m_row;
                blackCell.m_column = origin->m_column - 1;

                cerr << "\tputting black cell in " << blackCell << endl;

                Entry &blackEntry = getEntryAt(blackCell);
                assert(blackEntry.m_value != ENTRY_VAL_WHITE);
                
                blackEntry.m_value = ENTRY_VAL_BLACK;
                blackEntry.m_direction = ENTRY_DIR_NONE;
            }

            if (origin->m_column + word->m_length < m_structure.m_columns) {
                blackCell.m_row = origin->m_row;
                blackCell.m_column = origin->m_column + word->m_length;

                cerr << "\tputting black cell in " << blackCell << endl;

                Entry &blackEntry = getEntryAt(blackCell);
                assert(blackEntry.m_value != ENTRY_VAL_WHITE);
                
                blackEntry.m_value = ENTRY_VAL_BLACK;
                blackEntry.m_direction = ENTRY_DIR_NONE;
            }
            
            break;
        }
        case ENTRY_DIR_DOWN: {

            if (origin->m_row > 0) {
                blackCell.m_row = origin->m_row - 1;
                blackCell.m_column = origin->m_column;

                cerr << "\tputting black cell in " << blackCell << endl;

                Entry &blackEntry = getEntryAt(blackCell);
                assert(blackEntry.m_value != ENTRY_VAL_WHITE);
                
                blackEntry.m_value = ENTRY_VAL_BLACK;
                blackEntry.m_direction = ENTRY_DIR_NONE;
            }

            if (origin->m_row + word->m_length < m_structure.m_rows) {
                blackCell.m_row = origin->m_row + word->m_length;
                blackCell.m_column = origin->m_column;

                cerr << "\tputting black cell in " << blackCell << endl;

                Entry &blackEntry = getEntryAt(blackCell);
                assert(blackEntry.m_value != ENTRY_VAL_WHITE);
                
                blackEntry.m_value = ENTRY_VAL_BLACK;
                blackEntry.m_direction = ENTRY_DIR_NONE;
            }
            
            break;
        }
        default: {
            assert(false);
            break;
        }
    }
}

void FillIn::blockSurroundingCells(const Word *word)
{
    const CellAddress *origin = &word->m_origin;
    CellAddress from;
    unsigned top_row, bottom_row, left_column, right_column;
    
    cerr << "block surrounding cells" << endl;
    
    top_row = bottom_row = left_column = right_column = UINT_MAX;
    
    switch (word->m_direction) {
        case ENTRY_DIR_ACROSS: {
            
            // top row
            if (origin->m_row > 0) {
                top_row = origin->m_row - 1;
            }
            
            // bottom row
            if (origin->m_row < m_structure.m_rows - 1) {
                bottom_row = origin->m_row + 1;
            }
            
            // left column
            if (origin->m_column > 0) {
                left_column = origin->m_column - 1;
            }
            
            // right column
            if (origin->m_column + word->m_length < m_structure.m_columns) {
                right_column = origin->m_column + word->m_length;
            }
            
            break;
        }
        case ENTRY_DIR_DOWN: {
            
            // top row
            if (origin->m_row > 0) {
                top_row = origin->m_row - 1;
            }
            
            // bottom row
            if (origin->m_row + word->m_length < m_structure.m_rows) {
                bottom_row = origin->m_row + word->m_length;
            }
            
            // left column
            if (origin->m_column > 0) {
                left_column = origin->m_column - 1;
            }
            
            // right column
            if (origin->m_column < m_structure.m_columns - 1) {
                right_column = origin->m_column + 1;
            }
            
            break;
        }
        default: {
            assert(false);
            break;
        }
    }
    
    if (top_row != UINT_MAX) {
        cerr << "\ttop row is " << top_row << endl;

        from.m_row = top_row;
        from.m_column = origin->m_column;

        blockRow(&from, word->m_direction, word->m_length);
    }
    
    if (bottom_row != UINT_MAX) {
        cerr << "\tbottom row is " << bottom_row << endl;

        from.m_row = bottom_row;
        from.m_column = origin->m_column;

        blockRow(&from, word->m_direction, word->m_length);
    }
    
    if (left_column != UINT_MAX) {
        cerr << "\tleft column is " << left_column << endl;

        from.m_row = origin->m_row;
        from.m_column = left_column;

        blockColumn(&from, word->m_direction, word->m_length);
    }
    
    if (right_column != UINT_MAX) {
        cerr << "\tright column is " << right_column << endl;

        from.m_row = origin->m_row;
        from.m_column = right_column;

        blockColumn(&from, word->m_direction, word->m_length);
    }
}

void FillIn::blockRow(const CellAddress *from, const EntryDirection direction, unsigned length)
{
    CellAddress cell = *from;
    unsigned x = 0;
    
    if (direction == ENTRY_DIR_DOWN) {
        length = 1;
        
        if (from->m_column > 0) {
            --cell.m_column;
            ++length;
        }
        if (from->m_column < m_structure.m_columns - 1) {
            ++length;
        }

        cerr << "\t\tshort blocking from " << cell << " for " << length << endl;
    }
    
    for (; x < length; ++x) {
        if (blockCell(&cell)) {
//            blocked->push_back(cell);
        }
        ++cell.m_column;
    }
}

void FillIn::blockColumn(const CellAddress *from, const EntryDirection direction, unsigned length)
{
    CellAddress cell = *from;
    unsigned x = 0;
    
    if (direction == ENTRY_DIR_ACROSS) {
        length = 1;
        
        if (from->m_row > 0) {
            --cell.m_row;
            ++length;
        }
        if (from->m_row < m_structure.m_rows - 1) {
            ++length;
        }

        cerr << "\t\tshort blocking from " << cell << " for " << length << endl;
    }
    
    for (; x < length; ++x) {
        if (blockCell(&cell)) {
//            blocked->push_back(cell);
        }
        ++cell.m_row;
    }
}

bool FillIn::blockCell(const CellAddress *cell)
{
    Entry &entry = getEntryAt(*cell);
    
//    cerr << "\t\tchecking cell " << *cell << endl;
    
    // skip black cells
    if (entry.m_value != ENTRY_VAL_NONE) {
        cerr << "\t\tskipping non-empty cell " << *cell << endl;
        return false;
    }
    
    // block dense cells
    if (isDenseCrossing(cell)) {
        entry.m_value = ENTRY_VAL_BLACK;
        cerr << "\t\tblocked dense cell " << *cell << endl;
        return true;
    }
    
    return false;
}

// dense if new two words in opposite directions
bool FillIn::isDenseCrossing(const CellAddress *cell)
{
    unsigned adjacencies[4] = { ENTRY_DIR_NONE };
    unsigned acrossCount = 0, downCount = 0;
    unsigned x = 0;
    
    // top
    if (cell->m_row > 0) {
        adjacencies[0] = getEntryAt(cell->m_row - 1, cell->m_column).m_direction;
    }
    // bottom
    if (cell->m_row < m_structure.m_rows - 1) {
        adjacencies[1] = getEntryAt(cell->m_row + 1, cell->m_column).m_direction;
    }
    // left
    if (cell->m_column > 0) {
        adjacencies[2] = getEntryAt(cell->m_row, cell->m_column - 1).m_direction;
    }
    // right
    if (cell->m_column < m_structure.m_columns - 1) {
        adjacencies[3] = getEntryAt(cell->m_row, cell->m_column + 1).m_direction;
    }
    
    for (x = 0; x < 4; ++x) {
        if (adjacencies[x] & ENTRY_DIR_ACROSS) {
            ++acrossCount;
        }
        if (adjacencies[x] & ENTRY_DIR_DOWN) {
            ++downCount;
        }
    }
    return (acrossCount > 0) && (downCount > 0);
}

void FillIn::finishFilling()
{
    unsigned i, j;
    
    for (i = 0; i < m_structure.m_rows; ++i) {
        for (j = 0; j < m_structure.m_columns; ++j) {
            EntryValue *value = &m_entries[i][j].m_value;

            // replace empty with black
            if (*value == ENTRY_VAL_NONE) {
                *value = ENTRY_VAL_BLACK;
            }
        }
    }
}

#pragma mark - Utilities

CellAddress FillIn::randomCellAddress() const
{
    CellAddress cell;
    cell.m_row = randomNumber(0, m_structure.m_rows - 1);
    cell.m_column = randomNumber(0, m_structure.m_columns - 1);
    return cell;
}

#pragma mark - Words distribution

void FillIn::createDistribution(const unsigned min, const unsigned max, std::vector<unsigned> *distribution)
{
    // hardcoded parameters
    static unsigned knotsVals[3] = { 5, 10, 1 };

    // support variables
    unsigned buffer[200] = { 0 };
    unsigned knots[3] = { 0 };
    unsigned *p;
    unsigned i, j, count;
    float mu;
    
    // interpolation
    const unsigned length = max - min;
    knots[0] = 0;
    knots[1] = length / 2.5;
    knots[2] = length;
    
    //count = (range.length - i) / 2 + 1;
    
    // iteration
    p = buffer;
    
    // high frequency - cosine
    for (i = knots[0]; i < knots[1]; ++i) {
        mu = (float)(i - knots[0]) / (knots[1] - knots[0]);
        count = interpolateCosine(knotsVals[0], knotsVals[1], mu);
        
        // copy word length for count times
        for (j = 0; j < count; ++j) {
            *p = min + i;
            ++p;
        }
        //DDLog(@"frequency of %d = %d", range.location + i, count);
    }
    
    // low frequency - power (XXX: exclude longest, 1 might be added
    // in random grid)
    for (i = knots[1]; i < knots[2]; ++i) {
        mu = (float)(i - knots[1]) / (knots[2] - knots[1]);
        count = interpolateLinear(knotsVals[2], knotsVals[1], pow(1 - mu, 2));
        
        // copy word length for count times
        for (j = 0; j < count; ++j) {
            *p = min + i;
            ++p;
        }
        
        //DDLog(@"mu of %d = %f", i, mu);
        //DDLog(@"frequency of %d = %d", range.location + i, count);
    }
    
    // reserve adequate size
    const unsigned size = p - buffer;
    distribution->resize(size);
    //DDLog(@"distribution of length %d", *length);

    // copy local buffer
    copy(buffer, buffer + size, distribution->begin());
}

unsigned FillIn::randomWordLengthFromDistribution() const
{
    return m_distribution[m_randomizer() % m_distribution.size()];
}

#pragma mark - Output

ostream &operator<<(ostream &out, const FillIn &fillIn)
{
    const GridStructure &structure = fillIn.getStructure();
    unsigned i, j;
    
    out << endl;
    for (i = 0; i < structure.m_rows; ++i) {
        for (j = 0; j < structure.m_columns; ++j) {
            out << (char)fillIn.getEntryAt(i, j).m_value;
        }
        out << endl;
    }

    out << endl;
    for (i = 0; i < structure.m_rows; ++i) {
        for (j = 0; j < structure.m_columns; ++j) {
            out << fillIn.getEntryAt(i, j).getDirectionChar();
        }
        out << endl;
    }

    return out;
}

ostream &operator<<(ostream &out, const FillIn::Word &word)
{
    out << "word from " << word.m_origin;
    out << " (" << FillIn::Entry::getDirectionString(word.m_direction) << ")";
    out << " of length " << word.m_length;
    
    return out;
}

ostream &operator<<(ostream &out, const FillIn::Step &step)
{
    out << "step at " << step.getCell();
    out << " (" << FillIn::Entry::getDirectionString(step.getDirection()) << ")";
    
    return out;
}
