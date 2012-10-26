/*
 * WordSet.h
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

#ifndef __WORD_SET_H
#define __WORD_SET_H

#include <algorithm>
#include <bitset>
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
    /* global alphabet management (IMPORTANT: only uppercase letters!) */

    const uint32_t LETTERS_COUNT    = 26;
    const uint32_t DIGITS_COUNT     = 10;

    typedef std::bitset<LETTERS_COUNT> ABMask;

    typedef struct {
        uint32_t length;
        uint32_t ids[1];
    } IDArray;

// maps i to i-th letter of alphabet (0 -> 'A', 1 -> 'B', ...)
    inline char index2Letter(const uint32_t i)
    {
        return (char)('A' + i);
    }

// maps ch to its alphabet index ('A' -> 0, 'B' -> 1, ...)
    inline uint32_t letter2Index(const char ch)
    {
        return (ch - 'A');
    }

// maps i to i-th numeric character (0 -> '0', 1 -> '1', ...)
    inline char index2Number(const uint32_t i)
    {
        return (char)('0' + i);
    }

// maps ch to its numeric value ('0' -> 0, '1' -> 1, ...)
    inline uint32_t number2Index(const char ch)
    {
        return (ch - '0');
    }

    /* utils */

#ifdef CRUCIO_C_ARRAYS
// wordset of fixed length
    class WordSet
    {
    public:
        WordSet(const uint32_t len);
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
//        const uint32_t getWordId(const string& word) const {
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
            const uint32_t cpStart = getHash(pos, 'A');
            for (uint32_t i = 0; i < LETTERS_COUNT; ++i) {
                if (m_cpMatrix[cpStart + i]->length > 0) {
                    possible->set(i);
                }
            }
        }

    private:

        // fixed word length
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
            return (pos * LETTERS_COUNT + letter2Index(ch));
        }
    };
#else
// wordset of fixed length
    class WordSet
    {
    public:
        WordSet(const uint32_t len);

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
//        const uint32_t getWordId(const std::string& word) const {
//            const std::vector<std::string>::const_iterator wIt =
//                    std::lower_bound(m_words.begin(),
//                                     m_words.end(),
//                                     word);
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
        const vector<uint32_t>* getCPVector(const uint32_t pos,
                                            const char ch) const {

            return &m_cpMatrix[getHash(pos, ch)];
        }

        // possible letters at position pos
        void getPossibleAt(const uint32_t pos, ABMask* const possible) const {

            // initially empty letter mask
            possible->reset();

            // adds all characters that appear at position pos
            const uint32_t cpStart = getHash(pos, 'A');
            for (uint32_t i = 0; i < LETTERS_COUNT; ++i) {
                if (!m_cpMatrix[cpStart + i].empty()) {
                    possible->set(i);
                }
            }
        }

    private:

        // words vector and fixed words length
        const uint32_t m_length;
        vector<string> m_words;

        // the (<position, letter> -> words offsets) hash table
        vector<vector<uint32_t> > m_cpMatrix;

        // hash function for m_cpMatrix buckets addressing
        uint32_t getHash(const uint32_t pos, const char ch) const {
            return (pos * LETTERS_COUNT + reverseAlphabet(ch));
        }
    };
#endif

// encloses a set of wordsets whose word length parameter falls
// within [m_minLength, m_maxLength]
    class WordSetIndex
    {
    public:
        WordSetIndex(const uint32_t minLength, const uint32_t maxLength);
        ~WordSetIndex();

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

std::ostream& operator<<(std::ostream& out, const crucio::ABMask& m);

#endif
