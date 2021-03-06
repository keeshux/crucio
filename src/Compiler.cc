//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#include "Compiler.h"
#include "LanguageMatcher.h"

using namespace crucio;
using namespace std;

Compiler::Compiler() :
    m_unique(false),
    m_deterministic(false),
    m_model(NULL)
{
}

Compiler::~Compiler()
{
}

Compiler::Result Compiler::compile(Model* const model, const Walk& walk)
{
    assert(model != NULL);
    
    // timeout reference
    m_compileSeconds = time(NULL);

    // model reference
    m_model = model;

    // configures internal objects
    const Compiler::Result result = configure(walk);
    if (result != Compiler::SUCCESS) {
        return result;
    }

    // checks for empty domains
    if (m_model->isOverConstrained()) {
        return FAILURE_OVERCONSTRAINED;
    }

    // prior check for non-determinism
    if (isDeterministic()) {
        const map<uint32_t, set<uint32_t> >& wordsByLength =
            m_model->getWordsByLength();

        // no length mapping to one-and-only-one word
        if (find_if(wordsByLength.begin(), wordsByLength.end(),
                    IsSimpleDomain()) == wordsByLength.end()) {
            return FAILURE_ND_GRID;
        }
    }

    // repeats until success or failure, or deterministic
    // solution if requested
    bool validSolution = true;
    do {

        // resets internal status
        reset();

        // defaults to valid solution (helps debugging, avoids
        // infinite loop)
        validSolution = true;

        // starts backtracking from variable 0
        const Result result = compileFrom(0);
        if (result != SUCCESS) {
            return result;
        }

        // posterior check for non-determinism
        if (isDeterministic()) {
            *crucio_vout << "deterministic solution check ... ";

            validSolution = isDeterministicSolution();

            if (validSolution) {
                *crucio_vout << "OK!" << endl;
            } else {
                *crucio_vout << "FAILED!" << endl;
                *crucio_vout << "algorithm will now restart" << endl;
            }
            *crucio_vout << endl;
        }
    } while (!validSolution);

    // finally valid solution
    return SUCCESS;
}

// checks for solution determinism (assumes ALL words are complete, i.e.
// model compiling succeeded)
bool Compiler::isDeterministicSolution() const
{
    // word index and solution words iterator
    uint32_t wi;
    vector<Word*>::iterator swIt;

    // determinism control is based on word-filling
    const uint32_t wordsNum = m_model->getWordsNum();

    // solution wordlist contains now completed words
    set<string> solWordList;
    for (wi = 0; wi < wordsNum; ++wi) {
        const Word* const w = m_model->getWord(wi);
        solWordList.insert(w->get());
    }

    // solution dictionary based on solution word list
    LanguageMatcher solMatcher(&solWordList);
    Dictionary solDict(m_model->getAlphabet(), &solMatcher);

    // creates new words based on solution dictionary
    vector<Word*> solWords(wordsNum, (Word*) NULL);
    for (wi = 0; wi < wordsNum; ++wi) {
        const Word* const w = m_model->getWord(wi);

        // solution dictionary, same definition reference
        solWords[wi] = new Word(&solDict, w->getDefinition());
        solWords[wi]->doMatch();
    }

    // checks for 1-choice constraint at each filling stage
    bool success = true;
    for (wi = 0; wi < wordsNum; ++wi) {

        // first word having 1-element domain
        swIt = find_if(solWords.begin(), solWords.end(), IsSimpleDomain());

        // no 1-element domains implies non-determinism
        if (swIt == solWords.end()) {
            success = false;
            break;
        }

        // computes chosen word index and gets related object
        const uint32_t sWi = (uint32_t)distance(solWords.begin(), swIt);
        Word* const sw = solWords[sWi];

        // gets matchings object to reach domain only-value
        const uint32_t chosenID = sw->getID();
        const string& chosen = sw->get();

        // assigns domain value to the word
        sw->set(chosen);

        // removes value from domains of words of same length
        const set<uint32_t>& slWords = m_model->getWordsByLength(sw->getLength());
        set<uint32_t>::const_iterator slwIt;
        for (slwIt = slWords.begin(); slwIt != slWords.end(); ++slwIt) {
            const uint32_t slWi = *slwIt;

            // skips current word
            if (slWi != sWi) {
                Word* const slw = solWords[slWi];

                // checks for prior assignment (that is (slw == 0))
                if (slw) {

                    // excludes chosen value for sw from slw
                    slw->exclude(chosenID);
                    slw->doMatch();
                }
            }
        }

        // updates crossing words
        const list<pair<uint32_t, WordCrossing> >& neighbours =
            m_model->getWordNeighbours(sWi);
        list<pair<uint32_t, WordCrossing> >::const_iterator nbIt;
        for (nbIt = neighbours.begin(); nbIt != neighbours.end(); ++nbIt) {
            Word* const cw = solWords[nbIt->first];

            // checks for prior assignment (that is (cw == 0))
            if (cw) {

                // crossing positions
                const uint32_t swPos = nbIt->second.getPosition();
                const uint32_t cwPos = nbIt->second.getCPosition();

                // updates shared letter
                cw->setAt(cwPos, sw->getAt(swPos));
                cw->doMatch();
            }
        }

        // deletes last choice
        delete *swIt;
        *swIt = NULL;
    }

    // deallocates words where needed
    for (swIt = solWords.begin(); swIt != solWords.end(); ++swIt) {
        Word* const sw = *swIt;

        // deallocates word iff not already
        if (sw) {
            delete sw;
        }
    }

    return success;
}
