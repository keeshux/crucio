/*
 * Dictionary.h
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

#ifndef __DICTIONARY_H
#define __DICTIONARY_H

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

namespace crucio {

    /* global alphabet management (IMPORTANT: only uppercase letters!) */

    const uint32_t ALPHABET_SIZE = 26;

    typedef std::bitset<ALPHABET_SIZE> ABMask;

    typedef struct {
        uint32_t length;
        uint32_t ids[1];
    } IDArray;

    // maps i to i-th letter of alphabet (0 -> 'A', 1 -> 'B', ...)
    inline char alphabet(const uint32_t i) {
        return (char)('A' + i);
    }

    // maps ch to its alphabet index ('A' -> 0, 'B' -> 1, ...)
    inline uint32_t reverseAlphabet(const char ch) {
        return (ch - 'A');
    }

    /* dictionary implementation */

#ifdef CRUCIO_C_ARRAYS
    // wordset of fixed length
    class WordSet {
    public:
        WordSet(const uint32_t);
        ~WordSet();

        // load a words array (must be uppercase)
        void load(std::vector<std::string>&);
        
//        bool contains(const std::string& word) const {
//            return std::binary_search(m_words.begin(),
//                                      m_words.end(), word);
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
//        const uint32_t getWordId(const std::string& word) const {
//            const std::vector<std::string>::const_iterator wIt =
//            std::lower_bound(m_words.begin(),
//                             m_words.end(), word);
//            
//            // found?
//            if (word >= *wIt) {
//                return std::distance(m_words.begin(), wIt);
//            } else {
//                
//                // same as std::distance(m_words.begin(), m_words.end())
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
            for (uint32_t i = 0; i < ALPHABET_SIZE; ++i) {
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
            return (pos * ALPHABET_SIZE + reverseAlphabet(ch));
        }
    };
#else
    // wordset of fixed length
    class WordSet {
    public:
        WordSet(const uint32_t);

        // inserts a word (must be uppercase)
        void insert(const std::string&);

//        bool contains(const std::string& word) const {
//            return std::binary_search(m_words.begin(),
//                    m_words.end(), word);
//        }
        uint32_t getSize() const {
            return m_words.size();
        }

        // wordset key is the offset within m_words
        const std::string& getWord(const uint32_t id) const {
            return m_words[id];
        }

//        // finds word offset within wordset
//        const uint32_t getWordId(const std::string& word) const {
//            const std::vector<std::string>::const_iterator wIt =
//                    std::lower_bound(m_words.begin(),
//                    m_words.end(), word);
//
//            // found?
//            if (word >= *wIt) {
//                return std::distance(m_words.begin(), wIt);
//            } else {
//
//                // same as std::distance(m_words.begin(), m_words.end())
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
            const uint32_t cpStart = getHash(pos, 'A');
            for (uint32_t i = 0; i < ALPHABET_SIZE; ++i) {
                if (!m_cpMatrix[cpStart + i].empty()) {
                    possible->set(i);
                }
            }
        }

    private:

        // words vector and fixed words length
        const uint32_t m_length;
        std::vector<std::string> m_words;

        // the (<position, letter> -> words offsets) hash table
        std::vector<std::vector<uint32_t> > m_cpMatrix;

        // hash function for m_cpMatrix buckets addressing
        uint32_t getHash(const uint32_t pos, const char ch) const {
            return (pos * ALPHABET_SIZE + reverseAlphabet(ch));
        }
    };
#endif

    // encloses a set of wordsets whose word length parameter falls
    // within [m_minLength, m_maxLength]
    class WordSetIndex {
    public:
        WordSetIndex(const uint32_t minLength, const uint32_t maxLength) :
                m_minLength(minLength),
                m_maxLength(maxLength),
                m_wordSets(maxLength - minLength + 1) {
            const uint32_t wsSize = m_wordSets.size();
            for (uint32_t wsi = 0; wsi < wsSize; ++wsi) {
                m_wordSets[wsi] = new WordSet(getReverseHash(wsi));
            }
        }
        ~WordSetIndex() {
            const uint32_t wsSize = m_wordSets.size();
            for (uint32_t wsi = 0; wsi < wsSize; ++wsi) {
                delete m_wordSets[wsi];
            }
        }

        // computes size as wordsets sizes sum
        uint32_t getSize() const {
            uint32_t totalSize = 0;
            std::vector<WordSet*>::const_iterator wsIt;
            for (wsIt = m_wordSets.begin(); wsIt !=
                    m_wordSets.end(); ++wsIt) {
                const WordSet* const ws = *wsIt;
                totalSize += ws->getSize();
            }
            return totalSize;
        }

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

    class Dictionary {
    public:
        static const uint32_t MIN_LENGTH = 2;
        static const uint32_t MAX_LENGTH = 32;
        
        static const char ANY_CHAR;
        static const ABMask ANY_MASK;
        
        class MatchingResult {
        public:
            friend class Dictionary;

            // dictionary reference
            const Dictionary* getDictionary() const {
                return m_dictionary;
            }

            // matching words length
            uint32_t getWordsLength() const {
                return m_wordsLength;
            }

            // true if no matchings
            bool isEmpty() const {
                return m_ids.empty();
            }

            // true if whole dictionary
            bool isFull() const {
                return (m_ids.size() == m_dictionary->getSize(m_wordsLength));
            }

            // matchings size
            uint32_t getSize() const {
                return m_ids.size();
            }

            // matching IDs vector
            const std::vector<uint32_t>& getIds() const {
                return m_ids;
            }

            // vector to set
            void getIdsUnion(std::set<uint32_t>* const s) const {
                std::vector<uint32_t>::const_iterator idIt;
                for (idIt = m_ids.begin(); idIt != m_ids.end(); ++idIt) {
                    s->insert(*idIt);
                }
            }
            void getIdsIntersection(std::set<uint32_t>* const s,
                    std::set<uint32_t>* const removed) const {
                std::set<uint32_t>::iterator sIdIt, sNextIdIt;
                for (sIdIt = s->begin(); sIdIt != s->end(); sIdIt = sNextIdIt) {
                    sNextIdIt = sIdIt;
                    ++sNextIdIt;

                    // current set ID
                    const uint32_t sId = *sIdIt;

                    // removes and saves unshared values
                    if (!std::binary_search(m_ids.begin(), m_ids.end(), sId)) {
                        s->erase(sIdIt);
                        removed->insert(sId);
                    }
                }
            }

            // maps to Dictionary::getWord
            const std::string getWord(const uint32_t id) const {
                return m_dictionary->getWord(m_wordsLength, id);
            }

            // first matching (NOTE: must be !empty())
            uint32_t getFirstWordId() const {
                return *m_ids.begin();
            }
            const std::string getFirstWord() const {
                return m_dictionary->getWord(m_wordsLength, getFirstWordId());
            }

        private:
            const Dictionary* const m_dictionary;
            const uint32_t m_wordsLength;
            std::vector<uint32_t> m_ids;

            MatchingResult(const Dictionary* const d, const uint32_t len) :
                    m_dictionary(d),
                    m_wordsLength(len),
                    m_ids() {
            }
        };

        Dictionary(const std::set<std::string>&);
        Dictionary(const std::string&);
        ~Dictionary();

        const std::string& getFilename() const {
            return m_filename;
        }

//        bool contains(const std::string& word) const {
//            const WordSet* const ws = m_index.getWordSet(word.length());
//            return ws->contains(word);
//        }
        uint32_t getSize() const {
            return m_index.getSize();
        }
        uint32_t getSize(const uint32_t len) const {
            const WordSet* const ws = m_index.getWordSet(len);
            return ws->getSize();
        }

        // maps to WordSet::getWord(id)
        const std::string getWord(const uint32_t len, const uint32_t id) const {
            const WordSet* const ws = m_index.getWordSet(len);
            return ws->getWord(id);
        }

//        // maps to WordSet::getWordId(word)
//        const uint32_t getWordId(const std::string& word) const {
//            const WordSet* const ws = m_index.getWordSet(word.length());
//            return ws->getWordId(word);
//        }

        // wrappers for MatchingResult ctors/dctors
        MatchingResult* createMatchingResult(const uint32_t len) const {
            return new MatchingResult(this, len);
        }
        void destroyMatchingResult(MatchingResult* const res) const {
            delete res;
        }

        // returns words matching a pattern, excluding given IDs (optional)
        bool getMatchings(const std::string&, MatchingResult* const,
                const std::set<uint32_t>* const = 0) const;

        // return possible letter at position pos given a matching result
        bool getPossible(const MatchingResult* const, const uint32_t,
                ABMask* const) const;

        // return possible letters given a matching result
        bool getPossible(const MatchingResult* const,
                std::vector<ABMask>* const) const;

    private:
        class MakeUpper {
        public:
            void operator()(char& ch) const {
                ch &= ~32;
            }
        };

#ifdef CRUCIO_C_ARRAYS
        class MinSizePtr {
        public:
            bool operator()(const IDArray* const v1, const IDArray* const v2) const {
                return (v1->length < v2->length);
            }
        };
#else
        class MinSizePtr {
        public:
            bool operator()(const std::vector<uint32_t>* const v1,
                    const std::vector<uint32_t>* const v2) const {
                return (v1->size() < v2->size());
            }
        };
#endif

        // origin filename (if any)
        const std::string m_filename;

        // wordsets vector wrapper
        WordSetIndex m_index;
    };
}

std::ostream& operator<<(std::ostream&, const crucio::ABMask&);
std::ostream& operator<<(std::ostream&,
        const crucio::Dictionary::MatchingResult* const);

#endif
