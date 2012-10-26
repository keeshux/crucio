/*
 * Walk.h
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
