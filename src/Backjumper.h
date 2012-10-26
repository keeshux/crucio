/*
 * Backjumper.h
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

#ifndef __BACKJUMPER_H
#define __BACKJUMPER_H

#include <set>

#include "Model.h"
#include "Walk.h"

namespace crucio {
    class Backjumper {
    public:
        Backjumper();

        // variables ordering and dependencies
        void configure(const std::vector<uint32_t>& order,
                       const std::vector<std::list<uint32_t> >& deps);

        // backjumping logic
        void jump(const uint32_t i, const std::set<uint32_t>* const failed = 0);

        bool isExhausted() const {
            return m_exhausted;
        }
        uint32_t getOrigin() const {
            return m_origin;
        }
        uint32_t getDestination() const {
            return m_destination;
        }

        // clears jumps set
        void reset() {
            m_jumps.clear();
            m_exhausted = true;
        }

    private:

        // variables ordering and dependencies
        std::vector<uint32_t> m_order;
        std::vector<uint32_t> m_revOrder;
        std::vector<std::list<uint32_t> > m_deps;

        std::set<uint32_t, std::greater<uint32_t> > m_jumps;
        bool m_exhausted;
        uint32_t m_origin;
        uint32_t m_destination;
    };
}

#endif
