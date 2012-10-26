/*
 * SolutionMatcher.cc
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

#include "SolutionMatcher.h"

using namespace crucio;
using namespace std;

SolutionMatcher::SolutionMatcher()
{
}

SolutionMatcher::~SolutionMatcher()
{
}

void SolutionMatcher::loadIndex(WordSetIndex* const wsIndex) const
{
}

bool SolutionMatcher::getMatchings(WordSetIndex *const wsIndex,
                                   const std::string& pattern,
                                   MatchingResult* const res,
                                   const std::set<uint32_t>* const excluded) const
{
    return true;
}

bool SolutionMatcher::getPossible(WordSetIndex *const wsIndex,
                                  const MatchingResult* const res,
                                  const uint32_t pos,
                                  ABMask* const possible) const
{
    return true;
}

bool SolutionMatcher::getPossible(WordSetIndex *const wsIndex,
                                  const MatchingResult* const res,
                                  std::vector<ABMask>* const possibleVector) const
{
    return true;
}
