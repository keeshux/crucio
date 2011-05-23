#include "WordCompiler.h"

using namespace crucio;
using namespace std;

#define USE_BJ
#define USE_BJ_FAST

WordCompiler::WordCompiler() :
		m_domains(),
		m_order(),
		m_revOrder(),
		m_deps(),
		m_revDeps(),
		m_bj() {
}

void WordCompiler::configure(const Walk& w) {
	uint32_t wi, ordWi;

	// words count
	const uint32_t wordsNum = m_model->getWordsNum();

	// sized after words vector
	m_deps.resize(wordsNum);
	m_revDeps.resize(wordsNum);

	// words graph visit
	w.visitWords(*m_model, &m_order);
	Walk::getReverseOrder(m_order, &m_revOrder);

	// direct and reverse dependencies
	for (wi = 0; wi < wordsNum; ++wi) {

		// current word index within ordering
		ordWi = m_revOrder[wi];

		// computes dependencies from related words
		const list<pair<uint32_t, WordCrossing> >& neighbours =
				m_model->getWordNeighbours(wi);
		list<pair<uint32_t, WordCrossing> >::const_iterator nbIt;
		for (nbIt = neighbours.begin(); nbIt != neighbours.end(); ++nbIt) {
			const uint32_t rWi = nbIt->first;

			// words instantiated BEFORE current are direct
			// dependencies; words instantiated AFTER current are
			// reverse dependencies
			if (m_revOrder[rWi] < m_revOrder[wi]) {
				m_deps[wi].push_back(*nbIt);
			} else {
				m_revDeps[wi].push_back(*nbIt);
			}
		}

		if (isVerbose()) {
			*m_verboseOut << "w" << wi << endl;
			list<pair<uint32_t, WordCrossing> >::const_iterator dIt;

			// affecting words
			const list<pair<uint32_t, WordCrossing> >& wiDeps = m_deps[wi];
			if (!wiDeps.empty()) {
				*m_verboseOut << "\tdepends on: ";
				for (dIt = wiDeps.begin(); dIt != wiDeps.end(); ++dIt) {
					const uint32_t dWi = dIt->first;
					const uint32_t pos = dIt->second.getPosition();
					const uint32_t dPos = dIt->second.getCPosition();

					*m_verboseOut << "w" << dWi << " " <<
							"(w" << wi << "[" << pos << "]=" <<
							"w" << dWi << "[" << dPos << "])";
					*m_verboseOut << " ";
				}
				*m_verboseOut << endl;
			}

			// affected words
			const list<pair<uint32_t, WordCrossing> >& wiRevDeps = m_revDeps[wi];
			if (!wiRevDeps.empty()) {
				*m_verboseOut << "\trestricts: ";
				for (dIt = wiRevDeps.begin(); dIt != wiRevDeps.end(); ++dIt) {
					const uint32_t dWi = dIt->first;
					const uint32_t pos = dIt->second.getPosition();
					const uint32_t dPos = dIt->second.getCPosition();

					*m_verboseOut << "w" << dWi << " " <<
							"(w" << wi << "[" << pos << "]=" <<
							"w" << dWi << "[" << dPos << "])";
					*m_verboseOut << " ";
				}
				*m_verboseOut << endl;
			}

			*m_verboseOut << endl;
		}
	}

	// backjumper setup
	vector<list<uint32_t> > ordDeps(wordsNum);
	for (wi = 0; wi < wordsNum; ++wi) {
		const list<pair<uint32_t, WordCrossing> >& wiDeps = m_deps[wi];

		// only adds word order without WordCrossing object
		list<pair<uint32_t, WordCrossing> >::const_iterator dIt;
		for (dIt = wiDeps.begin(); dIt != wiDeps.end(); ++dIt) {
			ordDeps[wi].push_back(dIt->first);
		}
	}
	m_bj.configure(m_order, ordDeps);
}

void WordCompiler::reset() {
	m_model->reset();
	m_bj.reset();
	m_domains = m_model->getInitWordsDomains();
}

bool WordCompiler::compileFrom(const uint32_t i) {

	// all variables instantiated?
	if (i == m_model->getWordsNum()) {

		// saves letters using completed words
		for (uint32_t li = 0; li < m_model->getLettersNum(); ++li) {
			Letter* const l = m_model->getLetter(li);

			// words this letter belongs to (NEVER empty)
			const list<LetterPosition>& letterWords = m_model->getLetterWords(li);

			// fw is the first word
			const LetterPosition& fLp = *letterWords.begin();
			const Word* const fw = m_model->getWord(fLp.getWordIndex());
			const uint32_t fwPos = fLp.getPosition();

			// sets letter as fw[fwPos]
			l->set(fw->getAt(fwPos));
		}

		return true;
	} else {

		// maps variable from ordering
		const uint32_t wi = m_order[i];
		Word* const w = m_model->getWord(wi);
		const uint32_t wLen = w->getLength();

		// admittable domain
		set<uint32_t> domainSet = m_domains[wi];
		if (isVerbose()) {
			*m_verboseOut << "pattern for " << wi << " = \'" << w->get() <<
					"\' (" << domainSet.size() << " matchings)" << endl;
		}

		// current and previous assignment
		string v, oldV;

		// removal stack through forward checking
		stack<pair<uint32_t, uint32_t> > remStack;

#ifdef USE_BJ_FAST
		// FC failures ignored
		set<uint32_t>* const failedPtr = 0;
#else
		// FC failures added for BJ
		set<uint32_t> failed;
		set<uint32_t>* const failedPtr = &failed;
#endif

		// iterates over domain
		while (!domainSet.empty()) {

			// chooses value to assign
			choose(wLen, &domainSet, &v);

			if (isVerbose()) {
				*m_verboseOut << "word " << wi << " = '" << v << "'" << endl;
			}

			// tries to assign v to current variable
			if (!assign(wi, v, &oldV, &remStack, failedPtr)) {
				retire(wi, oldV, &remStack);
				continue;
			}

			if (isVerbose()) {
				*m_verboseOut << endl;
				printModelGrid(*m_verboseOut, *m_model);
				*m_verboseOut << endl;
			}

			// recursively solved?
			if (compileFrom(i + 1)) {
				return true;
			} else {

				// retires variable
				retire(wi, oldV, &remStack);

#ifdef USE_BJ
				// returns if there are no more jumps or last
				// jump gets past current variable
				if (m_bj.isExhausted() ||
						(m_bj.getDestination() < i)) {
					return false;
				}
#endif
			}
		}

		if (isVerbose()) {
			*m_verboseOut << "word " << wi << " ... BACKTRACK!" << endl;
		}

		// algorithm fails iff first variable backtracks
		if (i > 0) {
#ifdef USE_BJ
			m_bj.jump(i, failedPtr);
#endif

			if (isVerbose()) {
				*m_verboseOut << "jump from " << m_order[m_bj.getOrigin()] <<
						" to " << m_order[m_bj.getDestination()] << endl;
				*m_verboseOut << endl;
			}
		}

		return false;
	}
}

void WordCompiler::choose(const uint32_t wLen, set<uint32_t>* const domainSet,
		string* const v) {
	uint32_t vi = 0;
	set<uint32_t>::iterator viIt;

#ifdef USE_BENCHMARK
	// ordered choice, no randomness
	viIt = domainSet->begin();
	vi = *viIt;
#else
	// random choice in matchings (TODO: a hash table would be
	// better for domains instead of a tree set)
	const uint32_t viPos = rand() % domainSet->size();
	uint32_t domPos = 0;
	viIt = domainSet->begin();
	while (domPos < viPos) {
		++domPos;
		++viIt;
	}
	vi = *viIt;
#endif

	// v is the dictionary mapping for vi
	const Dictionary* const d = m_model->getDictionary();
	*v = d->getWord(wLen, vi);

	// marks vi-th value as visited
	domainSet->erase(viIt);
}

bool WordCompiler::assign(const uint32_t wi, const string& v, string* const oldV,
		stack<pair<uint32_t, uint32_t> >* const remStack, set<uint32_t>* const failed) {

	// gets word object and its length
	Word* const w = m_model->getWord(wi);
	const uint32_t wLen = w->getLength();

	// assigns word value saving previous
	*oldV = w->get();
	w->set(v);
	w->doMatch();

	// excludes domain value from remaining words of same length
	if (isUnique()) {

		// excluded word ID
		const uint32_t excludedId = w->getFirstId();

		// selects words subset by length
		const map<uint32_t, set<uint32_t> >& wordsByLength =
				m_model->getWordsByLength();
		const set<uint32_t>& subset = wordsByLength.find(wLen)->second;

		// updates exclusion lists and recalculates domains
		set<uint32_t>::const_iterator slWiIt;
		for (slWiIt = subset.begin(); slWiIt != subset.end(); ++slWiIt) {
			const uint32_t slWi = *slWiIt;

			// skips current and previous words
			if (m_revOrder[slWi] <= m_revOrder[wi]) {
				continue;
			}

			// same length word
			Word* const slw = m_model->getWord(slWi);

			// excludes domain value (no need for rematch)
			set<uint32_t>* const slDom = &m_domains[slWi];
			const uint32_t slDomOldCount = slDom->size();
			slw->exclude(excludedId);
			slDom->erase(excludedId);

			// puts removed value on the stack
			remStack->push(make_pair(slWi, excludedId));

			if (isVerbose()) {
				const uint32_t slDomNewCount = slDom->size();
				const uint32_t slDomRemCount = slDomOldCount - slDomNewCount;

				if (slDomRemCount > 0) {
					*m_verboseOut << "\tword " << slWi <<
							": removed " << slDomRemCount << " matchings, ";
					*m_verboseOut << "now " << slDomNewCount << endl;
				}
			}

			// an empty domain implies failure
			if (slDom->empty()) {
#ifndef USE_BJ_FAST
				// adds failed variable order
				failed->insert(m_revOrder[slWi]);
#endif
				if (isVerbose()) {
					*m_verboseOut << "\tFC failed at " << slWi <<
						" (UNIQUE)" << endl;
				}
				return false;
			}
		}
	}

	// domains reduction
	const list<pair<uint32_t, WordCrossing> >& wiRevDeps = m_revDeps[wi];
	list<pair<uint32_t, WordCrossing> >::const_iterator dIt;
	for (dIt = wiRevDeps.begin(); dIt != wiRevDeps.end(); ++dIt) {

		// affected word
		const uint32_t dWi = dIt->first;
		const uint32_t pos = dIt->second.getPosition();
		const uint32_t dPos = dIt->second.getCPosition();
		Word* const dw = m_model->getWord(dWi);

		// sets shared character w[pos] at dw[dPos]
		dw->setAt(dPos, w->getAt(pos));

		// recalculates possible words
		dw->doMatch();

		// intersects domain saving removed values
		set<uint32_t>* const dDom = &m_domains[dWi];
		const uint32_t dDomOldCount = dDom->size();
		const Dictionary::MatchingResult* const dRes = dw->getMatchingResult();
		set<uint32_t> dRemDom;
		dRes->getIdsIntersection(dDom, &dRemDom);

		// puts removed values on the stack
		set<uint32_t>::const_iterator drIt;
		for (drIt = dRemDom.begin(); drIt != dRemDom.end(); ++drIt) {
			remStack->push(make_pair(dWi, *drIt));
		}

		if (isVerbose()) {
			const uint32_t dDomNewCount = dDom->size();
			const uint32_t dDomRemCount = dDomOldCount - dDomNewCount;

			if (dDomRemCount > 0) {
				*m_verboseOut << "\tword " << dWi <<
						": removed " << dDomRemCount << " matchings, ";
				*m_verboseOut << "now " << dDomNewCount << endl;
			}
		}

		// an empty domain implies failure
		if (dDom->empty()) {
#ifndef USE_BJ_FAST
			// adds failed variable order
			failed->insert(m_revOrder[dWi]);
#endif
			if (isVerbose()) {
				*m_verboseOut << "\tFC failed at " << dWi << endl;
			}
			return false;
		}
	}

	return true;
}

void WordCompiler::retire(const uint32_t wi, const string& oldV,
		stack<pair<uint32_t, uint32_t> >* const remStack) {

	// gets word object and its length
	Word* const w = m_model->getWord(wi);
	const uint32_t wLen = w->getLength();

	// domains update
	const list<pair<uint32_t, WordCrossing> >& wiRevDeps = m_revDeps[wi];
	list<pair<uint32_t, WordCrossing> >::const_iterator dIt;
	for (dIt = wiRevDeps.begin(); dIt != wiRevDeps.end(); ++dIt) {

		// affected word
		const uint32_t dWi = dIt->first;
		const uint32_t dPos = dIt->second.getCPosition();
		Word* const dw = m_model->getWord(dWi);

		// readmit domain value in same length words
		if (isUnique()) {

			// excluded word ID
			const uint32_t excludedId = w->getFirstId();

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
				slw->include(excludedId);
			}
		}

		// unsets shared character dw[dPos]
		dw->unsetAt(dPos);
	}

	// restores previous word value (indeed a pattern)
	w->set(oldV);

	// restores domains
	while (!remStack->empty()) {

		// newest removal
		const pair<uint32_t, uint32_t>& remTop = remStack->top();
		const uint32_t rWi = remTop.first;
		const uint32_t rValue = remTop.second;

		// readmit value
		m_domains[rWi].insert(rValue);

		// removes top
		remStack->pop();
	}
}
