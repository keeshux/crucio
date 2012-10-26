/*
 * Dictionary.cc
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

#include "Dictionary.h"

using namespace crucio;
using namespace std;

/* Dictionary */

// wildcard (any characater)
const char Dictionary::ANY_CHAR = '-';

// all ones 26-bit mask (any [A-Z] letter)
const ABMask Dictionary::ANY_MASK = ABMask(0x03FFFFFF);

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

ostream& operator<<(ostream& out, const MatchingResult* const res) {
    out << "{ ";
    const vector<uint32_t>& ids = res->getIds();
    uint32_t i;
    for (i = 0; i < ids.size(); ++i) {
        out << res->getWord(ids[i]) << " ";
    }
    out << "}";
    
    return out;
}
