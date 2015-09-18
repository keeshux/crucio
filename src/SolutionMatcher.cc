//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#include "SolutionMatcher.h"
#include "Word.h"

using namespace crucio;
using namespace std;

// pattern matches exclusion for each assigned letter (!= ANY_CHAR)
bool SolutionMatcher::isMatchingExclusion(const string& pattern,
        const string& exclusion)
{
    const uint32_t len = (uint32_t)pattern.length();
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

    *crucio_vout << "\tword " << *word->getDefinition() <<
                 " pattern: " << word->get() << " (" <<
                 exclusions.size() << " exclusions)" << endl;

    // start from full letter masks
    for (pos = 0; pos < len; ++pos) {
        ABMask* const possible = &possibleVector[pos];
        setAnyMask(alphabet, possible);
    }
    
    // only check words with a single missing character
    const uint32_t wildcards = word->getWildcards();
    if (wildcards > 1) {
        *crucio_vout << "\t\tskipped multiple wildcards" << endl;
        return true;
    }

    // filter out through exclusions
    set<uint32_t>::const_iterator xIt;
    for (xIt = exclusions.begin(); xIt != exclusions.end(); ++xIt) {

        // current excluded word
        const uint32_t wi = *xIt;
        const string& xword = getCustomWord(wi);

        // ignore unmatching exclusions
        if (!isMatchingExclusion(pattern, xword)) {
            *crucio_vout << "\t\tskipped unmatching exclusion: " <<
                         xword << endl;

            continue;
        }

        *crucio_vout << "\t\tanalyzing exclusion: " << xword << endl;

        // reset excluded word letters in masks
        for (pos = 0; pos < len; ++pos) {
            ABMask* const possible = &possibleVector[pos];

            // skip unassigned characters
            if (pattern[pos] != ANY_CHAR) {
                continue;
            }

            *crucio_vout << "\t\t\tBEFORE: domain[" << pos << "] = " <<
                         ABMaskString(alphabet, *possible) << endl;

            const char xch = xword.at(pos);
            const uint32_t xi = character2Index(alphabet, xch);
            possible->reset(xi);

            *crucio_vout << "\t\t\tAFTER:  domain[" << pos << "] = " <<
                         ABMaskString(alphabet, *possible) << endl;
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
    *crucio_vout << "adding custom word: " << word <<
                 " (id = " << id << ")" << endl;
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
    *crucio_vout << "removing custom word: " << word << endl;

    m_customIDs.erase(wordIt->second);
    m_customWords.erase(wordIt);

    return id;
}
