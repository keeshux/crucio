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
            ENTRY_DIR_DOWN      = 0x2
        };
        
        struct Entry
        {
            EntryValue value;
            EntryDirection direction;

            char getDirectionChar() const;
        };
        
        FillIn(const GridStructure &structure);
        ~FillIn();

        const GridStructure &getStructure() const
        {
            return m_structure;
        }

        void layout();
        void complete();

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

        class Step
        {
        public:
            CellAddress m_cell;
            EntryDirection m_direction;
        };
        
        static unsigned randomNumber(const unsigned min, const unsigned max)
        {
            return min + rand() % (max - min);
        }

        static EntryDirection randomEntryDirection()
        {
            return (EntryDirection)randomNumber(1, 2);
        }
        CellAddress randomCellAddress() const;
    };
}

std::ostream &operator<<(std::ostream &out, const crucio::FillIn &fi);

#endif
