/*
 * Output.h
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
    void printInputDescription(std::ostream& out,
                               const Dictionary& d,
                               const Grid& g,
                               const std::string& fillType,
                               const std::string& walkType,
                               const bool unique,
                               const bool deterministic,
                               const uint32_t seed,
                               const bool verbose);
    void printModelDescription(std::ostream& out, const Model& m);
    void printModelGrid(std::ostream& out, const Model& m);
    void printOutput(std::ostream& out, const Model& m);

    // crucio output manager
    class Output {
    public:
        Output(const Model& m);
        Output(std::istream& in);

        void printRaw(std::ostream& out) const;
        void printLatex(std::ostream& out,
                        const bool solution = false,
                        const bool fillIn = false) const;

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
