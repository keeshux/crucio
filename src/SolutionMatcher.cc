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
#include "Word.h"

using namespace crucio;
using namespace std;

bool isMatchingExclusion(const string& pattern, const string& exclusion) {
    const uint32_t len = pattern.length();
    
    for (uint32_t pos = 0; pos < len; ++pos) {
        if ((pattern[pos] != ANY_CHAR) &&
            (pattern[pos] != exclusion[pos])) {
            
            return false;
        }
    }
    return true;
}

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
                                   Word* const word)
{
    return true;
}

bool SolutionMatcher::getPossible(WordSetIndex* const wsIndex,
                                  Word* const word)
{
    const string& pattern = word->get();
    const MatchingResult* const res = word->getMatchings();
    vector<ABMask>& possibleVector = word->getAllowed();
    const set<uint32_t>& exclusions = word->getExclusions();

    // fixed length for words in matching result
    const Alphabet alphabet = wsIndex->getAlphabet();
    const uint32_t len = res->getWordsLength();
    
    // character position in a word or pattern
    uint32_t pos;

//    cout << "word " << word->getDefinition()->getIndex() << " pattern '" << word->get() << "', exclusions (" << exclusions.size() << "):" << endl;

    // XXX: known in Word
    const uint32_t wildcards = count(pattern.begin(), pattern.end(), ANY_CHAR);
    if (wildcards > 1) {
//        cout << "\tmultiple wildcards, skipped" << endl;
        return true;
    }

    // start from full letter masks
    for (pos = 0; pos < len; ++pos) {
        ABMask* const possible = &possibleVector[pos];
        setAnyMask(alphabet, possible);
    }
    
    // filter out through exclusions
    set<uint32_t>::const_iterator exIt;
    for (exIt = exclusions.begin(); exIt != exclusions.end(); ++exIt) {

        // current excluded word
        const uint32_t wi = *exIt;
        const string& exWord = getCustomWord(wi);
        
        // ignore unmatching exclusions
        if (!isMatchingExclusion(pattern, exWord)) {
//            cout << "\t" << exWord << ", skipped" << endl;
            continue;
        }

//        cout << "\t" << exWord << endl;
        
        // reset excluded word letters in masks
        for (pos = 0; pos < len; ++pos) {
            ABMask* const possible = &possibleVector[pos];
            
            // skip unassigned characters
            if (pattern[pos] != ANY_CHAR) {
                continue;
            }

//            cout << "\t\tBEFORE: domain[" << pos << "] = " <<
//                    ABMaskString(alphabet, *possible) << endl;

            const char xch = exWord.at(pos);
            const uint32_t xi = character2Index(alphabet, xch);
            possible->reset(xi);

//            cout << "\t\tAFTER: domain[" << pos << "] = " <<
//                    ABMaskString(alphabet, *possible) << endl;
        }
    }

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
//    cout << "adding '" << word << "' with id " << id << endl;
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
//    const string& word = wordIt->second;
//    cout << "removing '" << word << "'" << endl;
    m_customIDs.erase(wordIt->second);
    m_customWords.erase(wordIt);

    return id;
}
