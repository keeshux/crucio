//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#ifndef __LETTER_COMPILER_H
#define __LETTER_COMPILER_H

#include <algorithm>
#include <iostream>
#include <list>
#include <set>
#include <stack>
#include <vector>

#include "common.h"
#include "Backjumper.h"
#include "Compiler.h"

namespace crucio
{
    class LetterCompiler : public Compiler
    {
    public:
        LetterCompiler();

    protected:
        virtual Compiler::Result configure(const Walk& walk);
        virtual void reset();
        virtual Compiler::Result compileFrom(const uint32_t i);

    private:

        // internal objects
        std::vector<ABMask> m_domains;
        std::vector<uint32_t> m_order;
        std::vector<uint32_t> m_revOrder;
        std::vector<std::list<std::pair<uint32_t, LetterPosition> > > m_deps;
        std::vector<std::list<std::pair<uint32_t, LetterPosition> > > m_revDeps;
        Backjumper m_bj;

        // depending on model alphabet
        Alphabet m_alphabet;
        uint32_t m_alphabetSize;

        // subproblems
        char choose(ABMask* const domainMask);
        bool assign(const uint32_t li,
                    const char v,
                    std::stack<std::pair<uint32_t, ABMask> >* const remStack,
                    std::set<uint32_t>* const failed);
        void retire(const uint32_t li,
                    std::stack<std::pair<uint32_t, ABMask> >* const remStack);
    };
}

#endif
