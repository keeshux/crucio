/*
 * UnicityMatcher.cc
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

#include "UnicityMatcher.h"

using namespace crucio;
using namespace std;

UnicityMatcher::UnicityMatcher()
{
}

UnicityMatcher::~UnicityMatcher()
{
}

void UnicityMatcher::loadIndex(WordSetIndex* const wsIndex)
{
    // initially empty
}

bool UnicityMatcher::getMatchings(WordSetIndex *const wsIndex,
                                  const std::string& pattern,
                                  MatchingResult* const res,
                                  const std::set<uint32_t>* const excluded)
{
    return true;
}

bool UnicityMatcher::getPossible(WordSetIndex *const wsIndex,
                                 const MatchingResult* const res,
                                 std::vector<ABMask>* const possibleVector)
{
    return true;
}
