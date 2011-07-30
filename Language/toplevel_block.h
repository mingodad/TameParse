//
//  toplevel_block.h
//  Parse
//
//  Created by Andrew Hunter on 17/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LANGUAGE_TOPLEVEL_BLOCK_H
#define _LANGUAGE_TOPLEVEL_BLOCK_H

#include "Util/container.h"

#include "Language/block.h"
#include "Language/language_block.h"
#include "Language/import_block.h"

namespace language {
    /// \brief Class representing a toplevel block
    class toplevel_block : public block {
    private:
        /// \brief NULL if this is not a language block, otherwise the language block represented by this item
        language_block* m_LanguageBlock;
        
        /// \brief NULL if this is not an import block, otherwise the import block represented by this item
        import_block* m_ImportBlock;
        
    public:
        /// \brief Creates a new top level block containing a language block
        toplevel_block(language_block* language);
        
        /// \brief Creates a new top level block containing an import block
        toplevel_block(import_block* import);
        
        /// \brief Copies an existing toplevel block
        toplevel_block(const toplevel_block& copyFrom);
        
        /// \brief Destructor
        virtual ~toplevel_block();
        
        /// \brief Assigns this block
        toplevel_block& operator=(const toplevel_block& copyFrom);
        
        /// \brief NULL, or the language block represented by this toplevel block
        inline const language_block* language() const { return m_LanguageBlock; }
        
        /// \brief NULL, or the import block represented by this toplevel block
        inline const import_block* import() const { return m_ImportBlock; }
    };
}

#endif