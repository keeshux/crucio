//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#ifndef __WORD_SET_H
#define __WORD_SET_H

#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <numeric>
#include <set>
#include <map>
#include <string>
#include <vector>

#include "common.h"

namespace crucio
{
    typedef struct {
        uint32_t length;
        uint32_t ids[1];
    } IDArray;

    /* utils */

#ifdef CRUCIO_C_ARRAYS
// wordset of fixed length
    class WordSet
    {
    public:
        WordSet(const Alphabet alphabet, const uint32_t len);
        ~WordSet();

        // load a words array (must be uppercase)
        void load(const std::vector<std::string>& words);

//        bool contains(const std::string& word) const {
//            return std::binary_search(m_words.begin(),
//                                      m_words.end(),
//                                      word);
//        }
        uint32_t getSize() const {
            return m_size;
        }

        // wordset key is the offset within m_words
        const std::string getWord(const uint32_t id) const {
            return m_pointers[id];
        }
        const char* getWordPtr(const uint32_t id) const {
            return m_pointers[id];
        }

//        // finds word offset within wordset
//        const uint32_t getWordID(const string& word) const {
//            const vector<string>::const_iterator wIt =
//            lower_bound(m_words.begin(),
//                        m_words.end(),
//                        word);
//
//            // found?
//            if (word >= *wIt) {
//                return distance(m_words.begin(), wIt);
//            } else {
//
//                // same as distance(m_words.begin(), m_words.end())
//                return m_words.size();
//            }
//        }

        // vector of words (offsets) containing ch at position pos
        const IDArray* getCPVector(const uint32_t pos, const char ch) const {
            return m_cpMatrix[getHash(pos, ch)];
        }

        // possible letters at position pos
        void getPossibleAt(const uint32_t pos, ABMask* const possible) const {

            // initially empty letter mask
            possible->reset();

            // adds all characters that appear at position pos
            const uint32_t cpStart = getHash(pos, m_alphabet);
            for (uint32_t i = 0; i < m_alphabetSize; ++i) {
                if (m_cpMatrix[cpStart + i]->length > 0) {
                    possible->set(i);
                }
            }
        }

    private:

        // fixed alphabet and word length
        const Alphabet m_alphabet;
        const uint32_t m_alphabetSize;
        const uint32_t m_length;

        // words bitmap
        uint32_t m_size;
        char* m_words;

        // word id -> location in m_words
        const char** m_pointers;

        // the (<position, letter> -> array of word offsets) table
        const uint32_t m_cpBuckets;
        IDArray** m_cpMatrix;

        // hash function for m_cpMatrix buckets addressing
        uint32_t getHash(const uint32_t pos, const char ch) const {
            return (pos * m_alphabetSize + character2Index(m_alphabet, ch));
        }
    };
#else
// wordset of fixed length
    class WordSet
    {
    public:
        WordSet(const Alphabet alphabet, const uint32_t len);
        ~WordSet();

        // inserts a word (must be uppercase)
        void insert(const std::string& word);

//        bool contains(const std::string& word) const {
//            return binary_search(m_words.begin(),
//                    m_words.end(), word);
//        }
        uint32_t getSize() const {
            return m_words.size();
        }

        // wordset key is the offset within m_words
        const string& getWord(const uint32_t id) const {
            return m_words[id];
        }

//        // finds word offset within wordset
//        const uint32_t getWordID(const std::string& word) const {
//            const std::vector<std::string>::const_iterator wIt =
//                    std::lower_bound(m_words.begin(),
//                                     m_words.end(),
//                                     word);
//
//            // found?
//            if (word >= *wIt) {
//                return std::distance(m_words.begin(), wIt);
//            } else {
//
//                // same as distance(m_words.begin(), m_words.end())
//                return m_words.size();
//            }
//        }

        // vector of words (offsets) containing ch at position pos
        const std::vector<uint32_t>* getCPVector(const uint32_t pos,
                const char ch) const {

            return &m_cpMatrix[getHash(pos, ch)];
        }

        // possible letters at position pos
        void getPossibleAt(const uint32_t pos, ABMask* const possible) const {

            // initially empty letter mask
            possible->reset();

            // adds all characters that appear at position pos
            const uint32_t cpStart = getHash(pos, m_alphabet);
            for (uint32_t i = 0; i < m_alphabetSize; ++i) {
                if (!m_cpMatrix[cpStart + i].empty()) {
                    possible->set(i);
                }
            }
        }

    private:

        // fixed alphabet and word length, words vector
        const Alphabet m_alphabet;
        const uint32_t m_alphabetSize;
        const uint32_t m_length;
        std::vector<std::string> m_words;

        // the (<position, letter> -> words offsets) hash table
        std::vector<std::vector<uint32_t> > m_cpMatrix;

        // hash function for m_cpMatrix buckets addressing
        uint32_t getHash(const uint32_t pos, const char ch) const {
            return (pos * m_alphabetSize + reverseAlphabet(ch));
        }
    };
#endif

// encloses a set of wordsets whose word length parameter falls
// within [m_minLength, m_maxLength]
    class WordSetIndex
    {
    public:
        WordSetIndex(const Alphabet alphabet, const uint32_t minLength, const uint32_t maxLength);
        ~WordSetIndex();

        const Alphabet getAlphabet() const {
            return m_alphabet;
        }

        // computes size as wordsets sizes sum
        uint32_t getSize() const;

        const WordSet* getWordSet(const uint32_t len) const {
            return m_wordSets[getHash(len)];
        }
        WordSet* getWordSet(const uint32_t len) {
            return m_wordSets[getHash(len)];
        }

    private:

        // min and max length for a word
        const Alphabet m_alphabet;
        const uint32_t m_minLength;
        const uint32_t m_maxLength;

        // wordsets vector
        std::vector<WordSet*> m_wordSets;

        // hash functions to map (word length -> wordset)
        // and (wordset index -> word length)
        uint32_t getHash(const uint32_t len) const {
            return (len - m_minLength);
        }
        uint32_t getReverseHash(const uint32_t i) const {
            return (i + m_minLength);
        }
    };
}

#endif
