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

class IsNotAscii {
public:
    bool operator()(const char ch) const {
        const char upperCh = ch & ~32;
        return ((upperCh < 'A') || (upperCh > 'Z'));
    }
};

// checks for a word to be only-ASCII and in [MIN_LENGTH, MAX_LENGTH]
bool isValidWord(const std::string& word) {
    if ((word.length() < Dictionary::MIN_LENGTH) ||
        (word.length() > Dictionary::MAX_LENGTH)) {
        return false;
    }
    
    // true if no non-ASCII letters, that is (IsNotAscii() == false)
    // for each letter in the word
    return (std::find_if(word.begin(), word.end(), IsNotAscii()) == word.end());
}

/* WordSet */

#ifdef CRUCIO_C_ARRAYS

WordSet::WordSet(const uint32_t len) : m_length(len), m_size(0), m_words(NULL), m_pointers(NULL),
                                       m_cpBuckets(m_length * ALPHABET_SIZE), m_cpMatrix(NULL) {
}

WordSet::~WordSet() {
    if (m_words) {
        free(m_words);
        free(m_pointers);
        for (uint32_t i = 0; i < m_cpBuckets; ++i) {
            free(m_cpMatrix[i]);
        }
        free(m_cpMatrix);
    }
}

void WordSet::load(const vector<string>& words) {

    if (m_words) {
        free(m_words);
        free(m_pointers);
        for (uint32_t i = 0; i < m_cpBuckets; ++i) {
            free(m_cpMatrix[i]);
        }
        free(m_cpMatrix);
        m_size = 0;
        m_words = NULL;
        m_pointers = NULL;
        m_cpMatrix = NULL;
    }

    // 1) check words length and compute needed memory

    size_t wordsBytes = 0;
    vector<size_t> cpEntries(m_cpBuckets);
    vector<string>::const_iterator wIt;
    for (wIt = words.begin(); wIt != words.end(); ++wIt) {
        const string& word = *wIt;
        const size_t len = word.length();

        // words MUST have fixed length
        if (len != m_length) {
            throw DictionaryException("WordSet: invalid word length");
        }

        // string length (add \0)
        wordsBytes += len + 1;

        // character-position matrix lengths
        for (uint32_t pos = 0; pos < len; ++pos) {
            const uint32_t bucket = getHash(pos, word[pos]);

            ++cpEntries[bucket];
        }
    }

    // 2) allocate

    // calloc defaults to \0, no need to append manually at the end of the word
    m_words = (char*) calloc(wordsBytes, sizeof(char));

    // word id -> char pointer in bitmap
    m_pointers = (const char**) calloc(words.size(), sizeof(const char*));

    // dynamic IDArray structs
    m_cpMatrix = (IDArray**) calloc(m_cpBuckets, sizeof(IDArray*));
    for (uint32_t i = 0; i < m_cpBuckets; ++i) {
        const size_t length = cpEntries[i];
        IDArray* cpArray = (IDArray *) malloc(sizeof(IDArray) + length * sizeof(uint32_t));
        cpArray->length = length;

        // move to matrix
        m_cpMatrix[i] = cpArray;
    }

    // 3) copy and index words

    uint32_t wordId = 0;
    vector<size_t> cpCounters(m_cpBuckets);
    char *wordPtr = m_words;
    for (wIt = words.begin(); wIt != words.end(); ++wIt) {
        const char* cWord = wIt->c_str();
        const size_t len = strlen(cWord);

        // copy string and keep reference in pointer table
        strncpy(wordPtr, cWord, len);
        m_pointers[wordId] = wordPtr;

        // character position index
        for (uint32_t pos = 0; pos < len; ++pos) {

            // computes mapping (position=pos, letter=word[pos])
            const uint32_t bucket = getHash(pos, wordPtr[pos]);

            // appends word ID (ascending) to the bucket
            size_t& counter = cpCounters[bucket];
            m_cpMatrix[bucket]->ids[counter] = wordId;
            ++counter;
        }

        // advance
        wordPtr += len + 1;
        ++wordId;
    }

    // save words count (= last wordId + 1)
    m_size = wordId;
//    for (uint32_t i = 0; i < m_size; ++i) {
//        cout << ">>> " << i << " = " << m_pointers[i] << endl;
//    }

//    if (m_length == 7) {
//        cout << "words having R as 3rd letter" << endl;
//        const uint32_t b = getHash(2, 'R');
//        const IDArray *arr = m_cpMatrix[b];
//        cout << "bucket = " << b << endl;
//        for (int j = 0; j < arr->length; ++j) {
//            cout << "\t" << m_pointers[arr->ids[j]] << endl;
//        }
//    }
}

#else

WordSet::WordSet(const uint32_t len) : m_words(), m_length(len), m_cpMatrix(len * ALPHABET_SIZE) {
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

#endif

/* Dictionary */

// wildcard (any characater)
const char Dictionary::ANY_CHAR = '-';

// all ones 26-bit mask (any [A-Z] letter)
const ABMask Dictionary::ANY_MASK = ABMask(0x03FFFFFF);

Dictionary::Dictionary(const set<string>& words) : m_filename(), m_index(MIN_LENGTH, MAX_LENGTH) {

    // uppercase
    MakeUpper upper;

#ifdef CRUCIO_C_ARRAYS
    map<uint32_t, vector<string> > wordsets;
    
    // loads words into vector (a word each line)
    set<string>::const_iterator wIt;
    for (wIt = words.begin(); wIt != words.end(); ++wIt) {
        const string& word = *wIt;

        // checks word's length and format
        if (isValidWord(word)) {
            
            // makes an uppercase copy
            string upperWord = word;
            for_each(upperWord.begin(), upperWord.end(), upper);
            
            // put into same length set (create if non-existing)
            const size_t len = upperWord.length();
            map<uint32_t, vector<string> >::iterator refSet = wordsets.find(upperWord.length());
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
        WordSet *ws = m_index.getWordSet(length);
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
            WordSet* const ws = m_index.getWordSet(upperWord.length());
            ws->insert(upperWord);
        }
    }
#endif
}

Dictionary::Dictionary(const string& filename) : m_filename(filename), m_index(MIN_LENGTH, MAX_LENGTH) {

    // opens words list file
    ifstream wordsIn(filename.c_str());
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
            const size_t len = word.length();
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
            const size_t len = word.length();
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
        WordSet *ws = m_index.getWordSet(len);
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
            WordSet* const ws = m_index.getWordSet(word.length());
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
        WordSet* const ws = m_index.getWordSet(word.length());
        ws->insert(word);
    }
#endif
#endif

//    const time_t timeEnd = time(NULL);
//    const double timeElapsed = difftime(timeEnd, timeBegin);
//    cout << "dictionary loaded in " << timeElapsed << " seconds" << endl;
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

#ifdef CRUCIO_C_ARRAYS
    // intersects matchings; set intersection is at most large as smallest set,
    // so other sets are filtered on this one; smallest set search cost is a
    // good tradeoff for subsequent computation
    list<const IDArray*>::const_iterator minSetIt;
    minSetIt = min_element(cpVectors.begin(), cpVectors.end(), MinSizePtr());
    const IDArray* const minSet = *minSetIt;

    // iterates over minSet and search for its elements in the other sets
    uint32_t idi;
    list<const IDArray*>::const_iterator setIt;
    for (idi = 0; idi < minSet->length; ++idi) {
        const uint32_t id = minSet->ids[idi];

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
            res->m_ids.push_back(id);
        }
    }
#else
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
#endif

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
#ifdef CRUCIO_C_ARRAYS
            const char* word = ws->getWordPtr(id);
#else
            const string& word = ws->getWord(id);
#endif

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
#ifdef CRUCIO_C_ARRAYS
            const char* word = ws->getWordPtr(id);
#else
            const string& word = ws->getWord(id);
#endif

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
