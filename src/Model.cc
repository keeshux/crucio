/*
 * Model.cc
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

#include "Model.h"

using namespace crucio;
using namespace std;

Model::Model(const Type type, const Grid* const g, const Dictionary* const d) :
        m_type(type),
        m_grid(g),
        m_dictionary(d),
        m_mappings(0),
        m_letters(g->getNonBlackCells()),
        m_words(g->getWords()),
        m_initLetters(g->getNonBlackCells(), Dictionary::ANY_CHAR),
        m_initLettersDomains(g->getNonBlackCells(), Dictionary::ANY_MASK),
        m_lettersWords(g->getNonBlackCells()),
        m_lettersNeighbours(g->getNonBlackCells()),
        m_initWords(g->getWords()),
        m_initWordsDomains(g->getWords()),
        m_wordsLetters(g->getWords()),
        m_wordsNeighbours(g->getWords()),
        m_wordsByLength(),
        m_overConstrained(false) {

    // letters and words indexes
    int li, wi;
    const int lettersNum = m_letters.size();
    const int wordsNum = m_words.size();

    // matrices indexes
    uint32_t i, j, pos;

    // grid traversing
    const Cell* cl = 0;

    // letters/words indexes matrix
    m_mappings.resize(m_grid->getRows());
    for (i = 0; i < m_grid->getRows(); ++i) {
        m_mappings[i].resize(m_grid->getColumns());
    }

    // letters creation
    li = 0;
    for (i = 0; i < m_grid->getRows(); ++i) {
        for (j = 0; j < m_grid->getColumns(); ++j) {

            // current cell
            cl = m_grid->getCell(i, j);

            // letters are non-black cells
            if (!cl->isBlack()) {
                i = cl->getRow();
                j = cl->getColumn();

                // puts letter index in mappings matrix
                m_mappings[i][j].m_li = li;

                // creates and stores letter into letters vector
                Letter* const nl = new Letter(cl);
                m_letters[li] = nl;

                // initial letter value
                if (cl->isFixed()) {
                    const char ch = cl->getValue();

                    // sets value and saves it into init vector
                    nl->set(ch);
                    m_initLetters[li] = ch;
                }

                // next letter
                ++li;
            }
        }
    }

    // words creation
    wi = 0;
    for (i = 0; i < m_grid->getRows(); ++i) {
        for (j = 0; j < m_grid->getColumns(); ++j) {

            // current cell
            cl = m_grid->getCell(i, j);

            // words starting here
            const Definition* const acrossDef = cl->getAcrossDef();
            const Definition* const downDef = cl->getDownDef();

            // across
            if (acrossDef) {

                // creates and stores word into words vector
                Word* const nw = new Word(m_dictionary, acrossDef);
                m_words[wi] = nw;

                // cycles through letters in the word
                const uint32_t nwLen = acrossDef->getLength();
                for (pos = 0; pos < nwLen; ++pos) {

                    // puts word index in mappings matrix
                    m_mappings[i][j + pos].m_wiAcross = wi;

                    // puts fixed values in the word
                    const Cell* const nwCl = m_grid->getCell(i, j + pos);
                    if (!nwCl->isWhite()) {
                        nw->setAt(pos, nwCl->getValue());
                    }
                }

                // saves initial word value into init vector
                m_initWords[wi] = nw->get();

                // puts word into the (length -> words) map
                const pair<map<uint32_t, set<uint32_t> >::iterator, bool> res =
                        m_wordsByLength.insert(make_pair(nwLen, set<uint32_t>()));
                set<uint32_t>* const subset = &res.first->second;
                subset->insert(wi);

                // next word
                ++wi;
            }

            // down
            if (downDef) {

                // creates and stores word into words vector
                Word* const nw = new Word(m_dictionary, downDef);
                m_words[wi] = nw;

                // cycles through cells in the word
                const uint32_t nwLen = downDef->getLength();
                for (pos = 0; pos < nwLen; ++pos) {

                    // puts word index in mappings matrix
                    m_mappings[i + pos][j].m_wiDown = wi;

                    // puts fixed values in the word
                    const Cell* const nwCl = m_grid->getCell(i + pos, j);
                    if (!nwCl->isWhite()) {
                        nw->setAt(pos, nwCl->getValue());
                    }
                }

                // saves initial word value into init vector
                m_initWords[wi] = nw->get();

                // puts word into the (length -> words) map
                const pair<map<uint32_t, set<uint32_t> >::iterator, bool> res =
                        m_wordsByLength.insert(make_pair(nwLen, set<uint32_t>()));
                set<uint32_t>* const subset = &res.first->second;
                subset->insert(wi);

                // next word
                ++wi;
            }
        }
    }

    // letters links
    for (li = 0; li < lettersNum; ++li) {
        const Letter* const l = m_letters[li];
        const Cell* const lCl = l->getCell();

        // letter coordinates
        i = lCl->getRow();
        j = lCl->getColumn();

        // referred words
        const int wiAcross = m_mappings[i][j].m_wiAcross;
        const int wiDown = m_mappings[i][j].m_wiDown;

        // across word
        if (wiAcross != -1) {
            const Definition* const lDef = m_words[wiAcross]->getDefinition();
            const Cell* const lDefCl = lDef->getStartCell();
            const uint32_t lDefLen = lDef->getLength();

            // word starting coordinates
            i = lDefCl->getRow();
            j = lDefCl->getColumn();

            // browses word current letter belongs to
            for (pos = 0; pos < lDefLen; ++pos) {
                const int lLi = m_mappings[i][j + pos].m_li;

                // li is with lLi in word wiAcross at position pos
                if (lLi != li) {
                    m_lettersNeighbours[li].push_back(
                            make_pair(lLi, LetterPosition(wiAcross, pos)));
                }

                // letter lLi belongs to word wiAcross at position pos
                m_lettersWords[lLi].push_back(LetterPosition(wiAcross, pos));
            }
        }

        // down word
        if (wiDown != -1) {
            const Definition* const lDef = m_words[wiDown]->getDefinition();
            const Cell* const lDefCl = lDef->getStartCell();
            const uint32_t lDefLen = lDef->getLength();

            // word starting coordinates
            i = lDefCl->getRow();
            j = lDefCl->getColumn();

            // browses word current letter belongs to
            for (pos = 0; pos < lDefLen; ++pos) {
                const int lLi = m_mappings[i + pos][j].m_li;

                // li is with lLi in word wiDown at position pos
                if (lLi != li) {
                    m_lettersNeighbours[li].push_back(
                            make_pair(lLi, LetterPosition(wiDown, pos)));
                }

                // letter lLi belongs to word wiDown at position pos
                m_lettersWords[lLi].push_back(LetterPosition(wiDown, pos));
            }
        }
    }

    // words links
    for (wi = 0; wi < wordsNum; ++wi) {
        const Word* const w = m_words[wi];
        const uint32_t wLen = w->getLength();
        const Definition* const def = w->getDefinition();

        // word starting coordinates
        const Cell* const defCl = def->getStartCell();
        i = defCl->getRow();
        j = defCl->getColumn();

        // resizes word letters vector to word length
        m_wordsLetters[wi].resize(wLen);

        // browses word depending on its direction
        if (def->getDirection() == Definition::ACROSS) {
            for (pos = 0; pos < wLen; ++pos) {
                const int wLi = m_mappings[i][j + pos].m_li;

                // crossing down word
                const int cWi = m_mappings[i][j + pos].m_wiDown;
                if (cWi != -1) {
                    const Definition* const cDef =
                            m_words[cWi]->getDefinition();
                    const Cell* const cDefCl = cDef->getStartCell();

                    // letter position within down word
                    const uint32_t cwPos = i - cDefCl->getRow();

                    // wi at position pos crosses word cWi at position cwPos
                    m_wordsNeighbours[wi].push_back(
                            make_pair(cWi, WordCrossing(pos, cwPos)));
                }

                // pos-th letter in word wi is wLi
                m_wordsLetters[wi][pos] = wLi;
            }
        } else {
            for (pos = 0; pos < wLen; ++pos) {
                const uint32_t wLi = m_mappings[i + pos][j].m_li;

                // crossing across word
                const int cWi = m_mappings[i + pos][j].m_wiAcross;
                if (cWi != -1) {
                    const Definition* const cDef =
                            m_words[cWi]->getDefinition();
                    const Cell* const cDefCl = cDef->getStartCell();

                    // letter position within across word
                    const uint32_t cwPos = j - cDefCl->getColumn();

                    // wi at position pos crosses word cWi at position cwPos
                    m_wordsNeighbours[wi].push_back(
                            make_pair(cWi, WordCrossing(pos, cwPos)));
                }

                // pos-th letter in word wi is wLi
                m_wordsLetters[wi][pos] = wLi;
            }
        }
    }

    // first matchings
    for (wi = 0; wi < wordsNum; ++wi) {
        Word* const w = m_words[wi];
        
        // calculates words/letters domains
        w->doMatch(true);
        
        // empty domain
//        m_overConstrained |= w->getAllowed()->empty();
        
//        // XXX: shortcut
//        if (m_overConstrained) {
//            return;
//        }
    }
}

Model::~Model() {

    // letters
    vector<Letter*>::iterator lIt;
    for (lIt = m_letters.begin(); lIt != m_letters.end(); ++lIt) {
        delete *lIt;
    }

    // words
    vector<Word*>::iterator wIt;
    for (wIt = m_words.begin(); wIt != m_words.end(); ++wIt) {
        delete *wIt;
    }
}

void Model::computeLetterDomains() {
    const int wordsNum = m_words.size();
    int wi;
    uint32_t pos;

    // initial domains
    for (wi = 0; wi < wordsNum; ++wi) {
        Word* const w = m_words[wi];
        const uint32_t wLen = w->getLength();
        
        // updates domains of owned letters (initially ANY_MASK)
        const vector<uint32_t>& wiLetters = m_wordsLetters[wi];
        for (pos = 0; pos < wLen; ++pos) {
            const uint32_t wLi = wiLetters[pos];
            ABMask* const wlDom = &m_initLettersDomains[wLi];
            
            // intersects domain
            *wlDom &= w->getAllowed(pos);
            
            // empty letter domain
            m_overConstrained |= wlDom->none();
        }
    }
}

// WARNING: very high memory usage
void Model::computeWordDomains() {
    const int wordsNum = m_words.size();
    int wi;
    
    // initial domains
    for (wi = 0; wi < wordsNum; ++wi) {
        Word* const w = m_words[wi];
        
        // word domain
        set<uint32_t>* const wDom = &m_initWordsDomains[wi];
        const Dictionary::MatchingResult* const wRes = w->getMatchingResult();
        wRes->getIdsUnion(wDom);
        
        // empty word domain
        m_overConstrained |= wDom->empty();
    }
}
