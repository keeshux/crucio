//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#ifndef __LETTER_H
#define __LETTER_H

#include "Dictionary.h"
#include "Grid.h"

namespace crucio
{
    class Letter
    {
    public:
        Letter(const Cell* cellRef) :
            m_cellRef(cellRef),
            m_value(ANY_CHAR) {
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
            m_value = ANY_CHAR;
        }

    private:
        const Cell* const m_cellRef;
        char m_value;
    };
}

#endif
