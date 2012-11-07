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
    // TODO: shorten and
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

FillIn::FillIn(const GridStructure &structure) : m_structure(structure)
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
    Step baseStep;
    CellAddress lower, upper;

    // base step
    baseStep.m_fillIn = this;
    baseStep.m_cell = randomCellAddress();
    baseStep.m_direction = randomEntryDirection();
    crossable.push_back(baseStep);

    while (!crossable.empty()) {

        // 1) pick step from crossable
        
        cerr << "steps count = " << crossable.size() << endl;
        currentStep = crossable.begin(); // TODO: randomize

        // related entry
        const Entry &currentEntry = getEntryAt(currentStep->m_cell);
        const unsigned maxLength = currentStep->getBoundaries(&lower, &upper);

        cerr << "current step at " << *currentStep << " with max length " << maxLength << endl;
        
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
        if (currentEntry.m_direction & currentStep->m_direction) {
            cerr << "\tskipping (overlapping existing word)" << endl;
            crossable.erase(currentStep);
            continue;
        }

        // 2) place word
        
        // 3) block surrounding cells
        
        // 4) print grid with new word
        
        cout << *this << endl;
        
        // 5) add word cells as new steps (invert direction)
        
        // 6) remove step

        crossable.erase(crossable.begin());
    }

    // 7) commit remaining empty cells

    finishFilling();
}

Grid *FillIn::createGrid() const
{
    // TODO
    return NULL;
}

#pragma mark - Subproblems

unsigned FillIn::Step::getBoundaries(CellAddress *lower, CellAddress *upper) const
{
    return m_fillIn->getStructure().m_maxLength;
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

ostream &operator<<(ostream &out, const FillIn::Step &step)
{
    out << "<" << step.m_cell.m_row << ", " << step.m_cell.m_column << "> ";
    out << "(" << FillIn::Entry::getDirectionString(step.m_direction) << ")";
    
    return out;
}
