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

SolutionMatcher::SolutionMatcher() :
    m_customWords(),
    m_lastWordID(0)
{
}

SolutionMatcher::~SolutionMatcher()
{
}

void SolutionMatcher::loadIndex(WordSetIndex* const wsIndex)
{
    // initially empty
}

bool SolutionMatcher::getMatchings(WordSetIndex* const wsIndex,
                                   const std::string& pattern,
                                   MatchingResult* const res,
                                   const set<uint32_t>* const exclusions)
{
//    cout << "exclusions (" << exclusions->size() << "):" << endl;
//    for (set<uint32_t>::const_iterator i = exclusions->begin();
//         i != exclusions->end();
//         ++i) {
//
//        cout << "\t" << getCustomWord(*i) << endl;
//    }
    
    return true;
}

bool SolutionMatcher::getPossible(WordSetIndex* const wsIndex,
                                  const MatchingResult* const res,
                                  std::vector<ABMask>* const possibleVector)
{
    return true;
}

uint32_t SolutionMatcher::addCustomWord(const string& word)
{
    // search existing words
    uint32_t id = getCustomWordID(word);
    if (id != UINT_MAX) {
        return id;
    }

    // add new word
    id = m_lastWordID;

    // forward and reverse
    cout << "adding '" << word << "' with id " << id << endl;
    m_customWords.insert(make_pair(id, word));
    m_customIDs.insert(make_pair(word, id));
    ++m_lastWordID;
    
    return id;
}

const string& SolutionMatcher::getCustomWord(const uint32_t id) const
{
    static const string dummy = "";
    
    const map<uint32_t, string>::const_iterator wordIt = m_customWords.find(id);
    if (wordIt == m_customWords.end()) {
        return dummy;
    }
    return wordIt->second;
}

uint32_t SolutionMatcher::getCustomWordID(const string& word) const
{
    const map<string, uint32_t>::const_iterator idIt = m_customIDs.find(word);
    if (idIt == m_customIDs.end()) {
        return UINT_MAX;
    }
    return idIt->second;
}

uint32_t SolutionMatcher::removeCustomWordID(const uint32_t id)
{
    const map<uint32_t, string>::iterator wordIt = m_customWords.find(id);
    const string& word = wordIt->second;
    cout << "removing '" << word << "'" << endl;
    m_customIDs.erase(wordIt->second);
    m_customWords.erase(wordIt);

    return id;
}
