#include "Dictionary.h"

using namespace crucio;
using namespace std;

/* WordSet */

WordSet::WordSet(const uint32_t len) :
        m_words(),
        m_length(len),
        m_cpMatrix(len * ALPHABET_SIZE) {
}

void WordSet::insert(const string& word) {

    // words MUST have fixed length
    if (word.length() != m_length) {
        throw DictionaryException("WordSet: invalid word length");
    }

    // insertion
    const uint32_t id = m_words.size();
    m_words.push_back(word);

    // indexing
    const uint32_t len = word.length();
    for (uint32_t pos = 0; pos < len; ++pos) {

        // computes mapping (position=pos, letter=word[pos])
        const uint32_t bucket = getHash(pos, word[pos]);

        // adds word ID (ascending) into the bucket
        m_cpMatrix[bucket].push_back(id);
    }
}

/* Dictionary */

// wildcard (any characater)
const char Dictionary::ANY_CHAR = '-';

// all ones 26-bit mask (any [A-Z] letter)
const ABMask Dictionary::ANY_MASK = ABMask(0x03FFFFFF);

Dictionary::Dictionary(const set<string>& words) :
        m_filename(),
        m_index(MIN_LENGTH, MAX_LENGTH) {

    // loads words
    set<string>::const_iterator wIt;
    for (wIt = words.begin(); wIt != words.end(); ++wIt) {
        const string& word = *wIt;

        // checks word's length and format
        if (isValidWord(word)) {

            // makes a copy
            string upperWord = word;

            // IMPORTANT: turns to uppercase
            for_each(upperWord.begin(), upperWord.end(), MakeUpper());

            // selects wordset for insertion
            WordSet* const ws = m_index.getWordSet(upperWord.length());
            ws->insert(upperWord);
        }
    }
}

Dictionary::Dictionary(const string& filename) :
        m_filename(filename),
        m_index(MIN_LENGTH, MAX_LENGTH) {

    // temporary string vector to load words
    vector<string> sortedWords;

    // reserves 500k entries
    sortedWords.reserve(500000);

    // opens words list file
    ifstream wordsIn(filename.c_str());
    if (!wordsIn.is_open()) {
        throw DictionaryException("dictionary: unable to open words list");
    }

#ifdef USE_BENCHMARK
    // loads words directly (a word each line)
    string word;
    while (getline(wordsIn, word)) {
        if (isValidWord(word)) {
            for_each(word.begin(), word.end(), MakeUpper());

            // selects wordset for insertion
            WordSet* const ws = m_index.getWordSet(word.length());
            ws->insert(word);
        }
    }

    // closes file
    wordsIn.close();
#else
    // loads words into vector (a word each line)
    string word;
    while (getline(wordsIn, word)) {

        // checks word's length and format
        if (isValidWord(word)) {

            // IMPORTANT: turns to uppercase
            for_each(word.begin(), word.end(), MakeUpper());

            // adds word into the vector
            sortedWords.push_back(word);
        }
    }

    // closes file
    wordsIn.close();

    // IMPORTANT: never count on an already sorted-and-unique word
    // list (in order to do binary search)
    sort(sortedWords.begin(), sortedWords.end());
    unique(sortedWords.begin(), sortedWords.end());

    // adds filtered words
    vector<string>::const_iterator wIt;
    for (wIt = sortedWords.begin(); wIt != sortedWords.end(); ++wIt) {
        const string& word = *wIt;

        // selects wordset for insertion
        WordSet* const ws = m_index.getWordSet(word.length());
        ws->insert(word);
    }
#endif
}

Dictionary::~Dictionary() {
}

bool Dictionary::getMatchings(const string& pattern,
        MatchingResult* const res, const set<uint32_t>* const excluded) const {
    const uint32_t len = pattern.length();

    // initially empty result
    res->m_ids.clear();

    // single letters matching
    const WordSet* const ws = m_index.getWordSet(len);
    list<const vector<uint32_t>* > cpVectors;
    for (uint32_t pi = 0; pi < len; ++pi) {
        if (pattern[pi] != ANY_CHAR) {
            cpVectors.push_back(ws->getCPVector(pi, pattern[pi]));
        }
    }

    // wild pattern, that is an ANY_CHAR-only pattern; this maps to whole
    // subdictionary
    if (cpVectors.empty()) {

        // IMPORTANT: very expensive, but only happens at model creation; this
        // is because *_compiler classes only call Word::doMatch() after
        // having ASSIGNED a letter/word, not when they are retired; so empty
        // masks are _never_ re-matched, apart at model creation time when
        // initial domains have to be evaluated
        const uint32_t wsSize = ws->getSize();
        res->m_ids.reserve(wsSize);
        for (uint32_t id = 0; id < wsSize; ++id) {
            res->m_ids.push_back(id);
        }
        return true;
    }

    // intersects matchings; set intersection is at most large as smallest set,
    // so other sets are filtered on this one; smallest set search cost is a
    // good tradeoff for subsequent computation
    list<const vector<uint32_t>* >::const_iterator minSetIt;
    minSetIt = min_element(cpVectors.begin(), cpVectors.end(), MinSizePtr());
    const vector<uint32_t>* const minSet = *minSetIt;

    // iterates over minSet and search for its elements in the other sets
    vector<uint32_t>::const_iterator idIt;
    list<const vector<uint32_t>* >::const_iterator setIt;
    for (idIt = minSet->begin(); idIt != minSet->end(); ++idIt) {
        const uint32_t id = *idIt;

        // skips excluded elements (if given)
        if (excluded && (excluded->find(id) != excluded->end())) {
            continue;
        }

        // element must be found in every set
        bool foundAll = true;
        for (setIt = cpVectors.begin(); setIt != cpVectors.end(); ++setIt) {

            // skips minSet
            if (setIt == minSetIt) {
                continue;
            }

            const vector<uint32_t>* const currSet = *setIt;

            // searches the element in this set (set is sorted since
            // WordSet::insert() assigns ascending IDs, so binary
            // search is MUCH better)
            //if (find(currSet->begin(), currSet->end(), id) ==
            //      currSet->end()) {
            if (!binary_search(currSet->begin(), currSet->end(), id)) {
                foundAll = false;
                break;
            }
        }

        // if found in every set, element is added to intersection
        if (foundAll) {
            res->m_ids.push_back(id);
        }
    }

    // true if intersection is not empty
    return !res->m_ids.empty();
}

bool Dictionary::getPossible(const MatchingResult* const res,
        const uint32_t pos, ABMask* const possible) const {
    const uint32_t len = res->getWordsLength();

    // initially empty letter mask
    possible->reset();

    // no matchings, empty mask is returned
    if (res->isEmpty()) {
        return false;
    }

    // current wordset
    const WordSet* const ws = m_index.getWordSet(len);

    // are matchings equal to whole subdictionary?
    if (res->isFull()) {
        ws->getPossibleAt(pos, possible);
    } else {

        // iterates over matching words IDs
        const vector<uint32_t>& ids = res->getIds();
        vector<uint32_t>::const_iterator idIt;
        for (idIt = ids.begin(); idIt != ids.end(); ++idIt) {
            const uint32_t id = *idIt;
            const string& word = ws->getWord(id);

            // letter index at position pos in the word
            const uint32_t chIndex = reverseAlphabet(word[pos]);

            // puts letter into letter mask
            possible->set(chIndex);
        }
    }

    return true;
}

bool Dictionary::getPossible(const MatchingResult* const res,
        vector<ABMask>* const possibleVector) const {

    // fixed length for words in matching result
    const uint32_t len = res->getWordsLength();

    // character position in a word or pattern
    uint32_t pos;

    // initially empty letter masks
    for (pos = 0; pos < len; ++pos) {
        ABMask* const possible = &(*possibleVector)[pos];
        possible->reset();
    }

    // no matchings, empty masks are returned
    if (res->isEmpty()) {
        return false;
    }

    // current wordset
    const WordSet* const ws = m_index.getWordSet(len);

    // are matchings equal to whole subdictionary?
    if (res->isFull()) {
        for (pos = 0; pos < len; ++pos) {
            ABMask* const possible = &(*possibleVector)[pos];
            ws->getPossibleAt(pos, possible);
        }
    } else {

        // iterates over matching words IDs
        const vector<uint32_t>& ids = res->getIds();
        vector<uint32_t>::const_iterator idIt;
        for (idIt = ids.begin(); idIt != ids.end(); ++idIt) {
            const uint32_t id = *idIt;
            const string& word = ws->getWord(id);

            // finds possible letters at every position
            for (pos = 0; pos < len; ++pos) {
                ABMask* const possible = &(*possibleVector)[pos];

                // letter index at position pos in the word
                const uint32_t chIndex = reverseAlphabet(word[pos]);

                // puts letter into letter mask
                possible->set(chIndex);
            }
        }
    }

    return true;
}

/* <global> */

ostream& operator<<(ostream& out, const ABMask& m) {
    out << "{";
    uint32_t i;
    for (i = 0; i < m.size(); ++i) {
        if (m[i]) {
            out << alphabet(i);
        }
    }
    out << "}";

    return out;
}

ostream& operator<<(ostream& out,
        const Dictionary::MatchingResult* const res) {
    out << "{ ";
    const vector<uint32_t>& ids = res->getIds();
    uint32_t i;
    for (i = 0; i < ids.size(); ++i) {
        out << res->getWord(ids[i]) << " ";
    }
    out << "}";

    return out;
}
