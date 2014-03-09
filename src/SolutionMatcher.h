//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#ifndef __SOLUTION_MATCHER_H
#define __SOLUTION_MATCHER_H

#include "Dictionary.h"

namespace crucio
{

    class SolutionMatcher : public Matcher
    {
    public:
        SolutionMatcher();
        virtual ~SolutionMatcher();

        virtual void loadIndex(WordSetIndex* const wsIndex);

        virtual bool getMatchings(WordSetIndex* const wsIndex,
                                  Word* const word);

        virtual bool getPossible(WordSetIndex* const wsIndex,
                                 Word* const word);

        virtual uint32_t addCustomWord(const std::string& word);
        virtual const std::string& getCustomWord(const uint32_t id) const;
        virtual uint32_t getCustomWordID(const std::string& word) const;
        virtual uint32_t removeCustomWordID(const uint32_t id);

    private:
        std::map<uint32_t, std::string> m_customWords;
        std::map<std::string, uint32_t> m_customIDs;
        uint32_t m_lastWordID;

        static bool isMatchingExclusion(const std::string& pattern,
                                        const std::string& exclusion);
    };
}

#endif
