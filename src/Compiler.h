/*
 * Compiler.h
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

#ifndef __COMPILER_H
#define __COMPILER_H

#include "Model.h"
#include "Output.h"
#include "Walk.h"

namespace crucio
{
    class Compiler
    {
    public:
        enum Result {
            SUCCESS,
            FAILURE_IMPOSSIBLE,
            FAILURE_OVERCONSTRAINED,
            FAILURE_ND_GRID,
            FAILURE_TIMEOUT
        };

        Compiler();
        virtual ~Compiler();

        // no repeated words constraint
        void setUnique(const bool unique) {
            m_unique = unique;
        }
        bool isUnique() const {
            return m_unique;
        }

        // deterministic solution constraint
        void setDeterministic(const bool deterministic) {
            m_deterministic = deterministic;
        }
        bool isDeterministic() const {
            return m_deterministic;
        }
        
        // timeout constraint
        void setTimeoutMillis(const unsigned timeoutMillis) {
            m_timeoutMillis = timeoutMillis;
        }
        unsigned getTimeoutMillis() {
            return m_timeoutMillis;
        }

        // algorithm execution
        Result compile(Model* const model, const Walk& walk);

    protected:
        
        // TODO: convert subclasses to interface implementations

        // callbacks
        virtual Compiler::Result configure(const Walk& walk) = 0;
        virtual void reset() = 0;
        virtual Compiler::Result compileFrom(const uint32_t i) = 0;
        
        // timeout to be checked periodically in subclasses
        bool isTimeout() const
        {
            // 0 = no timeout
            if (m_timeoutMillis == 0) {
                return false;
            }

            // timeout millis elapsed
            return ((time(NULL) - m_compileMillis) > m_timeoutMillis);
        }

    private:

        // used by determinism check
        class IsSimpleDomain
        {
        public:

            // <length, words-by-length> pair
            bool operator()(const std::pair<uint32_t,
                            std::set<uint32_t> >& lenWords) const {
                return (lenWords.second.size() == 1);
            }

            // a pattern's domain cardinality is matching words count
            bool operator()(const Word* const w) const {
                return (w && (w->getMatchingResult()->getSize() == 1));
            }
        };

        // parameters
        bool m_unique;
        bool m_deterministic;
        time_t m_compileMillis;
        unsigned m_timeoutMillis;

        // determinism check
        bool isDeterministicSolution() const;

    protected:

        // model reference
        Model* m_model;
    };
}

#endif
