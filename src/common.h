//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#ifndef __COMMON_H
#define __COMMON_H

#ifndef CRUCIO_C_ARRAYS
#define CRUCIO_C_ARRAYS
#endif

//#define CRUCIO_BENCHMARK
#define CRUCIO_BJ
#define CRUCIO_BJ_FAST

#include <cassert>
#include <iostream>
#include <bitset>

namespace crucio
{
#ifdef WIN32
    typedef unsigned int uint;
#endif

    template<typename Ch, typename Traits = std::char_traits<Ch> >
    struct basic_nullbuf : std::basic_streambuf<Ch, Traits> {
        typedef std::basic_streambuf<Ch, Traits> base_type;
        typedef typename base_type::int_type int_type;
        typedef typename base_type::traits_type traits_type;
        
        virtual int_type overflow(int_type c) {
            return traits_type::not_eof(c);
        }
    };
    
    // convenient typedefs
    typedef basic_nullbuf<char> nullbuf;
    typedef basic_nullbuf<wchar_t> wnullbuf;
    
    extern std::ostream cnull;
    extern std::wostream wcnull;
    
    /* logging */
    
    extern std::ostream* crucio_vout;
    
    inline bool isVerbose()
    {
        return (crucio_vout == &std::cerr);
    }
    inline void setVerbose(const bool verbose)
    {
        if (verbose) {
            crucio_vout = &std::cerr;
        } else {
            crucio_vout = &cnull;
        }
    }

    /* global alphabet management (IMPORTANT: only uppercase letters!) */

    // alphabet kind (base character)
    enum Alphabet {
        LETTERS = 'A',
        DIGITS = '0'
    };

    // alphabet/domain size
    enum AlphabetSize {
        LETTERS_COUNT = 26,
        DIGITS_COUNT = 10
    };

    // domain mask to hold max alphabet size
    typedef std::bitset<LETTERS_COUNT> ABMask; // max(letters, digits)

    // wildcard (any characater)
    const char ANY_CHAR = '-';

    // size by alphabet
    inline uint32_t alphabetSize(const Alphabet alphabet)
    {
        switch (alphabet) {
        case LETTERS:
            return LETTERS_COUNT;

        case DIGITS:
            return DIGITS_COUNT;
        }

        // should never happen
        assert(false);
        return 0;
    }

    // totally active mask
    inline ABMask anyMask(const Alphabet alphabet)
    {
        ABMask mask;
        const uint32_t size = alphabetSize(alphabet);
        for (size_t i = 0; i < size; ++i) {
            mask.set(i);
        }
        return mask;
    }
    inline void setAnyMask(const Alphabet alphabet, ABMask* const mask)
    {
        const uint32_t size = alphabetSize(alphabet);
        for (size_t i = 0; i < size; ++i) {
            mask->set(i);
        }
    }

    // maps i to i-th letter of alphabet
    inline char index2Character(const Alphabet alphabet, const uint32_t i)
    {
        return (char)(alphabet + i);
    }

    // maps ch to its alphabet index
    inline uint32_t character2Index(const Alphabet alphabet, const char ch)
    {
        return (ch - alphabet);
    }

    // ABMask string representation by alphabet
    std::string ABMaskString(const Alphabet alphabet, const ABMask mask);

    /* exceptions */

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
