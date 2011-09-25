//
//  language_builder_stage.h
//  TameParse
//
//  Created by Andrew Hunter on 25/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _COMPILER_LANGUAGE_BUILDER_STAGE_H
#define _COMPILER_LANGUAGE_BUILDER_STAGE_H

#include <map>
#include <string>

#include "TameParse/Compiler/compilation_stage.h"
#include "TameParse/Compiler/language_stage.h"
#include "TameParse/Compiler/import_stage.h"

namespace compiler {
	///
	/// \brief Stage that creates grammars and NDFAs from all of the languages that were
	/// imported by an import_stage
	///
	class language_builder_stage : public compilation_stage {
	public:
		/// \brief Maps language names to 
		typedef std::map<std::wstring, language_stage*> language_map;

	private:
		/// \brief The import stage
		///
		/// This contains all of the languages loaded by this parser generator. This
		/// stage is not freed by this object
		import_stage* m_ImportStage;

		/// \brief Maps languages to the grammars and NDFAs that are built for them
		///
		/// These stages are destroyed when this object is destroyed
		language_map m_Languages;

	public:
		/// \brief Creates a new language builder stage
		language_builder_stage(console_container& console, const std::wstring& filename, import_stage* importStage);

		/// \brief Destructor
		virtual ~language_builder_stage();

        /// \brief Performs the actions associated with this compilation stage
		virtual void compile();
	};
}


#endif
