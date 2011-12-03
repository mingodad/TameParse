//
//  lexer_item.h
//  TameParse
//
//  Created by Andrew Hunter on 12/11/2011.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _COMPILER_LEXER_ITEM_H
#define _COMPILER_LEXER_ITEM_H

#include <string>

#include "TameParse/Dfa/position.h"
#include "TameParse/Dfa/accept_action.h"
#include "TameParse/Language/language_unit.h"

namespace compiler {
	/// \brief Class representing the data associated with an individual lexer item
	class lexer_item {
	public:
		/// \brief The type of the language unit where this block was defined
		typedef language::language_unit::unit_type unit_type;

		/// \brief The type of a lexer item
		enum item_type {
			regex,
			literal
		};

		/// \brief The type of this item
		item_type type;

		/// \brief The definition of this item
		std::wstring definition;

		/// \brief True if this item should be case-insensitive
		bool case_insensitive;

		/// \brief True if this item should be case sensitive
		///
		/// Used for lexer symbols blocks, which will be explicitly case sensitive
		/// if specified this way (so you can have a case sensitive part of an
		/// other case-insensitive expression)
		bool case_sensitive;

		/// \brief The identifier of the symbol that should be generated by this item
		int symbol;

		/// \brief The language unit type where this symbol was defined
		unit_type definition_type;

		/// \brief True if this is a weak symbol
		bool is_weak;

		/// \brief The file where this symbol is defined
		const std::wstring* filename;

		/// \brief The position where this symbol is defined
		dfa::position position;

		/// \brief Creates a new lexer item
		lexer_item(item_type type, const std::wstring& definition, bool case_insensitive, bool case_sensitive, const std::wstring* filename, const dfa::position& pos);

		/// \brief Creates a new lexer item
		lexer_item(item_type type, const std::wstring& definition, bool case_insensitive, bool case_sensitive, int symbol, unit_type definition_type, bool is_weak, const std::wstring* filename, const dfa::position& pos);
        
        /// \brief Copies a lexer item
        lexer_item(const lexer_item& copyFrom);
        
        /// \brief Assigns a lexer item
        lexer_item& operator=(const lexer_item& assignFrom);

		/// \brief Disposes a lexer item
		virtual ~lexer_item();
	};
}

#endif
