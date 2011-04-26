//
//  dfa_symbol_deduplicate.cpp
//  Parse
//
//  Created by Andrew Hunter on 26/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "dfa_symbol_deduplicate.h"
#include "Dfa/remapped_symbol_map.h"

using namespace dfa;

void test_dfa_symbol_deduplicate::run_tests() {
    symbol_map has_duplicates1;
    
    int firstSet    = has_duplicates1.identifier_for_symbols(range<int>(0, 20));
    int secondSet   = has_duplicates1.identifier_for_symbols(range<int>(10, 30));
    
    remapped_symbol_map* no_duplicates = remapped_symbol_map::deduplicate(has_duplicates1);
    
    int  count      = 0;
    int  extras     = 0;
    bool has0to10   = false;
    bool has10to20  = false;
    bool has20to30  = false;
    
    for (symbol_map::iterator it = no_duplicates->begin(); it != no_duplicates->end(); it++) {
        count++;

        remapped_symbol_map::new_symbols newSyms = no_duplicates->old_symbols(it->second);

        if (it->first == range<int>(0, 10)) {
            has0to10 = true;
            report("OldSet1", newSyms.find(firstSet) != newSyms.end() && newSyms.find(secondSet) == newSyms.end() && newSyms.size() == 1);
        } else if (it->first == range<int>(10, 20)) {
            has10to20 = true;
            report("OldSet2", newSyms.find(firstSet) != newSyms.end() && newSyms.find(secondSet) != newSyms.end() && newSyms.size() == 2);
        } else if (it->first == range<int>(20, 30)) {
            has20to30 = true;
            report("OldSet3", newSyms.find(firstSet) == newSyms.end() && newSyms.find(secondSet) != newSyms.end() && newSyms.size() == 1);
        } else {
            extras++;
        }
    }
    
    report("HasAllSets1", has0to10 && has10to20 && has20to30);
    report("NoExtras1", extras == 0 && count == 3);
    
    delete no_duplicates;
}