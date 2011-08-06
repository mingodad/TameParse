//
//  compiler.h
//  Parse
//
//  Created by Andrew Hunter on 30/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _COMPILER_COMPILER_H
#define _COMPILER_COMPILER_H

#include "ContextFree/grammar.h"
#include "ContextFree/terminal_dictionary.h"
#include "Language/language_block.h"
#include "Dfa/ndfa_regex.h"

namespace compiler {
    ///
    /// \brief Class that handles compiling a language block into a lexer and a grammar
    ///
    /// This class handles the steps necessary to compile a language block with no inheritance into a grammar and a lexer.
    /// These can in turn be used to build a finished parser.
    ///
    class language_compiler {
    private:
        /// \brief The language block that this will compile
        language::language_block* m_Language;
        
        /// \brief The dictionary of terminals defined by the language
        contextfree::terminal_dictionary m_Terminals;
        
        /// \brief The lexer defined by the language (as a NDFA)
        dfa::ndfa_regex m_Lexer;
        
        /// \brief The grammar defined by the language
        contextfree::grammar m_Grammar;
    };
    
    // TODO: need to add virtual methods to make it possible to write a subclass that can deal with more complex languages 
    // with inheritance
}

#endif
