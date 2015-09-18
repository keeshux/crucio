//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#include "LanguageMatcher.h"
#include "Word.h"

using namespace crucio;
using namespace std;

// checks for a word to be only-ASCII and in [MIN_LENGTH, MAX_LENGTH]
bool LanguageMatcher::isValidWord(const string& word)
{
    if ((word.length() < Dictionary::MIN_LENGTH) ||
            (word.length() > Dictionary::MAX_LENGTH)) {

        return false;
    }

    // true if no non-ASCII letters, that is (IsNotAscii() == false)
    // for each letter in the word
    return (find_if(word.begin(), word.end(), IsNotAscii()) == word.end());
}

LanguageMatcher::LanguageMatcher(const set<string>* const words) :
    m_words(words),
    m_filename()
{
    assert(words != NULL);
}

LanguageMatcher::LanguageMatcher(const string& filename) :
    m_words(NULL),
    m_filename(filename)
{
    assert(!filename.empty());
}

LanguageMatcher::~LanguageMatcher()
{
}

void LanguageMatcher::loadIndex(WordSetIndex* const wsIndex)
{
    if (m_words) {
        loadWords(wsIndex);
    } else {
        loadFilename(wsIndex);
    }

    // save alphabet locally
    m_alphabet = wsIndex->getAlphabet();
}

void LanguageMatcher::loadWords(WordSetIndex* const wsIndex) const
{
    // uppercase
    MakeUpper upper;

#ifdef CRUCIO_C_ARRAYS
    map<uint32_t, vector<string> > wordsets;

    // loads words into vector (a word each line)
    set<string>::const_iterator wIt;
    for (wIt = m_words->begin(); wIt != m_words->end(); ++wIt) {
        const string& word = *wIt;

        // checks word's length and format
        if (isValidWord(word)) {

            // makes an uppercase copy
            string upperWord = word;
            for_each(upperWord.begin(), upperWord.end(), upper);

            // put into same length set (create if non-existing)
            const size_t len = upperWord.length();
            map<uint32_t, vector<string> >::iterator refSet = wordsets.find((uint32_t)len);
            if (refSet == wordsets.end()) {
                refSet = wordsets.insert(make_pair(len, vector<string>())).first;
            }
            refSet->second.push_back(upperWord);
        }
    }

    // load word sets
    for (map<uint32_t, vector<string> >::iterator refSet = wordsets.begin(); refSet != wordsets.end(); ++refSet) {
        const uint32_t length = refSet->first;
        vector<string>& subwords = refSet->second;

        // never count on an already sorted-and-unique word list (in order to do binary search)
        sort(subwords.begin(), subwords.end());
        unique(subwords.begin(), subwords.end());

        // load into word set
        WordSet *ws = wsIndex->getWordSet(length);
        ws->load(subwords);

//        cout << "length " << length << " = " << subwords.size() << " words (" << ws->getSize() << " loaded)" << endl;
    }
#else
    // loads words
    set<string>::const_iterator wIt;
    for (wIt = words.begin(); wIt != words.end(); ++wIt) {
        const string& word = *wIt;

        // checks word's length and format
        if (isValidWord(word)) {

            // makes an uppercase copy
            string upperWord = word;
            for_each(upperWord.begin(), upperWord.end(), upper);

            // selects wordset for insertion
            WordSet* const ws = m_index->getWordSet(upperWord.length());
            ws->insert(upperWord);
        }
    }
#endif
}

void LanguageMatcher::loadFilename(WordSetIndex* const wsIndex) const
{
    // opens words list file
    ifstream wordsIn(m_filename.c_str());
    if (!wordsIn.is_open()) {
        throw DictionaryException("dictionary: unable to open words list");
    }

    // uppercase
    MakeUpper upper;

//    const time_t timeBegin = time(NULL);

#ifdef CRUCIO_C_ARRAYS
    map<uint32_t, size_t> wordCount;
    map<uint32_t, vector<string>* > wordSets;
    string word;

    // count words by length
    while (getline(wordsIn, word)) {

        // checks word's length and format
        if (isValidWord(word)) {
            const uint32_t len = (uint32_t)word.length();
            ++wordCount[len];
        }
    }

//    cout << "non-empty lenghts = " << wordCount.size() << endl;
//    map<uint32_t, size_t>::const_iterator wcIt;
//    for (wcIt = wordCount.begin(); wcIt != wordCount.end(); ++wcIt) {
//        cout << "predicted count for length " << wcIt->first << " = " << wcIt->second << " words" << endl;
//    }

    // reset file pointer
    wordsIn.clear();
    wordsIn.seekg(0, ios::beg);

    // loads words into vector (a word each line)
    while (getline(wordsIn, word)) {

        // checks word's length and format
        if (isValidWord(word)) {

            // IMPORTANT: make uppercase
            for_each(word.begin(), word.end(), upper);

            // put into same length set (create if non-existing)
            const uint32_t len = (uint32_t)word.length();
            map<uint32_t, vector<string>* >::iterator refSet = wordSets.find(len);
            if (refSet == wordSets.end()) {
                vector<string>* subwords = new vector<string>();
                subwords->reserve(wordCount[len]);

                refSet = wordSets.insert(make_pair(len, subwords)).first;
            }
            refSet->second->push_back(word);
        }
    }

    // closes file
    wordsIn.close();

    // load word sets
    map<uint32_t, vector<string>* >::iterator refSet;
    for (refSet = wordSets.begin(); refSet != wordSets.end(); ++refSet) {
        const uint32_t len = refSet->first;
        vector<string>* subwords = refSet->second;

        // never count on an already sorted-and-unique word list (in order to do binary search)
        sort(subwords->begin(), subwords->end());
        unique(subwords->begin(), subwords->end());

        // load into word set
        WordSet *ws = wsIndex->getWordSet(len);
        ws->load(*subwords);

//        cout << "length " << len << " = " << subwords->size() << " words (" << ws->getSize() << " loaded)" << endl;

        // release temporary vector immediately afterwards
        // reduces overall memory usage, there's no need to
        // keep all of them in memory at the same time
        delete subwords;
    }
#else
    vector<string> sortedWords;
    string word;

    // reserve enough space (for efficiency)
    const uint32_t reserved = count(istreambuf_iterator<char>(wordsIn), istreambuf_iterator<char>(), '\n');
    sortedWords.reserve(reserved);

    // reset file pointer
    wordsIn.clear();
    wordsIn.seekg(0, ios::beg);

    // loads words into vector (a word each line)
    while (getline(wordsIn, word)) {

        // checks word's length and format
        if (isValidWord(word)) {

            // IMPORTANT: make uppercase
            for_each(word.begin(), word.end(), upper);

#ifdef CRUCIO_BENCHMARK
            // loads word directly (assume word list is sorted and unique)
            WordSet* const ws = m_index->getWordSet(word.length());
            ws->insert(word);
#else
            // adds word to vector
            sortedWords.push_back(word);
#endif
        }
    }

    // closes file
    wordsIn.close();

#ifndef CRUCIO_BENCHMARK
    // never count on an already sorted-and-unique word list (in order to do binary search)
    sort(sortedWords.begin(), sortedWords.end());
    unique(sortedWords.begin(), sortedWords.end());

    // adds filtered words
    vector<string>::const_iterator wIt;
    for (wIt = sortedWords.begin(); wIt != sortedWords.end(); ++wIt) {
        const string& word = *wIt;

        // selects wordset for insertion
        WordSet* const ws = m_index->getWordSet(word.length());
        ws->insert(word);
    }
#endif
#endif

//    const time_t timeEnd = time(NULL);
//    const double timeElapsed = difftime(timeEnd, timeBegin);
//    cout << "dictionary loaded in " << timeElapsed << " seconds" << endl;
}

bool LanguageMatcher::getMatchings(WordSetIndex* const wsIndex,
                                   Word* const word)
{
    const string& pattern = word->get();
    MatchingResult* const res = word->getMatchings();
    const set<uint32_t>& exclusions = word->getExclusions();

    const uint32_t len = (uint32_t)pattern.length();

    // initially empty result
    res->clear();

    // single letters matching
    const WordSet* const ws = wsIndex->getWordSet(len);
#ifdef CRUCIO_C_ARRAYS
    list<const IDArray* > cpVectors;
#else
    list<const vector<uint32_t>* > cpVectors;
#endif
    for (uint32_t pi = 0; pi < len; ++pi) {
        if (pattern[pi] != ANY_CHAR) {
            cpVectors.push_back(ws->getCPVector(pi, pattern[pi]));
        }
    }

    // wild pattern, that is an ANY_CHAR-only pattern; this maps to whole
    // subdictionary
    if (cpVectors.empty()) {

        // IMPORTANT: very expensive, but only happens at model creation; this
        // is because compiler classes only call Word::doMatch() after
        // having ASSIGNED a letter/word, not when they are retired; so empty
        // masks are _never_ re-matched, apart at model creation time when
        // initial domains have to be evaluated
        const uint32_t wsSize = ws->getSize();
        res->reserve(wsSize);
        for (uint32_t id = 0; id < wsSize; ++id) {
            res->addID(id);
        }
        return true;
    }

    // intersects matchings; set intersection is at most large as smallest set,
    // so other sets are filtered on this one; smallest set search cost is a
    // good tradeoff for subsequent computation
#ifdef CRUCIO_C_ARRAYS
    list<const IDArray*>::const_iterator minSetIt;
    minSetIt = min_element(cpVectors.begin(), cpVectors.end(), MinSizePtr());
    const IDArray* const minSet = *minSetIt;

    // iterates over minSet and search for its elements in the other sets
    uint32_t idi;
    list<const IDArray*>::const_iterator setIt;
    for (idi = 0; idi < minSet->length; ++idi) {
        const uint32_t id = minSet->ids[idi];

        // skips excluded elements (if given)
        if (exclusions.find(id) != exclusions.end()) {
            continue;
        }

        // element must be found in every set
        bool foundAll = true;
        for (setIt = cpVectors.begin(); setIt != cpVectors.end(); ++setIt) {

            // skips minSet
            if (setIt == minSetIt) {
                continue;
            }

            const IDArray* const currSet = *setIt;

            // searches the element in this set (set is sorted since
            // WordSet::insert() assigns ascending IDs, so binary
            // search is MUCH better)
            //if (find(currSet->begin(), currSet->end(), id) ==
            //      currSet->end()) {
            if (!binary_search(currSet->ids, currSet->ids + currSet->length, id)) {
                foundAll = false;
                break;
            }
        }

        // if found in every set, element is added to intersection
        if (foundAll) {
            res->addID(id);
        }
    }
#else
    list<const vector<uint32_t>* >::const_iterator minSetIt;
    minSetIt = min_element(cpVectors.begin(), cpVectors.end(), MinSizePtr());
    const vector<uint32_t>* const minSet = *minSetIt;

    // iterates over minSet and search for its elements in the other sets
    vector<uint32_t>::const_iterator idIt;
    list<const vector<uint32_t>* >::const_iterator setIt;
    for (idIt = minSet->begin(); idIt != minSet->end(); ++idIt) {
        const uint32_t id = *idIt;

        // skips excluded elements (if given)
//        if (excluded && (excluded->find(id) != excluded->end())) {
        if ((excluded->find(id) != excluded->end())) {
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
#endif

    // true if intersection is not empty
    return !res->getIDs().empty();
}

//bool LanguageMatcher::getPossible(WordSetIndex* const wsIndex,
//                                  const MatchingResult* const res,
//                                  const uint32_t pos,
//                                  ABMask* const possible) const
//{
//    const uint32_t len = res->getWordsLength();
//
//    // initially empty letter mask
//    possible->reset();
//
//    // no matchings, empty mask is returned
//    if (res->isEmpty()) {
//        return false;
//    }
//
//    // current wordset
//    const WordSet* const ws = wsIndex->getWordSet(len);
//
//    // are matchings equal to whole subdictionary?
//    if (res->isFull()) {
//        ws->getPossibleAt(pos, possible);
//    } else {
//
//        // iterates over matching words IDs
//        const vector<uint32_t>& ids = res->getIDs();
//        vector<uint32_t>::const_iterator idIt;
//        for (idIt = ids.begin(); idIt != ids.end(); ++idIt) {
//            const uint32_t id = *idIt;
//#ifdef CRUCIO_C_ARRAYS
//            const char* word = ws->getWordPtr(id);
//#else
//            const string& word = ws->getWord(id);
//#endif
//
//            // letter index at position pos in the word
//            const uint32_t chIndex = letter2Index(word[pos]);
//
//            // puts letter into letter mask
//            possible->set(chIndex);
//        }
//    }
//
//    return true;
//}

bool LanguageMatcher::getPossible(WordSetIndex* const wsIndex,
                                  Word* const word)
{
    const MatchingResult* const res = word->getMatchings();
    vector<ABMask>& possibleVector = word->getAllowed();

    // fixed length for words in matching result
    const uint32_t len = res->getWordsLength();

    // character position in a word or pattern
    uint32_t pos;

    // initially empty letter masks
    for (pos = 0; pos < len; ++pos) {
        ABMask* const possible = &possibleVector[pos];
        possible->reset();
    }

    // no matchings, empty masks are returned
    if (res->isEmpty()) {
        return false;
    }

    // current wordset
    const WordSet* const ws = wsIndex->getWordSet(len);

    // are matchings equal to whole subdictionary?
    if (res->isFull()) {
        for (pos = 0; pos < len; ++pos) {
            ABMask* const possible = &possibleVector[pos];
            ws->getPossibleAt(pos, possible);
        }
    } else {

        // iterates over matching words IDs
        const vector<uint32_t>& ids = res->getIDs();
        vector<uint32_t>::const_iterator idIt;
        for (idIt = ids.begin(); idIt != ids.end(); ++idIt) {
            const uint32_t id = *idIt;
#ifdef CRUCIO_C_ARRAYS
            const char* word = ws->getWordPtr(id);
#else
            const string& word = ws->getWord(id);
#endif

            // finds possible letters at every position
            for (pos = 0; pos < len; ++pos) {
                ABMask* const possible = &possibleVector[pos];

                // letter index at position pos in the word
                const uint32_t chIndex = character2Index(m_alphabet, word[pos]);

                // puts letter into letter mask
                possible->set(chIndex);
            }
        }
    }

    return true;
}

/* unsupported methods */

uint32_t LanguageMatcher::addCustomWord(const string& word)
{
    return UINT_MAX;
}

const string& LanguageMatcher::getCustomWord(const uint32_t id) const
{
    static string dummy;

    return dummy;
}

uint32_t LanguageMatcher::getCustomWordID(const string& word) const
{
    return UINT_MAX;
}

uint32_t LanguageMatcher::removeCustomWordID(const uint32_t id)
{
    return UINT_MAX;
}
