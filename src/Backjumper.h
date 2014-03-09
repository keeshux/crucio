//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#ifndef __BACKJUMPER_H
#define __BACKJUMPER_H

#include <set>

#include "Model.h"
#include "Walk.h"

namespace crucio
{
    class Backjumper
    {
    public:
        Backjumper();

        // variables ordering and dependencies
        void configure(const std::vector<uint32_t>& order,
                       const std::vector<std::list<uint32_t> >& deps);

        // backjumping logic
        void jump(const uint32_t i, const std::set<uint32_t>* const failed = NULL);

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
