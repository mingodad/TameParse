//
//  compiler.h
//  Parse
//
//  Created by Andrew Hunter on 30/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _COMPILER_LANGUAGE_COMPILER_H
#define _COMPILER_LANGUAGE_COMPILER_H

#include <set>

#include "ContextFree/grammar.h"
#include "ContextFree/terminal_dictionary.h"
#include "Language/language_block.h"
#include "Dfa/ndfa_regex.h"
#include "Compiler/compilation_stage.h"

namespace compiler {
    ///
    /// \brief Class that handles compiling a language block into a lexer and a grammar
    ///
    /// This class handles the steps necessary to compile a language block with no inheritance into a grammar and a lexer.
    /// These can in turn be used to build a finished parser.
    ///
    class language_compiler : public compilation_stage {
    private:
        /// \brief The language block that this will compile
        const language::language_block* m_Language;
        
        /// \brief The dictionary of terminals defined by the language
        contextfree::terminal_dictionary m_Terminals;
        
        /// \brief The lexer defined by the language (as a NDFA)
        dfa::ndfa_regex m_Lexer;
        
        /// \brief The grammar defined by the language
        contextfree::grammar m_Grammar;
        
        /// \brief The IDs of symbols defined as being 'weak'
        std::set<int> m_WeakSymbols;
        
        /// \brief The IDs of symbols defined as being 'ignored'
        std::set<int> m_IgnoredSymbols;
        
    public:
        /// \brief Creates a compiler that will compile the specified language block
        language_compiler(console_container& console, const std::wstring& filename, const language::language_block* block);
        
        /// \brief Destructor
        virtual ~language_compiler();
        
        /// \brief Compiles the language, creating the dictionary of terminals, the lexer and the grammar
        void compile();
        
    private:
        /// \brief Adds any lexer items that are defined by a specific EBNF item to this object
        ///
        /// Returns the number of new items that were defined
        int add_ebnf_lexer_items(language::ebnf_item* item);

        /// \brief Compiles an EBNF item from the language into a context-free grammar item onto the end of the specified rule
        ///
        /// The lexer items should already be compiled before this call is made; it's a bug if any terminal items are found
        /// to be missing from the terminal dictionary.
        void compile_item(contextfree::rule& target, language::ebnf_item* item);
    };
    
    // TODO: need to add virtual methods to make it possible to write a subclass that can deal with more complex languages 
    // with inheritance
}

#endif
