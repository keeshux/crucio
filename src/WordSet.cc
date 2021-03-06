//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#include "WordSet.h"

using namespace crucio;
using namespace std;

/* WordSet */

#ifdef CRUCIO_C_ARRAYS

WordSet::WordSet(const Alphabet alphabet, const uint32_t len) :
    m_alphabet(alphabet),
    m_alphabetSize(alphabetSize(m_alphabet)),
    m_length(len),
    m_size(0),
    m_words(NULL),
    m_pointers(NULL),
    m_cpBuckets(m_length * m_alphabetSize),
    m_cpMatrix(NULL)
{
}

WordSet::~WordSet()
{
    if (m_words) {
        free(m_words);
        free(m_pointers);
        for (uint32_t i = 0; i < m_cpBuckets; ++i) {
            free(m_cpMatrix[i]);
        }
        free(m_cpMatrix);
    }
}

void WordSet::load(const vector<string>& words)
{
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
        cpArray->length = (uint32_t)length;

        // move to matrix
        m_cpMatrix[i] = cpArray;
    }

    // 3) copy and index words

    uint32_t wordID = 0;
    vector<size_t> cpCounters(m_cpBuckets);
    char *wordPtr = m_words;
    for (wIt = words.begin(); wIt != words.end(); ++wIt) {
        const char* cWord = wIt->c_str();
        const size_t len = strlen(cWord);

        // copy string and keep reference in pointer table
        strncpy(wordPtr, cWord, len);
        m_pointers[wordID] = wordPtr;

        // character position index
        for (uint32_t pos = 0; pos < len; ++pos) {

            // computes mapping (position=pos, letter=word[pos])
            const uint32_t bucket = getHash(pos, wordPtr[pos]);

            // appends word ID (ascending) to the bucket
            size_t& counter = cpCounters[bucket];
            m_cpMatrix[bucket]->ids[counter] = wordID;
            ++counter;
        }

        // advance
        wordPtr += len + 1;
        ++wordID;
    }

    // save words count (= last wordId + 1)
    m_size = wordID;
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

WordSet::WordSet(const Alphabet alphabet, const uint32_t len) :
    m_alphabet(alphabet),
    m_alphabetSize(alphabetSize(m_alphabet)),
    m_length(len),
    m_words(),
    m_cpMatrix(len * m_alphabetSize)
{
}

WordSet::~WordSet()
{
}

void WordSet::insert(const string& word)
{
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

/* WordSetIndex */

WordSetIndex::WordSetIndex(const Alphabet alphabet, const uint32_t minLength, const uint32_t maxLength) :
    m_alphabet(alphabet),
    m_minLength(minLength),
    m_maxLength(maxLength),
    m_wordSets(maxLength - minLength + 1)
{
    const uint32_t wsSize = (uint32_t)m_wordSets.size();
    for (uint32_t wsi = 0; wsi < wsSize; ++wsi) {
        m_wordSets[wsi] = new WordSet(m_alphabet, getReverseHash(wsi));
    }
}

WordSetIndex::~WordSetIndex()
{
    const uint32_t wsSize = (uint32_t)m_wordSets.size();
    for (uint32_t wsi = 0; wsi < wsSize; ++wsi) {
        delete m_wordSets[wsi];
    }
}

// computes size as wordsets sizes sum
uint32_t WordSetIndex::getSize() const
{
    uint32_t totalSize = 0;
    vector<WordSet*>::const_iterator wsIt;
    for (wsIt = m_wordSets.begin(); wsIt !=
            m_wordSets.end(); ++wsIt) {
        const WordSet* const ws = *wsIt;
        totalSize += ws->getSize();
    }
    return totalSize;
}
