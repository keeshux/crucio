#ifndef __COMMON_H
#define __COMMON_H

namespace crucio {
#ifdef WIN32
    typedef unsigned int uint;
#endif

    class CrucioException {
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

    class DictionaryException : public CrucioException {
    public:
        DictionaryException(const char* const what) : CrucioException(what) {
        }
    };

    class GridException : public CrucioException {
    public:
        GridException(const char* const what) : CrucioException(what) {
        }
    };
}

#endif
