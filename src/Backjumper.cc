#include "Backjumper.h"

using namespace crucio;
using namespace std;

// all variables are indexes in m_order
Backjumper::Backjumper() :
        m_order(),
        m_revOrder(),
        m_deps(),
        m_jumps(),
        m_exhausted(true),
        m_origin(0),
        m_destination(0) {
}

void Backjumper::configure(const vector<uint32_t>& order,
        const vector<list<uint32_t> >& deps) {
    m_order = order;
    Walk::getReverseOrder(m_order, &m_revOrder);
    m_deps = deps;
}

// i = variable, failed = further failed variables
void Backjumper::jump(const uint32_t i, const set<uint32_t>* const failed) {

    // variable index
    const uint32_t vi = m_order[i];

    // jump origin
    m_origin = i;

    // dependencies iterator
    list<uint32_t>::const_iterator dIt;
    uint32_t ji;

    // adds dependencies
    const list<uint32_t>& iDeps = m_deps[vi];
    for (dIt = iDeps.begin(); dIt != iDeps.end(); ++dIt) {
        ji = m_revOrder[*dIt];

        // skips variables beyond current
        if (ji < i) {
            m_jumps.insert(ji);
        }
    }

    // adds further dependencies if any
    if (failed) {
        set<uint32_t>::const_iterator fIt;
        for (fIt = failed->begin(); fIt != failed->end(); ++fIt) {
            const uint32_t fVi = m_order[*fIt];

            // adds current failed variable dependencies
            const list<uint32_t>& fiDeps = m_deps[fVi];
            for (dIt = fiDeps.begin(); dIt != fiDeps.end(); ++dIt) {
                ji = m_revOrder[*dIt];

                // skips variables beyond current
                if (ji < i) {
                    m_jumps.insert(ji);
                }
            }
        }
    }

    // no more jumps?
    if (m_jumps.empty()) {
        m_exhausted = true;
    } else {

        // removes jumps beyond i
        while (*m_jumps.begin() >= i) {
            m_jumps.erase(m_jumps.begin());
        }

        // rechecks for jumps existence
        if (m_jumps.empty()) {
            m_exhausted = true;
        } else {
            m_exhausted = false;

            // jumps as high as possible
            m_destination = *m_jumps.begin();
            m_jumps.erase(m_jumps.begin());
        }
    }
}
