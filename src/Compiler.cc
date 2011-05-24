#include "Compiler.h"

using namespace crucio;
using namespace std;

Compiler::Compiler() :
        m_unique(false),
        m_deterministic(false),
        m_verbose(false),
        m_model(0),
        m_verboseOut(0) {
}

Compiler::~Compiler() {
}

Compiler::Result Compiler::compile(Model* const m, const Walk& w) {

    // model reference
    m_model = m;

    // configures internal objects
    configure(w);

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
        if (!compileFrom(0)) {
            return FAILURE_IMPOSSIBLE;
        }

        // posterior check for non-determinism
        if (isDeterministic()) {
            if (isVerbose()) {
                *m_verboseOut << "deterministic solution check ... ";
            }
            validSolution = isDeterministicSolution();
            if (isVerbose()) {
                if (validSolution) {
                    *m_verboseOut << "OK!" << endl;
                } else {
                    *m_verboseOut << "FAILED!" << endl;
                    *m_verboseOut << "algorithm will now restart" << endl;
                }
                *m_verboseOut << endl;
            }
        }
    } while (!validSolution);

    // finally valid solution
    return SUCCESS;
}

// checks for solution determinism (assumes ALL words are complete, i.e.
// model compiling succeeded)
bool Compiler::isDeterministicSolution() const {

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
    const Dictionary solDict(solWordList);

    // creates new words based on solution dictionary
    vector<Word*> solWords(wordsNum, (Word*) 0);
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
        const uint32_t sWi = distance(solWords.begin(), swIt);
        Word* const sw = solWords[sWi];

        // gets matchings object to reach domain only-value
        const uint32_t chosenId = sw->getFirstId();
        const string& chosen = sw->getFirst();

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
                    slw->exclude(chosenId);
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
        *swIt = 0;
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
