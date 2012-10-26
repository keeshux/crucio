/*
 * common.h
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

#ifndef __COMMON_H
#define __COMMON_H

#ifndef CRUCIO_C_ARRAYS
#define CRUCIO_C_ARRAYS
#endif

//#define CRUCIO_BENCHMARK
#define CRUCIO_BJ
#define CRUCIO_BJ_FAST

namespace crucio
{
#ifdef WIN32
    typedef unsigned int uint;
#endif

    enum Alphabet {
        LETTERS = 'A',
        DIGITS = '0'
    };
    
    class CrucioException
    {
    public:
        CrucioException(const char* const what) :
            m_what(what) {
        }

        const char* what() const throw() {
            return m_what;
        }

    private:
        const char* const m_what;
    };

    class DictionaryException : public CrucioException
    {
    public:
        DictionaryException(const char* const what) : CrucioException(what) {
        }
    };

    class GridException : public CrucioException
    {
    public:
        GridException(const char* const what) : CrucioException(what) {
        }
    };
}

#endif
