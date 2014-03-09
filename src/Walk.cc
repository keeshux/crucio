//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#include "Walk.h"

using namespace crucio;
using namespace std;

/* Walk */

void Walk::getReverseOrder(const vector<uint32_t>& order,
                           vector<uint32_t>* const revOrder)
{
    uint32_t li, ordLi;

    // order maps (variable order -> variable index)
    // revOrder maps (variable index -> variable order)
    revOrder->resize(order.size());
    for (ordLi = 0; ordLi < order.size(); ++ordLi) {
        li = order[ordLi];
        (*revOrder)[li] = ordLi;
    }
}

/* BFSWalk */

void BFSWalk::visitLetters(const Model& m, vector<uint32_t>* const order) const
{
    const Cell* const root = m.getGrid()->getFirstNonBlackCell();

    // reserves enough space for order vector
    order->clear();
    order->reserve(m.getLettersNum());

    // data structures
    queue<const Cell*> toVisit;
    set<const Cell*, CellCompare> visited;

    // adds root cell
    toVisit.push(root);
    visited.insert(root);

    // bfs algorithm
    while (!toVisit.empty()) {

        // reads first cell from queue
        const Cell* const v = toVisit.front();

        // visits associated variable
        const int li = m.getLetterIndexByPos(v->getRow(), v->getColumn());
        order->push_back(li);
        toVisit.pop();

        // enumerates cell neighbourhood
        const list<const Cell*>& adjacencies = v->getNearCells();

        // explores adjacencies
        list<const Cell*>::const_iterator adjIt;
        for (adjIt = adjacencies.begin(); adjIt !=
                adjacencies.end(); ++adjIt) {
            const Cell* const adjCl = *adjIt;

            // checks for cell not to have been visited already
            const pair<set<const Cell*,
                  CellCompare>::iterator, bool> res =
                      visited.insert(adjCl);
            if (res.second) {

                // cell can now be put into the queue
                toVisit.push(adjCl);
            }
        }
    }
}

void BFSWalk::visitWords(const Model& m, vector<uint32_t>* const order) const
{
    const Definition* const root = m.getGrid()->getFirstDefinition();

    // reserves enough space for order vector
    order->clear();
    order->reserve(m.getWordsNum());

    // data structures
    queue<const Definition*> toVisit;
    set<const Definition*, DefinitionCompare> visited;

    // adds root definition
    toVisit.push(root);
    visited.insert(root);

    // bfs algorithm
    while (!toVisit.empty()) {

        // reads first definition from queue
        const Definition* const v = toVisit.front();

        // visits associated variable
        const Cell* const vCl = v->getStartCell();
        const int wi = m.getWordIndexByPos(v->getDirection(),
                                           vCl->getRow(), vCl->getColumn());
        order->push_back(wi);
        toVisit.pop();

        // enumerates definition neighbourhood
        const list<pair<uint32_t, pair<const Definition*, uint32_t> > >&
        adjacencies = v->getCrossingDefinitions();

        // explores adjacencies
        list<pair<uint32_t,
             pair<const Definition*, uint32_t> > >::const_iterator adjIt;
        for (adjIt = adjacencies.begin(); adjIt !=
                adjacencies.end(); ++adjIt) {
            const Definition* const adjDef = adjIt->second.first;

            // checks for definition not to have been visited already
            const pair<set<const Definition*,
                  DefinitionCompare>::iterator, bool> res =
                      visited.insert(adjDef);
            if (res.second) {

                // definition can now be put into the queue
                toVisit.push(adjDef);
            }
        }
    }
}

/* DFSWalk */

void DFSWalk::visitLetters(const Model& m, vector<uint32_t>* const order) const
{
    const Cell* const root = m.getGrid()->getFirstNonBlackCell();

    // reserves enough space for order vector
    order->clear();
    order->reserve(m.getLettersNum());

    // starts recursive dfs
    set<const Cell*, CellCompare> visited;
    recursiveVisitLetters(m, root, order, &visited);
}

void DFSWalk::visitWords(const Model& m, vector<uint32_t>* const order) const
{
    const Definition* const root = m.getGrid()->getFirstDefinition();

    // reserves enough space for order vector
    order->clear();
    order->reserve(m.getWordsNum());

    // starts recursive dfs
    set<const Definition*, DefinitionCompare> visited;
    recursiveVisitWords(m, root, order, &visited);
}

void DFSWalk::recursiveVisitLetters(const Model& m,
                                    const Cell* const v, vector<uint32_t>* const order,
                                    set<const Cell*, CellCompare>* const visited)
{
    // skips visited cells
    const pair<set<const Cell*,
          CellCompare>::iterator, bool> res =
              visited->insert(v);
    if (!res.second) {
        return;
    }

    // visits associated variable
    const int li = m.getLetterIndexByPos(v->getRow(), v->getColumn());
    order->push_back(li);

    // enumerates cell neighbourhood
    const list<const Cell*>& adjacencies = v->getNearCells();

    // recursively gets into adjacencies
    list<const Cell*>::const_iterator adjIt;
    for (adjIt = adjacencies.begin(); adjIt != adjacencies.end(); ++adjIt) {
        const Cell* const adjCl = *adjIt;
        recursiveVisitLetters(m, adjCl, order, visited);
    }
}

void DFSWalk::recursiveVisitWords(const Model& m,
                                  const Definition* const v, vector<uint32_t>* const order,
                                  set<const Definition*, DefinitionCompare>* const visited)
{
    // skips visited definitions
    const pair<set<const Definition*,
          DefinitionCompare>::iterator, bool> res =
              visited->insert(v);
    if (!res.second) {
        return;
    }

    // visits associated variable
    const Cell* const vCl = v->getStartCell();
    const int wi = m.getWordIndexByPos(v->getDirection(),
                                       vCl->getRow(), vCl->getColumn());
    order->push_back(wi);

    // enumerates definition neighbourhood
    const list<pair<uint32_t, pair<const Definition*, uint32_t> > >&
    adjacencies = v->getCrossingDefinitions();

    // recursively gets into adjacencies
    list<pair<uint32_t, pair<const Definition*, uint32_t> > >::const_iterator adjIt;
    for (adjIt = adjacencies.begin(); adjIt != adjacencies.end(); ++adjIt) {
        const Definition* const adjDef = adjIt->second.first;
        recursiveVisitWords(m, adjDef, order, visited);
    }
}
