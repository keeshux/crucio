//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#include "Dictionary.h"

using namespace crucio;
using namespace std;

/* Dictionary */

Dictionary::Dictionary(const Alphabet alphabet, Matcher* const matcher) :
    m_matcher(matcher),
    m_index(new WordSetIndex(alphabet, MIN_LENGTH, MAX_LENGTH))
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
    const vector<uint32_t>& ids = res->getIDs();
    uint32_t i;
    for (i = 0; i < ids.size(); ++i) {
        out << res->getWord(ids[i]) << " ";
    }
    out << "}";

    return out;
}
