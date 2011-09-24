//
//  output_stage.h
//  Parse
//
//  Created by Andrew Hunter on 29/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _COMPILER_OUTPUT_STAGE_H
#define _COMPILER_OUTPUT_STAGE_H

#include <string>

#include "TameParse/Dfa/range.h"

#include "TameParse/Contextfree/grammar.h"
#include "TameParse/Contextfree/terminal_dictionary.h"

#include "TameParse/Lr/lalr_builder.h"
#include "TameParse/Lr/parser_tables.h"

#include "TameParse/Compiler/compilation_stage.h"
#include "TameParse/Compiler/language_stage.h"
#include "TameParse/Compiler/lr_parser_stage.h"
#include "TameParse/Compiler/lexer_stage.h"

namespace compiler {
	///
	/// \brief Base class for a compilation stage that produces output in a given language
	///
	class output_stage : public compilation_stage {
	private:
		/// \brief The lexer stage that should be compiled
		lexer_stage* m_LexerStage;

		/// \brief The language stage that should be compiled
		language_stage* m_LanguageStage;

		/// \brief The LR parser that should be compiled
		lr_parser_stage* m_ParserStage;

	public:
		/// \brief Creates a new output stage
		output_stage(console_container& console, const std::wstring& filename, lexer_stage* lexer, language_stage* language, lr_parser_stage* parser);

		/// \brief Destructor
		virtual ~output_stage();

		/// \brief Compiles the parser specified by this stage
		///
		/// Subclasses can override this if they want to substantially change the way that the
		/// compiler is generated.
		virtual void compile();

	protected:
		// Higher-level stages of the process

		/// \brief Defines the symbols associated with this language
		virtual void define_symbols();

		/// \brief Writes out the lexer tables (the symbol map and the state table)
		virtual void define_lexer_tables();

		/// \brief Writes out the parser tables
		virtual void define_parser_tables();

		/// \brief Writes out the AST tables
		virtual void define_ast_tables();

	protected:
		// Functions that represent various steps of the output of a language.
		// These are intended to make it easy to write out a file in the specified language.
		// They do nothing in this base class (but they are not abstract)
		//
		// You can override compile() or the define_x methods to get more fine-grained control

		/// \brief About to begin writing out output
		virtual void begin_output();

		/// \brief Finishing writing out output
		virtual void end_output();

		/// \brief The output stage is about to produce a list of terminal symbols
		virtual void begin_terminal_symbols(const contextfree::grammar& gram);

		/// \brief Specifies the identifier for the terminal symbol with a given name
		virtual void terminal_symbol(const std::wstring& name, int identifier);

		/// \brief Finished writing out the terminal symbols
		virtual void end_terminal_symbols();

		/// \brief The output stage is about to produce a list of non-terminal symbols
		virtual void begin_nonterminal_symbols(const contextfree::grammar& gram);

		/// \brief Specifies the identifier for the non-terminal symbol with a given name
		virtual void nonterminal_symbol(const std::wstring& name, int identifier, const contextfree::item_container& item);

		// TODO: add 'other' symbols (EBNF items, guards, etc)

		/// \brief Finished writing out the terminal symbols
		virtual void end_nonterminal_symbols();

		/// \brief Starting to write out the lexer definitions
		virtual void begin_lexer_definitions();

		/// \brief Starting to write out the symbol map for the lexer
		virtual void begin_lexer_symbol_map(int maxSetId);

		/// \brief Specifies that a given range of symbols maps to a particular identifier
		virtual void symbol_map(const dfa::range<int>& symbolRange, int identifier);
		
		/// \brief Finishing writing out the symbol map for the lexer
		virtual void end_lexer_symbol_map();

		/// \brief About to begin writing out the lexer tables
		virtual void begin_lexer_state_machine(int numStates);

		/// \brief Starting to write out the transitions for a given state
		virtual void begin_lexer_state(int stateId);

		/// \brief Adds a transition for the current state
		///
		/// These are supplied in the order specified by the transition class: that is in
		/// acsending order of symbol set.
		virtual void lexer_state_transition(int symbolSet, int newState);

		/// \brief Finishes writing out a lexer state
		virtual void end_lexer_state();

		/// \brief Finished writing out the lexer table
		virtual void end_lexer_state_machine();

		/// \brief About to write out the list of accepting states for a lexer
		virtual void begin_lexer_accept_table();

		/// \brief The specified state is not an accepting state
		virtual void nonaccepting_state(int stateId);

		/// \brief The specified state is an accepting state
		virtual void accepting_state(int stateId, int acceptSymbolId);

		/// \brief Finished the lexer acceptance table
		virtual void end_lexer_accept_table();

		/// \brief Finished all of the lexer definitions
		virtual void end_lexer_definitions();

		/// \brief Starting to write out the definitions associated with the parser
		virtual void begin_parser_definitions();

		// TODO: maybe provide some fine-grained table generation?

		/// \brief Supplies the parser tables generated by the compiler
		virtual void parser_tables(const lr::lalr_builder& builder, const lr::parser_tables& tables);

		/// \brief Finished the parser definitions
		virtual void end_parser_definitions();

		/// \brief Starting to write out the definitions associated with the AST
		virtual void begin_ast_definitions(const contextfree::grammar& grammar, const contextfree::terminal_dictionary& terminals);

		/// \brief Starting to write the AST definitions for a particular terminal symbol
		virtual void begin_ast_terminal(int symbolId, const contextfree::item_container& item);

		/// \brief Finished writing the definitions for a terminal
		virtual void end_ast_terminal();

		/// \brief Starting to write the AST definitions for the specified nonterminal
		virtual void begin_ast_nonterminal(int identifier, const contextfree::item_container& item);

		/// \brief Starting to write out a rule in the current nonterminal
		virtual void begin_ast_rule(int identifier);

		/// \brief Writes out an individual item in the current rule (a nonterminal)
		virtual void rule_item_nonterminal(int nonterminalId, const contextfree::item_container& item);
        
        /// \brief Writes out an individual item in the current rule (a terminal)
        ///
        /// Note the distinction between the item ID, which is part of the grammar, and the
        /// symbol ID (which is part of the lexer and is the same as the value passed to 
        /// terminal_symbol)
        virtual void rule_item_terminal(int terminalItemId, int terminalSymbolId, const contextfree::item_container& item);

		/// \brief Finished writing out 
		virtual void end_ast_rule();

		/// \brief Finished writing the definitions for a nonterminal
		virtual void end_ast_nonterminal();

		/// \brief Finished writing out the AST information
		virtual void end_ast_definitions();
	};
}

#endif