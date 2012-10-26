/*
 * LetterCompiler.h
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
        virtual Compiler::Result configure(const Walk& w);
        virtual void reset();
        virtual bool compileFrom(const uint32_t i);

    private:

        // internal objects
        std::vector<ABMask> m_domains;
        std::vector<uint32_t> m_order;
        std::vector<uint32_t> m_revOrder;
        std::vector<std::list<std::pair<uint32_t, LetterPosition> > > m_deps;
        std::vector<std::list<std::pair<uint32_t, LetterPosition> > > m_revDeps;
        Backjumper m_bj;

        // depending on compiler type
        uint32_t m_alphabetSize;

        // subproblems
        char choose(ABMask* const domainMask);
        bool assign(const uint32_t li,
                    const char v,
                    std::stack<std::pair<uint32_t, ABMask> >* const remStack,
                    std::set<uint32_t>* const failed);
        void retire(const uint32_t li,
                    std::stack<std::pair<uint32_t, ABMask> >* const remStack);

        // alphabet pointers
        char (*m_index2Char)(const uint32_t i);
        uint32_t (*m_char2Index)(const char ch);
    };
}

#endif
