/*
 * SolutionDictionary.cc
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

#include "SolutionDictionary.h"

using namespace crucio;
using namespace std;

SolutionDictionary::SolutionDictionary() {
}

SolutionDictionary::~SolutionDictionary() {
}

uint32_t SolutionDictionary::getSize() const {
    return 0;
}

uint32_t SolutionDictionary::getSize(const uint32_t) const {
    return 0;
}

const string SolutionDictionary::getWord(const uint32_t, const uint32_t) const {
    return "";
}

bool SolutionDictionary::getMatchings(const std::string&, MatchingResult* const,
                                      const set<uint32_t>* const) const {

    return true;
}

bool SolutionDictionary::getPossible(const MatchingResult* const, const uint32_t,
                                     ABMask* const) const {

    return true;
}

bool SolutionDictionary::getPossible(const MatchingResult* const,
                                     vector<ABMask>* const) const {

    return true;
}
