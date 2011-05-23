#ifndef __OUTPUT_H
#define __OUTPUT_H

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "endian.h"
#include "Model.h"

namespace crucio {

	// formatted output functions
	void printInputDescription(std::ostream&, const Dictionary&,
			const Grid&, const std::string&, const std::string&,
			const bool, const bool, const uint32_t, const bool);
	void printModelDescription(std::ostream&, const Model&);
	void printModelGrid(std::ostream&, const Model&);
	void printOutput(std::ostream&, const Model&);

	// crucio output manager
	class Output {
	public:
		Output(const Model&);
		Output(std::istream&);

		void printRaw(std::ostream&) const;
		void printLatex(std::ostream&, const bool = false,
				const bool = false) const;

	private:
		class CellData {
		public:
			CellData() :
					m_number(0),
					m_unfilled('\0'),
					m_filled('\0') {
			}

			uint32_t m_number;
			char m_unfilled;
			char m_filled;
		};

		class DefinitionData {
		public:
			DefinitionData(const Definition::Direction dir, const uint32_t number,
					const uint32_t startRow, const uint32_t startColumn,
					const std::string& str) :
					m_direction(dir),
					m_number(number),
					m_startRow(startRow),
					m_startColumn(startColumn),
					m_string(str) {
			}

			const Definition::Direction getDirection() const {
				return m_direction;
			}
			const uint32_t getNumber() const {
				return m_number;
			}
			const uint32_t getStartRow() const {
				return m_startRow;
			}
			const uint32_t getStartColumn() const {
				return m_startColumn;
			}
			const std::string& getString() const {
				return m_string;
			}

			bool operator<(const DefinitionData& dd) const {
				return (std::make_pair(m_direction, m_number) <
						std::make_pair(dd.m_direction, dd.m_number));
			}

		private:
			Definition::Direction m_direction;
			uint32_t m_number;
			uint32_t m_startRow;
			uint32_t m_startColumn;
			std::string m_string;
		};

		class IsDown {
		public:
			bool operator()(const DefinitionData& dd) const {
				return (dd.getDirection() == Definition::DOWN);
			}
		};

		uint32_t m_rows;
		uint32_t m_columns;
		std::vector<std::vector<CellData> > m_cellsData;
		std::set<DefinitionData> m_defsData;
	};
}

#endif
