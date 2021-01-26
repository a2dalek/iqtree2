//
//  parsimonysearchparameters.h
//  Created by James Barbetti on 19-Jan-2021.
//

#ifndef parsimonysearchparameters_h
#define parsimonysearchparameters_h

#include "phylotree.h"

class ParsimonySearchParameters
{
public:
    std::string name;
    intptr_t    iterations;
    bool        lazy_mode;
    intptr_t    radius;
};

#endif /* parsimonysearchparameters_h */