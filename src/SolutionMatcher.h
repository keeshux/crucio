/*
 * SolutionMatcher.h
 * crucio
 *
 * Copyright 2012 Davide De Rosa
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

#ifndef __SOLUTION_MATCHER_H
#define __SOLUTION_MATCHER_H

#include "Dictionary.h"

namespace crucio {

    class SolutionMatcher : public Matcher {
    public:
        SolutionMatcher();
        virtual ~SolutionMatcher();

        virtual void loadIndex(WordSetIndex* const wsIndex) const;

        virtual bool getMatchings(WordSetIndex *const wsIndex,
                                  const std::string& pattern,
                                  MatchingResult* const res,
                                  const std::set<uint32_t>* const excluded = 0) const;

        virtual bool getPossible(WordSetIndex *const wsIndex,
                                 const MatchingResult* const res,
                                 const uint32_t pos,
                                 ABMask* const possible) const;

        virtual bool getPossible(WordSetIndex *const wsIndex,
                                 const MatchingResult* const res,
                                 std::vector<ABMask>* const possibleVector) const;

    };
}

#endif
