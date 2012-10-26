/*
 * WordSet.cc
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

#include "WordSet.h"

using namespace crucio;
using namespace std;

/* WordSetIndex */

WordSetIndex::WordSetIndex(const uint32_t minLength, const uint32_t maxLength) :
        m_minLength(minLength),
        m_maxLength(maxLength),
        m_wordSets(maxLength - minLength + 1) {
    
    const uint32_t wsSize = m_wordSets.size();
    for (uint32_t wsi = 0; wsi < wsSize; ++wsi) {
        m_wordSets[wsi] = new WordSet(getReverseHash(wsi));
    }
}

WordSetIndex::~WordSetIndex() {
    const uint32_t wsSize = m_wordSets.size();
    for (uint32_t wsi = 0; wsi < wsSize; ++wsi) {
        delete m_wordSets[wsi];
    }
}

// computes size as wordsets sizes sum
uint32_t WordSetIndex::getSize() const {
    uint32_t totalSize = 0;
    vector<WordSet*>::const_iterator wsIt;
    for (wsIt = m_wordSets.begin(); wsIt !=
         m_wordSets.end(); ++wsIt) {
        const WordSet* const ws = *wsIt;
        totalSize += ws->getSize();
    }
    return totalSize;
}

/* <global> */

ostream& operator<<(ostream& out, const ABMask& m) {
    out << "{";
    uint32_t i;
    for (i = 0; i < m.size(); ++i) {
        if (m[i]) {
            out << index2Letter(i);
        }
    }
    out << "}";
    
    return out;
}
