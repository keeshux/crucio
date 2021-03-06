//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#ifndef __WORD_H
#define __WORD_H

#include <set>
#include <string>
#include <vector>

#include "Dictionary.h"
#include "Grid.h"

namespace crucio
{
    class Word
    {
    public:
        Word(Dictionary* const dict, const Definition* defRef) :
            m_dictionary(dict),
            m_defRef(defRef),
            m_mask(defRef->getLength(), ANY_CHAR),
            m_wildcards(defRef->getLength()),
            m_letterMasks(defRef->getLength(), anyMask(dict->getAlphabet())),
            m_matchings(dict->createMatchingResult(defRef->getLength())),
            m_excluded() {
        }
        ~Word() {
            m_dictionary->destroyMatchingResult(m_matchings);
        }

        // referred definition
        const Definition* getDefinition() const {
            return m_defRef;
        }
        uint32_t getLength() const {
            return m_defRef->getLength();
        }

        // matching results
        const MatchingResult* getMatchings() const {
            return m_matchings;
        }
        MatchingResult* getMatchings() {
            return m_matchings;
        }

        // assignment
        const std::string& get() const {
            return m_mask;
        }
        void set(const std::string& mask) {
            m_mask = mask;
            m_wildcards = (uint32_t)count(m_mask.begin(), m_mask.end(),
                                          ANY_CHAR);
        }
        void unset() {
            std::fill(m_mask.begin(), m_mask.end(), ANY_CHAR);
            m_wildcards = (uint32_t)m_mask.length();
        }

        // pattern I/O
        char getAt(const uint32_t i) const {
            return m_mask[i];
        }
        void setAt(const uint32_t i, const char ch) {
            assert(ch != ANY_CHAR);
            if (m_mask[i] == ANY_CHAR) {
                --m_wildcards;
            }
            m_mask[i] = ch;
        }
        void unsetAt(const uint32_t i) {
            if (m_mask[i] != ANY_CHAR) {
                ++m_wildcards;
            }
            m_mask[i] = ANY_CHAR;
        }

        // rematches pattern, updates matching result
        void doMatch() {
            m_dictionary->getMatchings(this);
        }

        // rematches pattern, updates matching result and letter masks
        void doMatchUpdating() {
            m_dictionary->getMatchings(this);

            // updates letters masks
            m_dictionary->getPossible(this);
        }

        // word id in dictionary (WARNING: only after matching a complete mask!)
        const uint32_t getID() const {
            assert(isComplete());

            // search existing dictionary
            uint32_t id = m_matchings->getFirstID();
            if (id == UINT_MAX) {
                id = m_dictionary->addCustomWord(m_mask);
            }
            return id;
        }
        uint32_t addCustomID() {
            return m_dictionary->addCustomWord(m_mask);
        }
        uint32_t removeCustomID() {
            const uint32_t id = m_dictionary->getCustomWordID(m_mask);
            m_dictionary->removeCustomWordID(id);
            return id;
        }

        // exclusions management for doMatch()
        void exclude(const int id) {
            m_excluded.insert(id);
        }
        void include(const int id) {
            m_excluded.erase(id);
        }
        const std::set<uint32_t>& getExclusions() const {
            return m_excluded;
        }

        // current domains
        const std::vector<ABMask>& getAllowed() const {
            return m_letterMasks;
        }
        std::vector<ABMask>& getAllowed() {
            return m_letterMasks;
        }
        ABMask getAllowed(const uint32_t i) const {
            return m_letterMasks[i];
        }

        // current matchings (from last doMatch())
        const MatchingResult* getMatchingResult() const {
            return m_matchings;
        }

        // true if no wildcards in pattern
        const uint32_t getWildcards() const {
            return m_wildcards;
        }
        bool isComplete() const {
            return (m_wildcards == 0);
        }

    private:

        // dictionary and definition references
        Dictionary* const m_dictionary;
        const Definition* const m_defRef;

        std::string m_mask;
        uint32_t m_wildcards;
        std::vector<ABMask> m_letterMasks;
        MatchingResult* m_matchings;

        // ID based exclusions
        std::set<uint32_t> m_excluded;
    };
}

#endif
