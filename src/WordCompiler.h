/*
 * WordCompiler.h
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

#ifndef __WORD_COMPILER_H
#define __WORD_COMPILER_H

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
    class WordCompiler : public Compiler
    {
    public:
        WordCompiler();

    protected:
        virtual Compiler::Result configure(const Walk& walk);
        virtual void reset();
        virtual Compiler::Result compileFrom(const uint32_t i);

    private:

        // internal objects
        std::vector<std::set<uint32_t> > m_domains;
        std::vector<uint32_t> m_order;
        std::vector<uint32_t> m_revOrder;
        std::vector<std::list<std::pair<uint32_t, WordCrossing> > > m_deps;
        std::vector<std::list<std::pair<uint32_t, WordCrossing> > > m_revDeps;
        Backjumper m_bj;

        // subproblems
        void choose(const uint32_t wLen,
                    std::set<uint32_t>* const domainSet,
                    std::string* const v);
        bool assign(const uint32_t wi,
                    const std::string& v,
                    std::string* const oldV,
                    std::stack<std::pair<uint32_t, uint32_t> >* const remStack,
                    std::set<uint32_t>* const failed);
        void retire(const uint32_t wi,
                    const std::string& oldV,
                    std::stack<std::pair<uint32_t, uint32_t> >* const remStack);
    };
}

#endif
