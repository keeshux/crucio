/*
 * Model.h
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

#ifndef __MODEL_H
#define __MODEL_H

#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "Dictionary.h"
#include "Grid.h"
#include "Letter.h"
#include "Word.h"

namespace crucio
{

// <word index, position in the word>
    class LetterPosition
    {
    public:
        LetterPosition(const uint32_t wi, const uint32_t pos) :
            m_wi(wi),
            m_pos(pos) {
        }

        uint32_t getWordIndex() const {
            return m_wi;
        }
        uint32_t getPosition() const {
            return m_pos;
        }

    private:
        uint32_t m_wi;
        uint32_t m_pos;
    };

// <word position, crossing word position>
    class WordCrossing
    {
    public:
        WordCrossing(const uint32_t pos, const uint32_t cwPos) :
            m_pos(pos),
            m_cwPos(cwPos) {
        }

        uint32_t getPosition() const {
            return m_pos;
        }
        uint32_t getCPosition() const {
            return m_cwPos;
        }

    private:
        uint32_t m_pos;
        uint32_t m_cwPos;
    };

    class Model
    {
    public:
        Model(Dictionary* const d, const Grid* const g);
        ~Model();

        // model alphabet
        Alphabet getAlphabet() const {
            return m_alphabet;
        }

        const Grid* getGrid() const {
            return m_grid;
        }
        const Dictionary* getDictionary() const {
            return m_dictionary;
        }

        // domains
        void computeLetterDomains();
        void computeWordDomains();

        // letters
        uint32_t getLettersNum() const {
            return m_letters.size();
        }
        const std::vector<Letter*>& getLetters() const {
            return m_letters;
        }
        Letter* getLetter(const uint32_t li) {
            return m_letters[li];
        }
        const Letter* getLetter(const uint32_t li) const {
            return m_letters[li];
        }
        int getLetterIndexByPos(const uint32_t i, const uint32_t j) const {
            return m_mappings[i][j].m_li;
        }

        // words
        uint32_t getWordsNum() const {
            return m_words.size();
        }
        const std::vector<Word*>& getWords() const {
            return m_words;
        }
        Word* getWord(const uint32_t wi) {
            return m_words[wi];
        }
        const Word* getWord(const uint32_t wi) const {
            return m_words[wi];
        }
        int getWordIndexByPos(const Definition::Direction dir,
                              const uint32_t i, const uint32_t j) const {
            return ((dir == Definition::ACROSS) ?
                    m_mappings[i][j].m_wiAcross :
                    m_mappings[i][j].m_wiDown);
        }

        // links from letters
        const std::vector<ABMask>& getInitLettersDomains() const {
            return m_initLettersDomains;
        }
        ABMask getInitLetterDomain(const uint32_t li) const {
            return m_initLettersDomains[li];
        }
        const std::list<LetterPosition>& getLetterWords(const uint32_t li) const {
            return m_lettersWords[li];
        }
        const std::list<std::pair<uint32_t, LetterPosition> >&
        getLetterNeighbours(const uint32_t li) const {
            return m_lettersNeighbours[li];
        }

        // links from words
        const std::vector<std::set<uint32_t> >& getInitWordsDomains() const {
            return m_initWordsDomains;
        }
        const std::set<uint32_t>& getInitWordDomain(const uint32_t wi) const {
            return m_initWordsDomains[wi];
        }
        const std::vector<uint32_t>& getWordLetters(const uint32_t wi) const {
            return m_wordsLetters[wi];
        }
        const uint32_t getWordLetter(const uint32_t wi, const uint32_t pos) const {
            return m_wordsLetters[wi][pos];
        }
        const std::list<std::pair<uint32_t, WordCrossing> >&
        getWordNeighbours(const uint32_t wi) const {
            return m_wordsNeighbours[wi];
        }

        // maps words given their length
        const std::map<uint32_t, std::set<uint32_t> >& getWordsByLength() const {
            return m_wordsByLength;
        }
        const std::set<uint32_t>& getWordsByLength(const uint32_t len) const {
            return m_wordsByLength.find(len)->second;
        }

        // true if model has no solutions, i.e. empty domains
        bool isOverConstrained() const {
            return m_overConstrained;
        }

        void reset() {

            // sets letters to initial value
            for (uint32_t li = 0; li < m_letters.size(); ++li) {
                Letter* const l = m_letters[li];
                l->set(m_initLetters[li]);
            }

            // sets words to initial value
            for (uint32_t wi = 0; wi < m_words.size(); ++wi) {
                Word* const w = m_words[wi];
                w->set(m_initWords[wi]);
                w->doMatchUpdating();
            }
        }

    private:

        // structure containing mapped letter/words in grid matrix
        class LWInfo
        {
        public:
            int m_li;
            int m_wiAcross;
            int m_wiDown;

            LWInfo() :
                m_li(-1),
                m_wiAcross(-1),
                m_wiDown(-1) {
            }
        };

        // model alphabet with related dictionary
        const Alphabet m_alphabet;
        Dictionary* const m_dictionary;

        // grid reference
        const Grid* const m_grid;

        // helper data structure
        std::vector<std::vector<LWInfo> > m_mappings;

        // letters and words
        std::vector<Letter*> m_letters;
        std::vector<Word*> m_words;

        // links from letters
        std::vector<char> m_initLetters;
        std::vector<ABMask> m_initLettersDomains;
        std::vector<std::list<LetterPosition> > m_lettersWords;
        std::vector<std::list<std::pair<uint32_t, LetterPosition> > >
        m_lettersNeighbours;

        // links from words
        std::vector<std::string> m_initWords;
        std::vector<std::set<uint32_t> > m_initWordsDomains;
        std::vector<std::vector<uint32_t> > m_wordsLetters;
        std::vector<std::list<std::pair<uint32_t, WordCrossing> > >
        m_wordsNeighbours;

        // maps words given their length
        std::map<uint32_t, std::set<uint32_t> > m_wordsByLength;

        // overconstrained flag
        bool m_overConstrained;
    };
}

#endif
