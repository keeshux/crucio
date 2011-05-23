#ifndef __COMPILER_H
#define __COMPILER_H

#include "Model.h"
#include "Output.h"
#include "Walk.h"

namespace crucio {
	class Compiler {
	public:
		enum Result {
			SUCCESS,
			FAILURE_IMPOSSIBLE,
			FAILURE_OVERCONSTRAINED,
			FAILURE_ND_GRID
		};

		Compiler();
		virtual ~Compiler();

		// no repeated words constraint
		void setUnique(const bool unique) {
			m_unique = unique;
		}
		bool isUnique() const {
			return m_unique;
		}

		// deterministic solution constraint
		void setDeterministic(const bool deterministic) {
			m_deterministic = deterministic;
		}
		bool isDeterministic() const {
			return m_deterministic;
		}

		// prints out algorithmic steps
		void setVerbose(const bool verbose, std::ostream* const out) {
			m_verbose = verbose;
			m_verboseOut = out;
		}
		bool isVerbose() const {
			return m_verbose;
		}

		// algorithm execution
		Result compile(Model* const, const Walk&);

	protected:

		// callbacks
		virtual void configure(const Walk&) = 0;
		virtual void reset() = 0;
		virtual bool compileFrom(const uint32_t) = 0;

	private:

		// used by determinism check
		class IsSimpleDomain {
		public:

			// <length, words-by-length> pair
			bool operator()(const std::pair<uint32_t,
					std::set<uint32_t> >& lenWords) const {
				return (lenWords.second.size() == 1);
			}

			// a pattern's domain cardinality is matching words count
			bool operator()(const Word* const w) const {
				return (w && (w->getMatchingResult()->getSize() == 1));
			}
		};

		// parameters
		bool m_unique;
		bool m_deterministic;
		bool m_verbose;

		// determinism check
		bool isDeterministicSolution() const;

	protected:

		// model reference
		Model* m_model;

		// verbose output stream reference
		std::ostream* m_verboseOut;
	};
}

#endif
