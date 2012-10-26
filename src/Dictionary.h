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

#include "common.h"
#include "WordSet.h"

namespace crucio {

    class MatchingResult;

    /* Matcher: loading/matching implementation */

    class Matcher {
    public:
        virtual ~Matcher() {
        }

        // delegated index loading
        virtual void loadIndex(WordSetIndex* const wsIndex) const = 0;

        // return words matching a pattern, excluding given IDs (optional)
        virtual bool getMatchings(WordSetIndex* const wsIndex,
                                  const std::string& pattern,
                                  MatchingResult* const res,
                                  const std::set<uint32_t>* const excluded = 0) const = 0;

        // return possible letter at position pos given a matching result
        virtual bool getPossible(WordSetIndex* const wsIndex,
                                 const MatchingResult* const res,
                                 const uint32_t pos,
                                 ABMask* const possible) const = 0;

        // return possible letters given a matching result
        virtual bool getPossible(WordSetIndex* const wsIndex,
                                 const MatchingResult* const res,
                                 std::vector<ABMask>* const possibleVector) const = 0;

    };

    /* Dictionary */

    class Dictionary {
    public:
        static const char ANY_CHAR;
        static const ABMask ANY_MASK;

        static const uint32_t MIN_LENGTH = 2;
        static const uint32_t MAX_LENGTH = 32;

        Dictionary(const Matcher* const matcher);
        ~Dictionary();

        // proxy for MatchingResult ctors/dctors
        MatchingResult* createMatchingResult(const uint32_t len) const;
        void destroyMatchingResult(MatchingResult* const res) const;

        uint32_t getSize() const {
            return m_index->getSize();
        }
        uint32_t getSize(const uint32_t len) const {
            const WordSet* const ws = m_index->getWordSet(len);
            return ws->getSize();
        }

        //bool contains(const string& word) const {
        //    const WordSet* const ws = m_index->getWordSet(word.length());
        //    return ws->contains(word);
        //}

        // maps to WordSet::getWord(id)
        const std::string getWord(const uint32_t len, const uint32_t id) const {
            const WordSet* const ws = m_index->getWordSet(len);
            return ws->getWord(id);
        }

        //// maps to WordSet::getWordId(word)
        //const uint32_t getWordId(const string& word) const {
        //    const WordSet* const ws = m_index->getWordSet(word.length());
        //    return ws->getWordId(word);
        //}

        /* matcher delegation */

        void loadIndex() const {
            return m_matcher->loadIndex(m_index);
        }
        bool getMatchings(const std::string& pattern,
                          MatchingResult* const res,
                          const std::set<uint32_t>* const excluded = 0) const {

            return m_matcher->getMatchings(m_index, pattern, res);
        }
        bool getPossible(const MatchingResult* const res,
                         const uint32_t pos,
                         ABMask* const possible) const {

            return m_matcher->getPossible(m_index, res, pos, possible);
        }
        bool getPossible(const MatchingResult* const res,
                         std::vector<ABMask>* const possibleVector) const {

            return m_matcher->getPossible(m_index, res, possibleVector);
        }

    private:

        // laoding/matching algorithm
        const Matcher *const m_matcher;

        // wordsets vector wrapper
        WordSetIndex* m_index;
    };

    class MatchingResult {
    public:
        MatchingResult(const Dictionary* const d, const uint32_t len) :
            m_dictionary(d),
            m_wordsLength(len),
            m_ids() {
        }

//        // dictionary reference
//        const Dictionary* getDictionary() const {
//            return m_dictionary;
//        }

        // matching words length
        uint32_t getWordsLength() const {
            return m_wordsLength;
        }

        // true if no matchings
        bool isEmpty() const {
            return m_ids.empty();
        }

        // true if whole subdictionary
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

std::ostream& operator<<(std::ostream& out, const crucio::MatchingResult* const res);

#endif
