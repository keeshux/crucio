/*
 * FillInFactory.h
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

#ifndef __FILLIN_FACTORY_H
#define __FILLIN_FACTORY_H

#include "Grid.h"

namespace crucio
{
    struct GridStructure
    {
        unsigned rows;
        unsigned columns;
        unsigned minLength;
        unsigned maxLength;
    };
    
    class FillIn
    {
    public:
        FillIn(const GridStructure &structure);
        ~FillIn();

        const GridStructure &getStructure() const
        {
            return m_structure;
        }
        const char getEntryAt(const unsigned i, const unsigned j) const
        {
            return (char)m_entries[i][j].value;
        }
        const char getEntryAt(const CellAddress &address) const
        {
            return getEntryAt(address.m_row, address.m_column);
        }

        void layout();
        void complete();

        Grid *createGrid() const;
        
    private:
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

        struct GridEntry
        {
            EntryValue value;
            EntryDirection direction;
        };

        const GridStructure m_structure;
        GridEntry **m_entries;
    };
}

std::ostream &operator<<(std::ostream &out, const crucio::FillIn &fi);

#endif
