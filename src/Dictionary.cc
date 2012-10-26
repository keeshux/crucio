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

Dictionary::Dictionary(const Matcher* const matcher) :
    m_matcher(matcher),
    m_index(new WordSetIndex(MIN_LENGTH, MAX_LENGTH))
{

    // load index through matcher
    loadIndex();
}

Dictionary::~Dictionary()
{
    delete m_index;
}

MatchingResult* Dictionary::createMatchingResult(const uint32_t len) const
{
    return new MatchingResult(this, len);
}

void Dictionary::destroyMatchingResult(MatchingResult* const res) const
{
    delete res;
}

/* <global> */

ostream& operator<<(ostream& out, const MatchingResult* const res)
{
    out << "{ ";
    const vector<uint32_t>& ids = res->getIds();
    uint32_t i;
    for (i = 0; i < ids.size(); ++i) {
        out << res->getWord(ids[i]) << " ";
    }
    out << "}";

    return out;
}
