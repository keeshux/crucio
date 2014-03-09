//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#ifndef __WALK_H
#define __WALK_H

#include <queue>
#include <set>
#include <vector>

#include "common.h"
#include "Model.h"

namespace crucio
{
    class Walk
    {
    public:
        static void getReverseOrder(const std::vector<uint32_t>&,
                                    std::vector<uint32_t>* const);

        virtual ~Walk() {
        }

        virtual void visitLetters(const Model&,
                                  std::vector<uint32_t>* const) const = 0;
        virtual void visitWords(const Model&,
                                std::vector<uint32_t>* const) const = 0;
    };

// breadth-first search
    class BFSWalk : public Walk
    {
    public:
        virtual void visitLetters(const Model&,
                                  std::vector<uint32_t>* const) const;
        virtual void visitWords(const Model&,
                                std::vector<uint32_t>* const) const;
    };

// depth-first search
    class DFSWalk : public Walk
    {
    public:
        virtual void visitLetters(const Model&,
                                  std::vector<uint32_t>* const) const;
        virtual void visitWords(const Model&,
                                std::vector<uint32_t>* const) const;

    private:
        static void recursiveVisitLetters(const Model&,
                                          const Cell* const, std::vector<uint32_t>* const,
                                          std::set<const Cell*, CellCompare>* const);
        static void recursiveVisitWords(const Model&,
                                        const Definition* const, std::vector<uint32_t>* const,
                                        std::set<const Definition*, DefinitionCompare>* const);
    };
}

#endif
