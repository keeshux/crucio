#ifndef __LETTER_COMPILER_H
#define __LETTER_COMPILER_H

#include <algorithm>
#include <iostream>
#include <list>
#include <set>
#include <stack>
#include <vector>

#include "Backjumper.h"
#include "Compiler.h"

namespace crucio {
    class LetterCompiler : public Compiler {
    public:
        LetterCompiler();

    protected:
        virtual void configure(const Walk&);
        virtual void reset();
        virtual bool compileFrom(const uint32_t);

    private:

        // internal objects
        std::vector<ABMask> m_domains;
        std::vector<uint32_t> m_order;
        std::vector<uint32_t> m_revOrder;
        std::vector<std::list<std::pair<uint32_t, LetterPosition> > > m_deps;
        std::vector<std::list<std::pair<uint32_t, LetterPosition> > > m_revDeps;
        Backjumper m_bj;

        // subproblems
        char choose(ABMask* const);
        bool assign(const uint32_t, const char,
                std::stack<std::pair<uint32_t, ABMask> >* const,
                std::set<uint32_t>* const);
        void retire(const uint32_t,
                std::stack<std::pair<uint32_t, ABMask> >* const);
    };
}

#endif
