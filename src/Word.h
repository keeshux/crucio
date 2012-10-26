/*
 * Word.h
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
        Word(const Dictionary* const dict, const Definition* defRef) :
            m_dictionary(dict),
            m_defRef(defRef),
            m_mask(defRef->getLength(),
                   Dictionary::ANY_CHAR),
            m_wildcards(defRef->getLength()),
            m_letterMasks(defRef->getLength(),
                          ABMask(Dictionary::ANY_MASK)),
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

        // assignment
        const std::string& get() const {
            return m_mask;
        }
        void set(const std::string& mask) {
            m_mask = mask;
            m_wildcards = count(m_mask.begin(), m_mask.end(),
                                Dictionary::ANY_CHAR);
        }
        void unset() {
            std::fill(m_mask.begin(), m_mask.end(), Dictionary::ANY_CHAR);
            m_wildcards = m_mask.length();
        }

        // pattern I/O
        char getAt(const uint32_t i) const {
            return m_mask[i];
        }
        void setAt(const uint32_t i, const char ch) {
            assert(ch != Dictionary::ANY_CHAR);
            if (m_mask[i] == Dictionary::ANY_CHAR) {
                --m_wildcards;
            }
            m_mask[i] = ch;
        }
        void unsetAt(const uint32_t i) {
            if (m_mask[i] != Dictionary::ANY_CHAR) {
                ++m_wildcards;
            }
            m_mask[i] = Dictionary::ANY_CHAR;
        }

        // rematches pattern, updates matching result
        void doMatch() {
            m_dictionary->getMatchings(m_mask, m_matchings, &m_excluded);
        }

        // rematches pattern, updates matching result and letter masks
        void doMatchUpdating() {
            m_dictionary->getMatchings(m_mask, m_matchings, &m_excluded);

            // updates letters masks
            m_dictionary->getPossible(m_matchings, &m_letterMasks);
        }

        // exclusion list management for doMatch()
        void exclude(const uint32_t id) {
            m_excluded.insert(id);
        }
        void include(const uint32_t id) {
            m_excluded.erase(id);
        }

        // current domains
        const std::vector<ABMask>& getAllowed() const {
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
        bool isComplete() const {
            return (m_wildcards == 0);
        }

        // retrieves first matching word (only useful if isComplete())
        const uint32_t getFirstId() const {
            return m_matchings->getFirstWordId();
        }

    private:

        // dictionary and definition references
        const Dictionary* const m_dictionary;
        const Definition* const m_defRef;

        std::string m_mask;
        uint32_t m_wildcards;
        std::vector<ABMask> m_letterMasks;
        MatchingResult* m_matchings;
        std::set<uint32_t> m_excluded;
    };
}

#endif
