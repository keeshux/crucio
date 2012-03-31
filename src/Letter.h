/*
 * Letter.h
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

#ifndef __LETTER_H
#define __LETTER_H

#include "Dictionary.h"
#include "Grid.h"

namespace crucio {
    class Letter {
    public:
        Letter(const Cell* cellRef) :
                m_cellRef(cellRef),
                m_value(Dictionary::ANY_CHAR) {
        }

        // referred cell
        const Cell* getCell() const {
            return m_cellRef;
        }

        // assignment
        char get() const {
            return m_value;
        }
        void set(const char value) {
            m_value = value;
        }
        void unset() {
            m_value = Dictionary::ANY_CHAR;
        }

    private:
        const Cell* const m_cellRef;
        char m_value;
    };
}

#endif
