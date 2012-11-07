/*
 * FillIn.h
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

#ifndef __FILLIN_H
#define __FILLIN_H

#include "Grid.h"

namespace crucio
{
    template <typename I>
    I random_element(I begin, I end)
    {
        const unsigned long n = std::distance(begin, end);
//        const unsigned long divisor = (RAND_MAX + 1) / n;
        
        unsigned long k;
//        do { k = std::rand() / divisor; } while (k >= n);
        k = rand() % n;
        
        std::advance(begin, k);
        return begin;
    }

    class FillIn
    {
    public:
        enum EntryValue
        {
            ENTRY_VAL_NONE      = '?',
            ENTRY_VAL_WHITE     = '-',
            ENTRY_VAL_BLACK     = '#'
        };
        
        enum EntryDirection
        {
            ENTRY_DIR_NONE      = 0x0,
            ENTRY_DIR_ACROSS    = 0x1,
            ENTRY_DIR_DOWN      = 0x2,
            ENTRY_DIR_BOTH      = 0x3 // ACROSS | DOWN
        };
        
        struct Entry
        {
            EntryValue m_value;
            EntryDirection m_direction;

            static EntryDirection getOppositeDirection(const EntryDirection direction)
            {
                if (direction == ENTRY_DIR_ACROSS) {
                    return ENTRY_DIR_DOWN;
                } else if (direction == ENTRY_DIR_DOWN) {
                    return ENTRY_DIR_ACROSS;
                } else {
                    assert(false);

                    // dummy
                    return ENTRY_DIR_NONE;
                }
            }
            
            static char getDirectionChar(const EntryDirection direction);
            static const char *getDirectionString(const EntryDirection direction);

            char getDirectionChar() const;
            const char *getDirectionString() const;
        };
        
        class Word
        {
        public:
            CellAddress m_origin;
            EntryDirection m_direction;
            unsigned m_length;
        };
        
        class Step
        {
        public:
            Step(const FillIn *fillIn, const CellAddress &cell, const EntryDirection direction);
            
            const Entry &getEntry() const
            {
                return m_fillIn->getEntryAt(m_cell);
            }
            const CellAddress &getCell() const
            {
                return m_cell;
            }
            EntryDirection getDirection() const
            {
                return m_direction;
            }
            
            // returns distance (= max length)
            unsigned getBoundaries(CellAddress *lower, CellAddress *upper) const;
            void getRandomWord(Word *word, CellAddress *lower, CellAddress *upper) const;

        private:
            const FillIn *const m_fillIn;
            const CellAddress m_cell;
            const EntryDirection m_direction;

            bool hasValidRow(const CellAddress &cl) const
            {
                return (cl.m_row < m_fillIn->m_structure.m_rows);
            }
            bool hasValidColumn(const CellAddress &cl) const
            {
                return (cl.m_column < m_fillIn->m_structure.m_columns);
            }
            bool isAcrossBegin(const CellAddress &cl) const
            {
                if (!hasValidColumn(cl)) {
                    return false;
                }
                return ((cl.m_column == 0) ||
                        (m_fillIn->getEntryAt(cl.m_row, cl.m_column - 1).m_value != ENTRY_VAL_WHITE));
            }
            bool isAcrossEnd(const CellAddress &cl) const
            {
                if (!hasValidColumn(cl)) {
                    return false;
                }
                return ((cl.m_column == m_fillIn->m_structure.m_columns - 1) ||
                        (m_fillIn->getEntryAt(cl.m_row, cl.m_column + 1).m_value != ENTRY_VAL_WHITE));
            }
            bool isDownBegin(const CellAddress &cl) const
            {
                if (!hasValidRow(cl)) {
                    return false;
                }
                return ((cl.m_row == 0) ||
                        (m_fillIn->getEntryAt(cl.m_row - 1, cl.m_column).m_value != ENTRY_VAL_WHITE));
            }
            bool isDownEnd(const CellAddress &cl) const
            {
                if (!hasValidRow(cl)) {
                    return false;
                }
                return ((cl.m_row == m_fillIn->m_structure.m_rows - 1) ||
                        (m_fillIn->getEntryAt(cl.m_row + 1, cl.m_column).m_value != ENTRY_VAL_WHITE));
            }
        };
        
        FillIn(const GridStructure &structure);
        ~FillIn();

        const GridStructure &getStructure() const
        {
            return m_structure;
        }

        void layout();
        Grid *createGrid() const;
        
        const Entry &getEntryAt(const unsigned i, const unsigned j) const
        {
            return m_entries[i][j];
        }
        const Entry &getEntryAt(const CellAddress &address) const
        {
            return getEntryAt(address.m_row, address.m_column);
        }

    private:
        const GridStructure m_structure;
        Entry **m_entries;

        // subproblems
        void placeWord(const Word *word);
        void blockSurroundingCells(const Word *word);
        void blockRow(const CellAddress *from, const EntryDirection direction, unsigned length);
        void blockColumn(const CellAddress *from, const EntryDirection direction, unsigned length);
        bool blockCell(const CellAddress *cell);
        bool isDenseCrossing(const CellAddress *cell);
        void finishFilling();

        Entry &getEntryAt(const unsigned i, const unsigned j)
        {
            return m_entries[i][j];
        }
        Entry &getEntryAt(const CellAddress &address)
        {
            return getEntryAt(address.m_row, address.m_column);
        }

        // utilities
        static unsigned randomNumber(const unsigned min, const unsigned max)
        {
            return min + rand() % (max - min + 1);
        }
        static EntryDirection randomEntryDirection()
        {
            return (EntryDirection)randomNumber(1, 2);
        }
        CellAddress randomCellAddress() const;
    };
}

std::ostream &operator<<(std::ostream &out, const crucio::FillIn &fillIn);
std::ostream &operator<<(std::ostream &out, const crucio::FillIn::Word &word);
std::ostream &operator<<(std::ostream &out, const crucio::FillIn::Step &step);

#endif
