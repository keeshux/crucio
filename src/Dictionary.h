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
    
    const uint32_t LETTERS_COUNT    = 26;
    const uint32_t DIGITS_COUNT     = 10;
    
    typedef std::bitset<LETTERS_COUNT> ABMask;
    
    typedef struct {
        uint32_t length;
        uint32_t ids[1];
    } IDArray;
    
    // maps i to i-th letter of alphabet (0 -> 'A', 1 -> 'B', ...)
    inline char index2Letter(const uint32_t i) {
        return (char)('A' + i);
    }
    
    // maps ch to its alphabet index ('A' -> 0, 'B' -> 1, ...)
    inline uint32_t letter2Index(const char ch) {
        return (ch - 'A');
    }
    
    // maps i to i-th numeric character (0 -> '0', 1 -> '1', ...)
    inline char index2Number(const uint32_t i) {
        return (char)('0' + i);
    }
    
    // maps ch to its numeric value ('0' -> 0, '1' -> 1, ...)
    inline uint32_t number2Index(const char ch) {
        return (ch - '0');
    }
    
    /* abstract */
    
    class MatchingResult;
    
    class Dictionary {
    public:
        static const char ANY_CHAR;
        static const ABMask ANY_MASK;
        
        static const uint32_t MIN_LENGTH = 2;
        static const uint32_t MAX_LENGTH = 32;
        
        virtual ~Dictionary() {
        }

        // proxy for MatchingResult ctors/dctors
        MatchingResult* createMatchingResult(const uint32_t) const;
        void destroyMatchingResult(MatchingResult* const) const;
        
        virtual uint32_t getSize() const = 0;
        virtual uint32_t getSize(const uint32_t) const = 0;
        
        virtual const std::string getWord(const uint32_t, const uint32_t) const = 0;

        // returns words matching a pattern, excluding given IDs (optional)
        virtual bool getMatchings(const std::string&, MatchingResult* const,
                                  const std::set<uint32_t>* const = 0) const = 0;
        
        // return possible letter at position pos given a matching result
        virtual bool getPossible(const MatchingResult* const, const uint32_t,
                                 ABMask* const) const = 0;
        
        // return possible letters given a matching result
        virtual bool getPossible(const MatchingResult* const,
                                 std::vector<ABMask>* const) const = 0;
    };
    
    class MatchingResult {
    public:
        MatchingResult(const Dictionary* const d, const uint32_t len) :
                m_dictionary(d),
                m_wordsLength(len),
                m_ids() {
        }
        
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
        std::vector<uint32_t>& getIds() {
            return m_ids;
        }
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
        
        // first matching (NOTE: must be !isEmpty())
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
    };
}

std::ostream& operator<<(std::ostream&, const crucio::ABMask&);
std::ostream& operator<<(std::ostream&, const crucio::MatchingResult* const);

#endif
