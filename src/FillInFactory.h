/*
 * FillInFactory.h
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

#ifndef __FILLIN_FACTORY_H
#define __FILLIN_FACTORY_H

#include "common.h"

namespace crucio
{
    typedef struct {
        unsigned rows;
        unsigned columns;
        unsigned minLength;
        unsigned maxLength;
    } GridStructure;
    
    class FillIn
    {
    public:
        FillIn(const GridStructure &structure);
        ~FillIn();
        
        void layout();
        void complete();
        
    private:
        const GridStructure m_structure;
    };
}

std::ostream &operator<<(std::ostream &out, const crucio::FillIn &fi);

#endif
