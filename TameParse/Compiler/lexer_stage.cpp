//
//  lexer_stage.cpp
//  Parse
//
//  Created by Andrew Hunter on 21/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

// TODO: it'd be nice to have a way of designing this so it's not dependent on the language_stage stage
//       (we don't do this at the moment to keep things reasonably simple, there's a lot of dependencies that means that DI
//       wouldn't really fix the problem, and would just create a new 'giant constructor of doom' problem)

#include <sstream>
#include "TameParse/Compiler/lexer_stage.h"

using namespace std;
using namespace dfa;
using namespace contextfree;
using namespace lr;
using namespace language;
using namespace compiler;

namespace compiler {
    /// \brief NDFA accept action that will sort actions generated by the language compiler appropriately
    ///
    /// Actions generated by the parser stage are ordered according first to the type of language unit they are defined
    /// in (weak keywords have the highest priority) and then by the symbol ID (ie, the order that they are defined within
    /// the unit, and the order that the units are defined in if there is more than one of the same type)
    class language_accept_action : public accept_action {
    private:
        language_unit::unit_type m_UnitType;
        bool m_IsWeak;
        
    public:
        /// \brief Creates a standard accept action for the specified symbol
        language_accept_action(int symbol, language_unit::unit_type unitType, bool isWeak)
        : accept_action(symbol)
        , m_UnitType(unitType)
        , m_IsWeak(isWeak) {
        }
        
        /// \brief For subclasses, allows the NDFA to clone this accept action for storage purposes
        virtual accept_action* clone() const {
            return new language_accept_action(symbol(), m_UnitType, m_IsWeak);
        }

        /// \brief Determines if this action is less important than another
        ///
        /// By default, actions with lower symbol IDs are more important than those with higher symbol IDs
        virtual bool operator<(const accept_action& compareTo) const {
            // Work out if this action is also one defined by the language
            const language_accept_action* compareToLanguageAction = dynamic_cast<const language_accept_action*>(&compareTo);
            
            // We are always higher priority than the standard set of accept actions
            if (!compareToLanguageAction) return true;

            // Weak actions have higher priority than strong ones
            if (compareToLanguageAction->m_IsWeak != m_IsWeak) {
                if (compareToLanguageAction->m_IsWeak)  return true;
                else                                    return false;
            }
            
            // Compare the unit types; these are ordered in priority order in the language_unit class (we are less important if our type is of a lower priority)
            if (compareToLanguageAction->m_UnitType < m_UnitType) return true;
            if (compareToLanguageAction->m_UnitType > m_UnitType) return false;
            
            // Compare symbols: lower symbol IDs have a higher priority
            return symbol() > compareTo.symbol();
        }
        
        /// \brief Determines if this action is equivalent to another
        virtual bool operator==(const accept_action* compareTo) const {
            // Definitely the same if the pointers are equal
            if (compareTo == this) return true;
            if (compareTo == NULL) return false;
            
            // Should have the same type as this action
            const language_accept_action* compareToLanguageAction = dynamic_cast<const language_accept_action*>(compareTo);
            if (!compareToLanguageAction) return false;
            
            // Should pass the standard equality test
            if (!accept_action::operator==(compareTo)) return false;
            
            // Unit types should be the same otherwise
            return m_UnitType == compareToLanguageAction->m_UnitType;
        }
    };
}

/// \brief Class that extends ndfa_regex to support taking expressions from a lexer_data object
class ndfa_lexer_compiler : public ndfa_regex {
private:
    /// \brief Item list in a lexer data item
    typedef lexer_data::item_list item_list;

    /// \brief The lexer data that should be used to compile expressions
    const lexer_data* m_Data;

public:
    ndfa_lexer_compiler(const lexer_data* data)
    : m_Data(data) { }

    /// \brief Compiles the value of a {} expression
    virtual bool compile_expression(const symbol_string& expression, builder& cons) {
        // Look up the expression in the lexer data
        const item_list& items = m_Data->get_expressions(convert_syms(expression));

        // Use the standard behaviour if we don't find any items
        if (items.empty()) return ndfa_regex::compile_expression(expression, cons);

        // Remember the current state of the builder
        bool isLower = cons.make_lowercase();
        bool isUpper = cons.make_uppercase();

        // Start a new subexpression
        cons.push();

        // The result can be any of the supplied items
        bool first = true;
        for (item_list::const_iterator item = items.begin(); item != items.end(); item++) {
            // Or items together
            if (!first) {
                cons.begin_or();
            }

            // Set case sensitivity
            if (item->case_insensitive) {
                cons.set_case_options(true, true);
            } else {
                // Preserve case sensitivity of the enclosing block when the symbols don't explicitly specify what to do
                // TODO: you can specifically say 'case sensitive lexer-symbols' but we treat that as a no-op for now
                cons.set_case_options(isLower, isUpper);
            }

            // Add as 
            switch (item->type) {
                case lexer_item::regex:
                    add_regex(cons, convert(item->definition));
                    break;

                case lexer_item::literal:
                    add_literal(cons, convert(item->definition));
                    break;
            }

            // No longer the first item
            first = false;
        }

        // Done: reset the constructor
        cons.set_case_options(isLower, isUpper);
        cons.pop();

        // Found an expression
        return true;
    }
};

/// \brief Creates a new lexer compiler
///
/// The compiler will not 'own' the objects passed in to this constructor; however, they must have a lifespan
/// that is at least as long as the compiler itself (it's safe to call the destructor but no other call if they
/// have been destroyed)
lexer_stage::lexer_stage(console_container& console, const std::wstring& filename, language_stage* languageCompiler)
: compilation_stage(console, filename)
, m_Language(languageCompiler)
, m_WeakSymbols(languageCompiler->grammar())
, m_Dfa(NULL)
, m_Lexer(NULL) {
}

/// \brief Destroys the lexer compiler
lexer_stage::~lexer_stage() {
    // Destroy the DFA if it exists
    if (m_Dfa) {
        delete m_Dfa;
    }
    
    if (m_Lexer) {
        delete m_Lexer;
    }
}

/// \brief Compiles the lexer
void lexer_stage::compile() {
    // Grab the input
    const lexer_data*       lex             = m_Language->lexer();
    terminal_dictionary*    terminals       = m_Language->terminals();
    const set<int>*         weakSymbolIds   = m_Language->weak_symbols();
    
    // Reset the weak symbols
    m_WeakSymbols = lr::weak_symbols(m_Language->grammar());
    
    // Sanity check
    if (!lex || !terminals || !weakSymbolIds) {
        cons().report_error(error(error::sev_bug, filename(), L"BUG_LEXER_BAD_PARAMETERS", L"Missing input for the lexer stage", position(-1, -1, -1)));
        return;
    }
    
    // Output a staging message
    cons().verbose_stream() << L"  = Constructing final lexer" << endl;

    // Create the ndfa
    typedef lexer_data::item_list item_list;
    ndfa_lexer_compiler*    stage0 = new ndfa_lexer_compiler(lex);

    ndfa::builder   ignoreBuilder   = stage0->get_cons();
    bool            firstIgnore     = true;
    int             ignoreSymbol    = -1;
    const set<int>* usedIgnored     = m_Language->used_ignored_symbols();

    ignoreBuilder.push();

    // Iterate through the definition lists for each item
    for (lexer_data::iterator itemList = lex->begin(); itemList != lex->end(); itemList++) {
        // Iterate through the individual definitions for this item
        for (item_list::const_iterator item = itemList->second.begin(); item != itemList->second.end(); item++) {
            // Ignore items without a valid accept action (this is a bug)
            if (item->definition_type == language_unit::unit_null) {
                cons().report_error(error(error::sev_bug, filename(), L"BUG_MISSING_ACTION", L"Missing action for lexer symbol", position(-1, -1, -1)));
                continue;
            }

            // Decide on a symbol ID
            int     symbolId    = item->symbol;
            bool    blandIgnore = false;

            // We modify ignore symbols if they aren't used in the grammar so that they all map to a single place
            // This may prove confusing if the user wishes to use the lexer independently
            if (item->definition_type == language_unit::unit_ignore_definition) {
                // If this is an ignored item with no syntactic meaning, give it the same symbol ID as the first ignored item we encountered
                if (usedIgnored->find(symbolId) == usedIgnored->end()) {
                    blandIgnore = true;
                    if (ignoreSymbol < 0) {
                        ignoreSymbol = symbolId;
                    } else {
                        symbolId = ignoreSymbol;
                    }
                }
            }

            // Add the corresponding items
            switch (item->type) {
                case lexer_item::regex:
                    if (blandIgnore) {
                        // Combine 'bland' ignored items into a single symbol
                        if (!firstIgnore) {
                            ignoreBuilder.begin_or();
                        }
                        ignoreBuilder.push();
                        ignoreBuilder.set_case_options(item->case_insensitive, item->case_insensitive);
                        stage0->add_regex(ignoreBuilder, item->definition);
                        ignoreBuilder.pop();

                        firstIgnore = false;
                    } else {
                        // Add as a new symbol
                        stage0->set_case_insensitive(item->case_insensitive);
                        stage0->add_regex(0, item->definition, language_accept_action(symbolId, item->definition_type, item->is_weak));
                    }
                    break;

                case lexer_item::literal:
                    if (blandIgnore) {
                        // Combine 'bland' ignored items into a single symbol
                        if (!firstIgnore) {
                            ignoreBuilder.begin_or();
                        }
                        ignoreBuilder.push();
                        ignoreBuilder.set_case_options(item->case_insensitive, item->case_insensitive);
                        stage0->add_literal(ignoreBuilder, item->definition);
                        ignoreBuilder.pop();

                        firstIgnore = false;
                    } else {
                        stage0->set_case_insensitive(item->case_insensitive);
                        stage0->add_literal(0, item->definition, language_accept_action(symbolId, item->definition_type, item->is_weak));
                    }
                    break;
            }
        }
    }

    // Finish the 'bland' ignore builder, if there were any 'bland' symbols
    if (!firstIgnore) {
        // Pop the entry on the stack
        ignoreBuilder.pop();

        // One or more items
        //const state& previous = ignoreBuilder.previous_state();
        //const state& current  = ignoreBuilder.current_state();
        
        // Can skip from current to previous (so this expression will repeat)
        //ignoreBuilder.goto_state(current);
        //ignoreBuilder >> previous >> epsilon();
        
        // Final state is the current state (and preserve the 'previous' state)
        //ignoreBuilder.goto_state(current, previous);

        // Set the symbol
        ignoreBuilder >> language_accept_action(ignoreSymbol, language_unit::unit_ignore_definition, false);
    }

    // Write out some stats about the ndfa
    cons().verbose_stream() << L"    Number states in the NDFA:              " << stage0->count_states() << endl;
    
    // Compile the NDFA to a NDFA without overlapping symbol sets
    dfa::ndfa* stage1 = stage0->to_ndfa_with_unique_symbols();
    
    if (!stage1) {
        cons().report_error(error(error::sev_bug, filename(), L"BUG_DFA_FAILED_TO_CONVERT", L"Failed to create an NDFA with unique symbols", position(-1, -1, -1)));
        return;
    }
    
    // Write some information about the first stage
    cons().verbose_stream() << L"    Initial number of character sets:       " << stage0->symbols().count_sets() << endl;
    cons().verbose_stream() << L"    Final number of character sets:         " << stage1->symbols().count_sets() << endl;

    delete stage0;
    stage0 = NULL;
    
    // Compile the NDFA to a DFA
    dfa::ndfa* stage2 = stage1->to_dfa();
    delete stage1;
    stage1 = NULL;
    
    if (!stage2) {
        cons().report_error(error(error::sev_bug, filename(), L"BUG_DFA_FAILED_TO_COMPILE", L"Failed to compile DFA", position(-1, -1, -1)));
        return;
    }
    
    // Identify any terminals that are always replaced by other terminals (warning)
    set<int>            unusedTerminals;
    map<int, set<int> > clashes;
    for (int terminalId = 0; terminalId < m_Language->terminals()->count_symbols(); terminalId++) {
        unusedTerminals.insert(terminalId);
    }
    
    for (int stateId = 0; stateId < stage2->count_states(); stateId++) {
        // Get the actions for this state
        const ndfa::accept_action_list& acceptActions = stage2->actions_for_state(stateId);
        
        // Ignore empty action sets
        if (acceptActions.empty()) continue;
        
        // Pick the highest action
        ndfa::accept_action_list::const_iterator action = acceptActions.begin();
        accept_action* highest = *action;
        
        action++;
        for (; action != acceptActions.end(); action++) {
            if (*highest < **action) {
                clashes[highest->symbol()].insert((*action)->symbol());
                highest = *action;
            } else {
                clashes[(*action)->symbol()].insert(highest->symbol());
            }
        }
        
        // Remove from the set of unused terminals
        unusedTerminals.erase(highest->symbol());
    }
    
    // Report warnings for any terminals that are never generated by the lexer
    for (set<int>::const_iterator unusedSymbol = unusedTerminals.begin(); unusedSymbol != unusedTerminals.end(); unusedSymbol++) {
        // Don't report ignored symbols if they can never be generated
        if (m_Language->ignored_symbols()->find(*unusedSymbol) != m_Language->ignored_symbols()->end()) {
            continue;
        }

        // Get the position of this terminal
        position        pos     = m_Language->terminal_definition_pos(*unusedSymbol);
        const wstring&  file    = m_Language->terminal_definition_file(*unusedSymbol);
        
        // Get the name of the terminal
        const wstring& name = m_Language->terminals()->name_for_symbol(*unusedSymbol);
        
        // Build the warning message
        wstringstream msg;
        msg << L"Lexer symbol can never be generated: " << name;
        
        cons().report_error(error(error::sev_warning, file, L"SYMBOL_CANNOT_BE_GENERATED", msg.str(), pos));
        
        // Get the symbols that clash with this one
        set<int>& clashSet = clashes[*unusedSymbol];
        if (!clashSet.empty()) {
            // Write out the symbols that are generated instead
            for (set<int>::iterator clashSymbol = clashSet.begin(); clashSymbol != clashSet.end(); clashSymbol++) {
                wstringstream msg2;
                msg2 << L"'" << name << L"' clashes with: " << m_Language->terminals()->name_for_symbol(*clashSymbol);
                cons().report_error(error(error::sev_detail, m_Language->terminal_definition_file(*clashSymbol), L"SYMBOL_CLASHES_WITH", msg2.str(), m_Language->terminal_definition_pos(*clashSymbol)));
            }
        }
    }
    
    // TODO: also identify any terminals that clash with terminals at the same level (warning)
    
    // Build up the weak symbols set if there are any
    if (weakSymbolIds->size() > 0) {
        // Build up the weak symbol set as a series of items
        item_set weakSymSet(m_Language->grammar());
        
        // Count how many symbols there were initially
        int initialSymCount = terminals->count_symbols();
        
        // Iterate through the symbol IDs
        for (set<int>::const_iterator weakSymId = weakSymbolIds->begin(); weakSymId != weakSymbolIds->end(); weakSymId++) {
            weakSymSet.insert(item_container(new terminal(*weakSymId), true));
        }
        
        // Add these symbols to the weak symbols object
        m_WeakSymbols.add_symbols(*stage2, weakSymSet, *terminals);
        
        // Display how many new terminal symbols were added
        int finalSymCount = terminals->count_symbols();
        
        cons().verbose_stream() << L"    Number of extra weak symbols:           " << finalSymCount - initialSymCount << endl;
    }
    
    // Compact the resulting DFA
    cons().verbose_stream() << L"    Number of states in the lexer DFA:      " << stage2->count_states() << endl;

    dfa::ndfa* stage3;

    if (cons().get_option(L"disable-compact-dfa").empty()) {
        stage3 = stage2->to_compact_dfa();
        delete stage2;
        stage2 = NULL;
    
        // Write some information about the DFA we just produced
        cons().verbose_stream() << L"    Number of states in the compacted DFA:  " << stage3->count_states() << endl;
    } else {
        stage3 = stage2;
        stage2 = NULL;
    }
    
    // Eliminate any unnecessary symbol sets
    dfa::ndfa* stage4;
    
    if (cons().get_option(L"disable-merged-dfa").empty()) {
        stage4 = stage3->to_ndfa_with_merged_symbols();
        delete stage3;
        stage3 = NULL;
    } else {
        stage4 = stage3;
        stage3 = NULL;
    }
    
    // Write some information about the DFA we just produced
    cons().verbose_stream() << L"    Number of symbols in the compacted DFA: " << stage4->symbols().count_sets() << endl;
    
    m_Dfa = stage4;
    
    // Build the final lexer
    m_Lexer = new lexer(*m_Dfa);
    
    // Write some parting words
    // (Well, this is really kibibytes but I can't take blibblebytes seriously as a unit of measurement)
    cons().verbose_stream() << L"    Approximate size of final lexer:        " << (m_Lexer->size() + 512) / 1024 << L" kilobytes" << endl;
}
