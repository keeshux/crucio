//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#ifndef __DICTIONARY_H
#define __DICTIONARY_H

#include "common.h"
#include "WordSet.h"

namespace crucio
{
    class Word;
    class MatchingResult;

    /* Matcher: loading/matching implementation */

    class Matcher
    {
    public:
        virtual ~Matcher() {
        }

        // delegated index loading
        virtual void loadIndex(WordSetIndex* const wsIndex) = 0;

        // return words matching a pattern, excluding given IDs (optional)
        virtual bool getMatchings(WordSetIndex* const wsIndex,
                                  Word* const word) = 0;

        // return possible letters given a matching result
        virtual bool getPossible(WordSetIndex* const wsIndex,
                                 Word* const word) = 0;

        // dynamic custom entries
        virtual uint32_t addCustomWord(const std::string& word) = 0;
        virtual const std::string& getCustomWord(const uint32_t id) const = 0;
        virtual uint32_t getCustomWordID(const std::string& word) const = 0;
        virtual uint32_t removeCustomWordID(const uint32_t id) = 0;

    };

    /* Dictionary */

    class Dictionary
    {
    public:
        static const uint32_t MIN_LENGTH = 2;
        static const uint32_t MAX_LENGTH = 32;

        Dictionary(const Alphabet alphabet, Matcher* const matcher);
        ~Dictionary();

        // proxy for MatchingResult ctors/dctors
        MatchingResult* createMatchingResult(const uint32_t len) const;
        void destroyMatchingResult(MatchingResult* const res) const;

        const Alphabet getAlphabet() const {
            return m_index->getAlphabet();
        }
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

        const std::string getWord(const uint32_t len, const uint32_t id) const {
            const WordSet* const ws = m_index->getWordSet(len);
            return ws->getWord(id);
        }
        //const uint32_t getWordID(const string& word) const {
        //    const WordSet* const ws = m_index->getWordSet(word.length());
        //    return ws->getWordID(word);
        //}

        /* matcher delegation */

        void loadIndex() const {
            return m_matcher->loadIndex(m_index);
        }
        bool getMatchings(Word* const word) const {
            return m_matcher->getMatchings(m_index, word);
        }
        bool getPossible(Word* const word) const {
            return m_matcher->getPossible(m_index, word);
        }
        uint32_t addCustomWord(const std::string& word) {
            return m_matcher->addCustomWord(word);
        }
        const std::string& getCustomWord(const uint32_t id) const {
            return m_matcher->getCustomWord(id);
        }
        uint32_t getCustomWordID(const std::string& word) const {
            return m_matcher->getCustomWordID(word);
        }
        uint32_t removeCustomWordID(const uint32_t id) {
            return m_matcher->removeCustomWordID(id);
        }

    private:

        // laoding/matching algorithm
        Matcher *const m_matcher;

        // wordsets vector wrapper
        WordSetIndex* m_index;
    };

    class MatchingResult
    {
    public:
        friend class Dictionary;

        // fixed words length
        uint32_t getWordsLength() const {
            return m_wordsLength;
        }

        // results
        uint32_t getSize() const {
            return (uint32_t)m_IDs.size();
        }
        bool isEmpty() const {
            return m_IDs.empty();
        }
        bool isFull() const {
            return (m_IDs.size() == m_dictionary->getSize(m_wordsLength));
        }
        const std::vector<uint32_t>& getIDs() const {
            return m_IDs;
        }
        uint32_t getFirstID() const {
            if (m_IDs.empty()) {
                return UINT_MAX;
            }
            return *m_IDs.begin();
        }

        // results modification
        void clear() {
            m_IDs.clear();
        }
        void reserve(const uint32_t size) {
            m_IDs.reserve(size);
        }
        void addID(const uint32_t id) {
            m_IDs.push_back(id);
        }

        // external set operations
        void saveIDsUnion(std::set<uint32_t>* const dest) const {
            std::vector<uint32_t>::const_iterator idIt;

            for (idIt = m_IDs.begin(); idIt != m_IDs.end(); ++idIt) {
                dest->insert(*idIt);
            }
        }
        void saveIDsIntersection(std::set<uint32_t>* const dest,
                                 std::set<uint32_t>* const removed) const {

            std::set<uint32_t>::iterator sIDIt, sNextIDIt;
            for (sIDIt = dest->begin(); sIDIt != dest->end(); sIDIt = sNextIDIt) {
                sNextIDIt = sIDIt;
                ++sNextIDIt;

                // current set ID
                const uint32_t sID = *sIDIt;

                // removes and saves unshared values
                if (!std::binary_search(m_IDs.begin(), m_IDs.end(), sID)) {
                    dest->erase(sIDIt);
                    removed->insert(sID);
                }
            }
        }

        // dictionary proxy
        const std::string getWord(const uint32_t id) const {
            return m_dictionary->getWord(m_wordsLength, id);
        }

    private:
        const Dictionary* const m_dictionary;
        const uint32_t m_wordsLength;
        std::vector<uint32_t> m_IDs;

        MatchingResult(const Dictionary* const d, const uint32_t len) :
            m_dictionary(d),
            m_wordsLength(len),
            m_IDs() {
        }
    };
}

std::ostream& operator<<(std::ostream& out, const crucio::MatchingResult* const res);

#endif
