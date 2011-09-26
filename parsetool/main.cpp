//
//  main.cpp
//  parsetool
//
//  Created by Andrew Hunter on 24/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/TameParse.h"
#include "boost_console.h"

#include <iostream>
#include <memory>

using namespace std;
using namespace dfa;
using namespace tameparse;
using namespace language;
using namespace compiler;

int main (int argc, const char * argv[])
{
    // Create the console
    boost_console       console(argc, argv);
    console_container   cons(&console, false);
    
    try {
        // Give up if the console is set not to start
        if (!console.can_start()) {
            return 1;
        }
        
        // Startup message
        console.message_stream() << L"TameParse " << version::major_version << "." << version::minor_version << "." << version::revision << endl;
        console.verbose_stream() << endl;
        
        // Parse the input file
        parser_stage parserStage(cons, console.input_file());
        parserStage.compile();
        
        // Stop if we have an error
        if (console.exit_code()) {
            return console.exit_code();
        }
        
        // The definition file should exist
        if (!parserStage.definition_file().item()) {
            console.report_error(error(error::sev_bug, console.input_file(), L"BUG_NO_FILE_DATA", L"File did not produce any data", position(-1, -1, -1)));
            return error::sev_bug;
        }
        
        // Parse any imported files
        import_stage importStage(cons, console.input_file(), parserStage.definition_file());
        importStage.compile();

        // Stop if we have an error
        if (console.exit_code()) {
            return console.exit_code();
        }
        
        // Convert to grammars & NDFAs
        language_builder_stage builderStage(cons, console.input_file(), &importStage);
        builderStage.compile();
        
        // Stop if we have an error
        if (console.exit_code()) {
            return console.exit_code();
        }
        
        // Work out the name of the language to build and the start symbols
        wstring         buildLanguageName   = console.get_option(L"compile-language");
        wstring         buildClassName      = console.get_option(L"class-name");
        vector<wstring> startSymbols        = console.get_option_list(L"start-symbol");
        wstring         targetLanguage      = console.get_option(L"target-language");
        wstring         buildNamespaceName  = console.get_option(L"namespace-name");
        position        parseBlockPosition  = position(-1, -1, -1);
        
        // Use the options by default
        if (console.get_option(L"compile-language").empty()) {
            // TODO: get information from the parser block
            // TODO: the class-name option overrides the name specified in the parser block (but gives a warning)
            // TODO: use the position of the parser block to report 
            // TODO: deal with multiple parser blocks somehow (error? generate multiple classes?)
        }
        
        // If there is only one language in the original file and none specified, then we will generate that
        if (buildLanguageName.empty()) {
            int languageCount = 0;
            
            // Find all of the language blocks
            for (definition_file::iterator defnBlock = parserStage.definition_file()->begin(); defnBlock != parserStage.definition_file()->end(); defnBlock++) {
                if ((*defnBlock)->language()) {
                    languageCount++;
                    if (languageCount > 1) {
                        // Multiple languages: can't infer one
                        buildLanguageName.clear();
                        break;
                    } else {
                        // This is the first language: use its identifier as the language to build
                        buildLanguageName = (*defnBlock)->language()->identifier();
                    }
                }
            }
            
            // Tell the user about this
            if (!buildLanguageName.empty()) {
                wstringstream msg;
                msg << L"Language name not explicitly specified: will use '" << buildLanguageName << L"'";
                console.report_error(error(error::sev_info, console.input_file(), L"INFERRED_LANGUAGE", msg.str(), parseBlockPosition));
            }
        }
        
        // Error if there is no language specified
        if (buildLanguageName.empty()) {
            console.report_error(error(error::sev_error, console.input_file(), L"NO_LANGUAGE_SPECIFIED", L"Could not determine which language block to compile", parseBlockPosition));
            return error::sev_error;
        }
        
        // Infer the class name to use if none is specified (same as the language name)
        if (buildClassName.empty()) {
            buildClassName = buildLanguageName;
        }
        
        // Get the language that we're going to compile
        const language_block*   compileLanguage       = importStage.language_with_name(buildLanguageName);
        language_stage*         compileLanguageStage  = builderStage.language_with_name(buildLanguageName);
        if (!compileLanguage || !compileLanguageStage) {
            // The language could not be found
            wstringstream msg;
            msg << L"Could not find the target language '" << buildLanguageName << L"'";
            console.report_error(error(error::sev_error, console.input_file(), L"MISSING_TARGET_LANGUAGE", msg.str(), parseBlockPosition));
            return error::sev_error;
        }
        
        // Infer the start symbols if there are none
        if (startSymbols.empty()) {
            // TODO: Use the first nonterminal defined in the language block
            // TODO: warn if we establish a start symbol this way
        }
        
        if (startSymbols.empty()) {
            // Error if we can't find any start symbols at all
            console.report_error(error(error::sev_error, console.input_file(), L"NO_START_SYMBOLS", L"Could not determine a start symbol for the language (use the start-symbol option to specify one manually)", parseBlockPosition));
            return error::sev_error;
        }
        
        // Generate the lexer for the target language
        lexer_stage lexerStage(cons, importStage.file_with_language(buildLanguageName), compileLanguageStage);
        lexerStage.compile();

        // Stop if we have an error
        if (console.exit_code()) {
            return console.exit_code();
        }

        // Generate the parser
        lr_parser_stage lrParserStage(cons, importStage.file_with_language(buildLanguageName), compileLanguageStage, &lexerStage, startSymbols);
        lrParserStage.compile();
        
        // Stop if we have an error
        if (console.exit_code()) {
            return console.exit_code();
        }
        
        // The --test option sets the target language to 'test'
        if (!console.get_option(L"test").empty()) {
            targetLanguage = L"test";
        }
        
        // Target language is C++ if no language is specified
        if (targetLanguage.empty()) {
            targetLanguage = L"cplusplus";
        }
        
        // Work out the prefix filename
        wstring prefixFilename = console.get_option(L"output-language");
        if (prefixFilename.empty()) {
            // Derive from the input file
            // This works provided the target language 
            prefixFilename = console.input_file();
        }
        
        // Create the output stage         
        auto_ptr<output_stage> outputStage(NULL);
        
        if (targetLanguage == L"cplusplus") {
            // Use the C++ language generator
            outputStage = auto_ptr<output_stage>(new output_cplusplus(cons, importStage.file_with_language(buildLanguageName), &lexerStage, compileLanguageStage, &lrParserStage, prefixFilename, buildClassName, buildNamespaceName));
        } else {
            // Unknown target language
            wstringstream msg;
            msg << L"Output language '" << targetLanguage << L"' is not known";
            console.report_error(error(error::sev_error, console.input_file(), L"UNKNOWN_OUTPUT_LANGUAGE_TYPE", msg.str(), position(-1, -1, -1)));
            return error::sev_error;
        }
        
        // Compile the final output
        if (outputStage.get()) {
            outputStage->compile();
        }
        
        // Done
        return console.exit_code();
    } catch (...) {
        console.report_error(error(error::sev_bug, L"", L"BUG_UNCAUGHT_EXCEPTION", L"Uncaught exception", position(-1, -1, -1)));
        throw;
    }
}
