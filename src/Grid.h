/*
 * Grid.h
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

#ifndef __GRID_H
#define __GRID_H

#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "common.h"

namespace crucio
{
    class Grid;
    class Definition;

    struct CellAddress
    {
        unsigned m_row;
        unsigned m_column;
    };
    
    struct GridStructure
    {
        unsigned m_rows;
        unsigned m_columns;
        unsigned m_minLength;
        unsigned m_maxLength;
    };
    
    class Cell
    {
    public:
        friend class Grid;

        static const char WHITE = '-';
        static const char BLACK = '#';

        static bool isLegal(const char ch) {
            return (strchr(sm_legalValues, ch) != NULL);
        }

        // parent grid
        const Grid* getParent() const {
            return m_parent;
        }

        // cell details
        uint32_t getRow() const {
            return m_row;
        }
        uint32_t getColumn() const {
            return m_column;
        }
        char getValue() const {
            return m_value;
        }
        bool isWhite() const {
            return (m_value == WHITE);
        }
        bool isBlack() const {
            return (m_value == BLACK);
        }
        bool isFixed() const {
            return (!isWhite() && !isBlack());
        }
        bool isCrossing() const {
            return m_crossing;
        }

        // adjacent cells
        const Cell* getNorth() const;
        const Cell* getWest() const;
        const Cell* getSouth() const;
        const Cell* getEast() const;

        // starting definitions
        const Definition* getAcrossDef() const {
            return m_acrossDef;
        }
        const Definition* getDownDef() const {
            return m_downDef;
        }

        // adjacent non-black cells
        const std::list<const Cell*>& getNearCells() const {
            return m_nearCells;
        }

    private:
        static const char sm_legalValues[];

        const Grid* const m_parent;

        const uint32_t m_row;
        const uint32_t m_column;
        const char m_value;
        const bool m_crossing;

        const Definition* m_acrossDef;
        const Definition* m_downDef;
        std::list<const Cell*> m_nearCells;

        Cell(const Grid* const parent,
             const uint32_t row,
             const uint32_t column,
             const char value,
             const bool crossing);

        // called by grid after ctor
        void setAcrossDef(const Definition* const acrossDef) {
            m_acrossDef = acrossDef;
        }
        void setDownDef(const Definition* const downDef) {
            m_downDef = downDef;
        }
        void calculateNearCells();
    };

    class Definition
    {
    public:
        friend class Grid;

        enum Direction {
            ACROSS,
            DOWN
        };

        // parent grid
        const Grid* parent() const {
            return m_parent;
        }

        // definition details
        uint32_t getIndex() const {
            return m_index;
        }
        Direction getDirection() const {
            return m_direction;
        }
        uint32_t getNumber() const {
            return m_number;
        }
        uint32_t getLength() const {
            return m_length;
        }

        // starting cell
        const Cell* getStartCell() const {
            return m_startCell;
        }

        // crossing definitions
        const std::list<std::pair<uint32_t, std::pair<const Definition*, uint32_t> > >&
        getCrossingDefinitions() const {

            return m_crossingDefinitions;
        }

    private:
        const Grid* const m_parent;

        const uint32_t m_index;
        const Direction m_direction;
        const uint32_t m_number;
        const uint32_t m_length;

        const Cell* m_startCell;
        std::list<std::pair<uint32_t, std::pair<const Definition*, uint32_t> > >
        m_crossingDefinitions;

        Definition(const Grid* const parent,
                   const uint32_t index,
                   const Direction dir,
                   const uint32_t number,
                   const uint32_t length);

        // called by grid after ctor
        void setStartCell(const Cell* const startCell) {
            m_startCell = startCell;
        }
        void calculateCrossingDefinitions();
    };

    class CellCompare
    {
    public:
        bool operator()(const Cell* const cl1,
                        const Cell* const cl2) const {
            return (std::make_pair(cl1->getRow(), cl1->getColumn()) <
                    std::make_pair(cl2->getRow(), cl2->getColumn()));
        }
    };

    class DefinitionCompare
    {
    public:
        bool operator()(const Definition* const d1,
                        const Definition* const d2) const {
            return (std::make_pair(d1->getNumber(), d1->getDirection()) <
                    std::make_pair(d2->getNumber(), d2->getDirection()));
        }
    };

    class Grid
    {
    public:

        // prevents incorrect rows/columns format too
        static const uint32_t MIN_SIZE = 2;
        static const uint32_t MAX_SIZE = 666;

        Grid(const char** charsGrid,
             const uint32_t rows, const uint32_t columns);
        Grid(const std::string& filename);
        ~Grid();

        // grid details
        const std::string& getFilename() const {
            return m_filename;
        }
        uint32_t getRows() const {
            return m_rows;
        }
        uint32_t getColumns() const {
            return m_columns;
        }
        uint32_t getWhiteCells() const {
            return m_whiteCells;
        }
        uint32_t getBlackCells() const {
            return m_blackCells;
        }
        uint32_t getFixedCells() const {
            return m_fixedCells;
        }
        uint32_t getNonBlackCells() const {
            return (m_whiteCells + m_fixedCells);
        }
        uint32_t getWords() const {
            return m_words;
        }
        uint32_t getCrossings() const {
            return m_crossings;
        }

        // cells traversal
        const Cell* getCell(const uint32_t i, const uint32_t j) const {
            return m_cells[i][j];
        }

        // always non-NULL since grid has at least ONE non-black cell
        const Cell* getFirstNonBlackCell() const {

            // returns first non-black cell
            for (uint32_t i = 0; i < m_rows; ++i) {
                for (uint32_t j = 0; j < m_columns; ++j) {
                    const Cell* const cl = m_cells[i][j];
                    if (!cl->isBlack()) {
                        return cl;
                    }
                }
            }

            // avoids warning
            return 0;
        }

        // always non-NULL since grid has at least ONE definition
        const Definition* getFirstDefinition() const {
            const Cell* const firstCl = getFirstNonBlackCell();

            // at least one of them is non-NULL since grid is connected
            return (firstCl->getAcrossDef() ?
                    firstCl->getAcrossDef() : firstCl->getDownDef());
        }

        // definitions vectors
        const std::vector<Definition*>& getAcrossDefinitions() const {
            return m_acrossDefinitions;
        }
        const std::vector<Definition*>& getDownDefinitions() const {
            return m_downDefinitions;
        }

        // statistics
        double getInterlockDegree() const {
            return ((double) m_crossings /
                    (m_whiteCells + m_fixedCells)) * 100;
        }

    private:
        void initGrid(const char** charsGrid,
                      const uint32_t rows, const uint32_t columns);
        static uint32_t getReachableFrom(const Cell* const cl,
                                         std::set<const Cell*, CellCompare>* const visited);

        const std::string m_filename;
        uint32_t m_rows;
        uint32_t m_columns;
        uint32_t m_whiteCells;
        uint32_t m_blackCells;
        uint32_t m_fixedCells;
        uint32_t m_words;
        uint32_t m_crossings;

        // contents
        std::vector<std::vector<Cell*> > m_cells;
        std::vector<Definition*> m_acrossDefinitions;
        std::vector<Definition*> m_downDefinitions;

        // checks for isolated cells
        bool isConnected() const;
    };
}

std::ostream& operator<<(std::ostream& out, const crucio::Grid& g);
std::ostream& operator<<(std::ostream& out, const crucio::Definition& def);

#endif
