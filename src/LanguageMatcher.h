//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#ifndef __LANGUAGE_MATCHER_H
#define __LANGUAGE_MATCHER_H

#include "Dictionary.h"

namespace crucio
{

    class LanguageMatcher : public Matcher
    {
    public:
        LanguageMatcher(const std::set<std::string>* const words);
        LanguageMatcher(const std::string& filename);
        virtual ~LanguageMatcher();

        const std::string& getFilename() const {
            return m_filename;
        }

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
        class MakeUpper
        {
        public:
            void operator()(char& ch) const {
                ch &= ~32;
            }
        };

        class IsNotAscii
        {
        public:
            bool operator()(const char ch) const {
                const char upperCh = ch & ~32;
                return ((upperCh < 'A') || (upperCh > 'Z'));
            }
        };

#ifdef CRUCIO_C_ARRAYS
        class MinSizePtr
        {
        public:
            bool operator()(const IDArray* const v1, const IDArray* const v2) const {
                return (v1->length < v2->length);
            }
        };
#else
        class MinSizePtr
        {
        public:
            bool operator()(const std::vector<uint32_t>* const v1,
                            const std::vector<uint32_t>* const v2) const {

                return (v1->size() < v2->size());
            }
        };
#endif

        // origin word list or filename (if any)
        const std::set<std::string>* m_words;
        const std::string m_filename;

        // cached alphabet
        Alphabet m_alphabet;

        // input validation
        static bool isValidWord(const std::string& word);

        // subroutines
        void loadWords(WordSetIndex *const wsIndex) const;
        void loadFilename(WordSetIndex *const wsIndex) const;
    };
}

#endif
