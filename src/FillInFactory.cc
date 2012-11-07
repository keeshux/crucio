/*
 * FillInFactory.cc
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

#include "FillInFactory.h"

using namespace crucio;
using namespace std;

FillIn::FillIn(const GridStructure &structure) : m_structure(structure)
{
    unsigned i, j;
    
    m_entries = new GridEntry*[m_structure.rows];
    for (i = 0; i < m_structure.rows; ++i) {
        m_entries[i] = new GridEntry[m_structure.columns];
        for (i = 0; i < m_structure.rows; ++i) {
            m_entries[i][j].value = ENTRY_VAL_NONE;
        }
    }
}

FillIn::~FillIn()
{
    unsigned i;
    
    for (i = 0; i < m_structure.rows; ++i) {
        delete[] m_entries[i];
    }
    delete[] m_entries;
}

void FillIn::layout()
{
}

void FillIn::complete()
{
}

ostream &operator<<(ostream &out, const FillIn &fi)
{
    return out;
}