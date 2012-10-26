/*
 * LanguageDictionary.h
 * crucio
 *
 * Copyright 2012 Davide De Rosa
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

#ifndef __LANGUAGE_DICTIONARY_H
#define __LANGUAGE_DICTIONARY_H

#include "Dictionary.h"

namespace crucio {

    class WordSetIndex;
    
    class LanguageDictionary : public Dictionary {
    public:
        LanguageDictionary(const std::set<std::string>&);
        LanguageDictionary(const std::string&);
        virtual ~LanguageDictionary();
        
        const std::string& getFilename() const {
            return m_filename;
        }
        
        virtual bool getMatchings(const std::string&, MatchingResult* const,
                                  const std::set<uint32_t>* const = 0) const;
        
        virtual bool getPossible(const MatchingResult* const, const uint32_t,
                                 ABMask* const) const;
        
        virtual bool getPossible(const MatchingResult* const,
                                 std::vector<ABMask>* const) const;

    private:
        class MakeUpper {
        public:
            void operator()(char& ch) const {
                ch &= ~32;
            }
        };
        
        class IsNotAscii {
        public:
            bool operator()(const char ch) const {
                const char upperCh = ch & ~32;
                return ((upperCh < 'A') || (upperCh > 'Z'));
            }
        };
        
#ifdef CRUCIO_C_ARRAYS
        class MinSizePtr {
        public:
            bool operator()(const IDArray* const v1, const IDArray* const v2) const {
                return (v1->length < v2->length);
            }
        };
#else
        class MinSizePtr {
        public:
            bool operator()(const std::vector<uint32_t>* const v1,
                            const std::vector<uint32_t>* const v2) const {
                return (v1->size() < v2->size());
            }
        };
#endif
        
        // origin filename (if any)
        const std::string m_filename;
        
        // input validation
        static bool isValidWord(const std::string&);
    };
}

#endif
