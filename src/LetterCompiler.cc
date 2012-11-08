/*
 * LetterCompiler.cc
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

#include "LetterCompiler.h"

using namespace crucio;
using namespace std;

LetterCompiler::LetterCompiler() :
    m_domains(),
    m_order(),
    m_revOrder(),
    m_deps(),
    m_revDeps(),
    m_bj()
{
}

Compiler::Result LetterCompiler::configure(const Walk& walk)
{
//    uint32_t li, ordLi;
    uint32_t li;

    // prepare alphabet conversions
    m_alphabet = m_model->getAlphabet();
    m_alphabetSize = alphabetSize(m_alphabet);

    // letters count
    const uint32_t lettersNum = m_model->getLettersNum();

    // sized after letters count
    m_deps.resize(lettersNum);
    m_revDeps.resize(lettersNum);

    // letters graph visit
    walk.visitLetters(*m_model, &m_order);
    Walk::getReverseOrder(m_order, &m_revOrder);

    // direct and reverse dependencies
    for (li = 0; li < lettersNum; ++li) {

//        // current letter index within ordering
//        ordLi = m_revOrder[li];

        // computes dependencies from related letters
        const list<pair<uint32_t, LetterPosition> >& neighbours =
            m_model->getLetterNeighbours(li);
        list<pair<uint32_t, LetterPosition> >::const_iterator nbIt;
        for (nbIt = neighbours.begin(); nbIt != neighbours.end(); ++nbIt) {
            const uint32_t rLi = nbIt->first;

            // letters instantiated BEFORE current are direct
            // dependencies; letters instantiated AFTER current are
            // reverse dependencies; li is excluded from related
            if (m_revOrder[rLi] < m_revOrder[li]) {
                m_deps[li].push_back(*nbIt);
            } else {
                m_revDeps[li].push_back(*nbIt);
            }
        }
        if (isVerbose()) {
            *crucio_vout << "l" << li << endl;
            list<pair<uint32_t, LetterPosition> >::const_iterator dIt;

            // affecting letters
            const list<pair<uint32_t, LetterPosition> >& liDeps = m_deps[li];
            if (!liDeps.empty()) {
                *crucio_vout << "\tdepends on: ";
                for (dIt = liDeps.begin(); dIt != liDeps.end(); ++dIt) {
                    const uint32_t dLi = dIt->first;
                    const LetterPosition& dLp = dIt->second;

                    *crucio_vout << "l" << dLi <<
                                 " (w" << dLp.getWordIndex() <<
                                 "[" << dLp.getPosition() << "])";
                    *crucio_vout << " ";
                }
                *crucio_vout << endl;
            }

            // affected letters
            const list<pair<uint32_t, LetterPosition> >& liRevDeps = m_revDeps[li];
            if (!liRevDeps.empty()) {
                *crucio_vout << "\trestricts: ";
                for (dIt = liRevDeps.begin(); dIt != liRevDeps.end(); ++dIt) {
                    const uint32_t dLi = dIt->first;
                    const LetterPosition& dLp = dIt->second;

                    *crucio_vout << "l" << dLi <<
                                 " (w" << dLp.getWordIndex() <<
                                 "[" << dLp.getPosition() << "])";
                    *crucio_vout << " ";
                }
                *crucio_vout << endl;
            }

            *crucio_vout << endl;
        }
    }

    // backjumper setup
    vector<list<uint32_t> > ordDeps(lettersNum);
    for (li = 0; li < lettersNum; ++li) {
        const list<pair<uint32_t, LetterPosition> >& liDeps = m_deps[li];

        // only adds letter order without LetterPosition object
        list<pair<uint32_t, LetterPosition> >::const_iterator dIt;
        for (dIt = liDeps.begin(); dIt != liDeps.end(); ++dIt) {
            ordDeps[li].push_back(dIt->first);
        }
    }
    m_bj.configure(m_order, ordDeps);

    // use letter-based domains
    m_model->computeLetterDomains();

    return Compiler::SUCCESS;
}

void LetterCompiler::reset()
{
    m_model->reset();
    m_bj.reset();
    m_domains = m_model->getInitLettersDomains();
}

Compiler::Result LetterCompiler::compileFrom(const uint32_t i)
{
    // all variables instantiated?
    if (i == m_model->getLettersNum()) {
        return Compiler::SUCCESS;
    } else {

        // maps variable from ordering
        const uint32_t li = m_order[i];

        // admittable domain
        ABMask domainMask = m_domains[li];
        *crucio_vout << "domain for " << li << " = " << ABMaskString(m_alphabet, domainMask) << endl;

        // removal stack through forward checking
        stack<pair<uint32_t, ABMask> > remStack;

#ifdef CRUCIO_BJ_FAST
        // FC failures ignored
        set<uint32_t>* const failedPtr = NULL;
#else
        // FC failures added for BJ
        set<uint32_t> failed;
        set<uint32_t>* const failedPtr = &failed;
#endif

        // iterates over domain
        while (domainMask.any()) {

            // chooses value to assign
            const char v = choose(&domainMask);

            *crucio_vout << "letter " << li << " = '" << v << "'" << endl;

            // tries to assign v to current variable
            if (!assign(li, v, &remStack, failedPtr)) {
                retire(li, &remStack);
                continue;
            }

            *crucio_vout << endl;
            printModelGrid(*crucio_vout, *m_model);
            *crucio_vout << endl;

            // recursively solved?
            if (compileFrom(i + 1) == Compiler::SUCCESS) {
                return Compiler::SUCCESS;
            } else {
                
                // retires variable
                retire(li, &remStack);

#ifdef CRUCIO_BJ
                // returns if there are no more jumps or last
                // jump gets past current variable
                if (m_bj.isExhausted() ||
                        (m_bj.getDestination() < i)) {

                    return Compiler::FAILURE_IMPOSSIBLE;
                }
#endif
            }

            // timeout check
            if (isTimeout()) {
                return Compiler::FAILURE_TIMEOUT;
            }
        }

        *crucio_vout << "letter " << li << " ... BACKTRACK!" << endl;

        // algorithm fails iff first variable backtracks
        if (i > 0) {
#ifdef CRUCIO_BJ
            m_bj.jump(i, failedPtr);
#endif

            *crucio_vout << "jump from " << m_order[m_bj.getOrigin()] <<
                         " to " << m_order[m_bj.getDestination()] << endl;
            *crucio_vout << endl;
        }

        return Compiler::FAILURE_IMPOSSIBLE;
    }
}

char LetterCompiler::choose(ABMask* const domainMask)
{
    uint32_t vi = 0;

#ifdef CRUCIO_BENCHMARK
    // ordered choice, no randomness
    while (!(*domainMask)[vi]) {
        ++vi;
    }
#else
    // random choice in [A-Z]

    // XXX: bad method for sparse bitmask (used in benchmarks)
    do {
        vi = rand() % m_alphabetSize;
    } while (!(*domainMask)[vi]);

    // better method for sparse bitmask
    /*  uint32_t active[m_alphabetSize];
        uint32_t ai = 0;
        for (vi = 0; vi < m_alphabetSize; ++vi) {

            // saves active bit position
            if ((*domainMask)[vi]) {
                active[ai] = vi;
                ++ai;
            }
        }

        // chooses randomly among active bits
        const uint32_t chosenAi = rand() % ai;
        vi = active[chosenAi];
    */
#endif

    // marks vi-th value as visited
    domainMask->reset(vi);

    // alphabetic mapping for vi
    return index2Character(m_alphabet, vi);
}

bool LetterCompiler::assign(const uint32_t li,
                            const char v,
                            stack<pair<uint32_t, ABMask> >* const remStack,
                            set<uint32_t>* const failed)
{
    // gets letter object
    Letter* const l = m_model->getLetter(li);

    // assigns letter value
    l->set(v);

    // words current letter belongs to
    const list<LetterPosition>& letterWords = m_model->getLetterWords(li);

    // puts new value into related words
    list<LetterPosition>::const_iterator lpIt;
    for (lpIt = letterWords.begin(); lpIt != letterWords.end(); ++lpIt) {

        // related word and its length
        const uint32_t wi = lpIt->getWordIndex();
        const uint32_t pos = lpIt->getPosition();
        Word* const w = m_model->getWord(wi);
        const uint32_t wLen = w->getLength();

        // sets character v at position pos within word w
        w->setAt(pos, v);

        // recalculates possible letters within the word
        w->doMatchUpdating();

        // word completed, constrains remaining words having same length
        if (isUnique() && w->isComplete()) {

            *crucio_vout << "completed word " <<
                         *w->getDefinition() << ": " << w->get() << "" << endl;

            // excluded word ID
            uint32_t excludedID = w->getID();
            if (excludedID == UINT_MAX) {
                excludedID = w->addCustomID(); // add custom dictionary entry
                if (excludedID == UINT_MAX) {
                    continue;
                }
            }

            // selects words subset by length
            const map<uint32_t, set<uint32_t> >& wordsByLength =
                m_model->getWordsByLength();
            const set<uint32_t>& subset = wordsByLength.find(wLen)->second;

            // updates exclusion lists and recalculates domains
            set<uint32_t>::const_iterator slWiIt;
            for (slWiIt = subset.begin(); slWiIt != subset.end(); ++slWiIt) {
                const uint32_t slWi = *slWiIt;

                // skips current word
                if (slWi == wi) {
                    continue;
                }

                // same length word
                Word* const slw = m_model->getWord(slWi);

                // excludes completed word and rematches pattern
                slw->exclude(excludedID);
                slw->doMatchUpdating();

                // domains update
                const vector<uint32_t>& wordLetters =
                    m_model->getWordLetters(slWi);
                for (uint32_t slwPos = 0; slwPos < wLen; ++slwPos) {
                    const int slwLi = wordLetters[slwPos];

                    // skips fixed and previously instantiated letters
                    if ((slwLi == -1) || (m_revOrder[slwLi] < m_revOrder[li])) {
                        continue;
                    }

                    // intersects domain saving removed values
                    ABMask* const slwDom = &m_domains[slwLi];
                    ABMask remValues = *slwDom;
                    *slwDom &= slw->getAllowed(slwPos);
                    remValues &= ~*slwDom;

                    // puts removed values on the stack
                    remStack->push(make_pair(slwLi, remValues));

                    if (remValues.any()) {
                        *crucio_vout << "\tletter " << slwLi <<
                                     ": removed " << ABMaskString(m_alphabet, remValues) << ", ";
                        *crucio_vout << "now " << ABMaskString(m_alphabet, *slwDom);
                        *crucio_vout << " (UNIQUE)" << endl;
                    }

                    // current assignment invalidated in other words
                    const uint32_t vi = character2Index(m_alphabet, v);
                    if (((uint32_t)slwLi == li) && remValues.test(vi)) {
                        *crucio_vout << "\tletter " << li <<
                                     ": invalidated (UNIQUE)" << endl;

                        return false;
                    }

                    // an empty domain implies failure
                    if (slwDom->none()) {
#ifndef CRUCIO_BJ_FAST
                        // adds failed variable order
                        failed->insert(m_revOrder[slwLi]);
#endif
                        *crucio_vout << "\tFC failed at " <<
                                     slwLi << " (UNIQUE)" << endl;

                        return false;
                    }
                }
            }
        }
    }

    // domains reduction
    const list<pair<uint32_t, LetterPosition> >& liRevDeps = m_revDeps[li];
    list<pair<uint32_t, LetterPosition> >::const_iterator dIt;
    for (dIt = liRevDeps.begin(); dIt != liRevDeps.end(); ++dIt) {

        // affected letter
        const uint32_t dLi = dIt->first;

        // affected word and letter position
        const LetterPosition& dLp = dIt->second;
        Word* const dw = m_model->getWord(dLp.getWordIndex());
        const uint32_t dPos = dLp.getPosition();

        // intersects domain saving removed values
        ABMask* const dDom = &m_domains[dLi];
        ABMask remValues = *dDom;
        *dDom &= dw->getAllowed(dPos);
        remValues &= ~*dDom;

        // puts removed values on the stack
        remStack->push(make_pair(dLi, remValues));

        if (remValues.any()) {
            *crucio_vout << "\tletter " << dLi <<
                         ": removed " << ABMaskString(m_alphabet, remValues) << ", ";
            *crucio_vout << "now " << ABMaskString(m_alphabet, *dDom) << endl;
        }

        // an empty domain implies failure
        if (dDom->none()) {
#ifndef CRUCIO_BJ_FAST
            // adds failed variable order
            failed->insert(m_revOrder[dLi]);
#endif
            *crucio_vout << "\tFC failed at " << dLi << endl;

            return false;
        }
    }

    return true;
}

void LetterCompiler::retire(const uint32_t li,
                            stack<pair<uint32_t, ABMask> >* const remStack)
{
    // gets letter object
    Letter* const l = m_model->getLetter(li);

    // words current letter belongs to
    const list<LetterPosition>& letterWords = m_model->getLetterWords(li);

    // removes current value from related words
    list<LetterPosition>::const_iterator lpIt;
    for (lpIt = letterWords.begin(); lpIt != letterWords.end(); ++lpIt) {

        // related word and its length
        const uint32_t wi = lpIt->getWordIndex();
        const uint32_t pos = lpIt->getPosition();
        Word* const w = m_model->getWord(wi);
        const uint32_t wLen = w->getLength();

        // word previously completed, readmit it in same length words
        if (isUnique() && w->isComplete()) {

            // excluded word ID
            uint32_t excludedID = w->getID();
            if (excludedID == UINT_MAX) {
                excludedID = w->removeCustomID();
                if (excludedID == UINT_MAX) {
                    continue;
                }
            }

            // selects words subset by length
            const map<uint32_t, set<uint32_t> >& wordsByLength =
                m_model->getWordsByLength();
            const set<uint32_t>& subset = wordsByLength.find(wLen)->second;

            // updates exclusion lists
            set<uint32_t>::const_iterator slWiIt;
            for (slWiIt = subset.begin(); slWiIt != subset.end(); ++slWiIt) {
                const uint32_t slWi = *slWiIt;
                Word* const slw = m_model->getWord(slWi);

                // reincludes completed word
                slw->include(excludedID);
            }
        }

        // removes character at position pos from word w
        w->unsetAt(pos);
    }

    // retires letter value
    l->unset();

    // restores domains
    while (!remStack->empty()) {

        // newest removal
        const pair<uint32_t, ABMask>& remTop = remStack->top();
        const uint32_t rLi = remTop.first;
        const ABMask& rMask = remTop.second;

        // readmit value
        m_domains[rLi] |= rMask;

        // removes top
        remStack->pop();
    }
}
