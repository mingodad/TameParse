//
//  lalr_builder.h
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LR_LALR_BUILDER_H
#define _LR_LALR_BUILDER_H

#include <map>
#include <set>

#include "Util/container.h"
#include "ContextFree/grammar.h"
#include "Lr/lalr_machine.h"
#include "Lr/lr_action.h"

namespace lr {
    /// \brief Forward declaration of the action rewriter class
    class action_rewriter;
    
    /// \brief Class that can contain an action rewriter
    typedef util::container<action_rewriter> action_rewriter_container;

    /// \brief List of action rewriters to apply when producing the final set of actions for a parser
    typedef std::vector<action_rewriter_container> action_rewriter_list;
    
    ///
    /// \brief Class that builds up a LALR state machine from a grammar
    ///
    class lalr_builder {
    public:
        // State ID, item ID pair (identifies an individual item within a state machine)
        typedef std::pair<int, int> lr_item_id;
        
        // Maps states to lists of propagations (target state, target item ID)
        typedef std::map<lr_item_id, std::set<lr_item_id> > propagation;
        
        /// \brief Set of LR(0) items that represent a closure of a LALR state
        typedef std::set<lr0_item_container> closure_set;
        
    private:
        /// \brief The grammar that this builder will use
        contextfree::grammar* m_Grammar;
        
        /// \brief The LALR state machine that this is building up
        ///
        /// We store only the kernel states here.
        lalr_machine m_Machine;
        
        /// \brief List of action rewriter objects
        action_rewriter_list m_ActionRewriters;
        
        /// \brief Where lookaheads propagate for each item in the state machine
        mutable propagation m_Propagate;
        
        /// \brief Maps state IDs to sets of LR actions
        mutable std::map<int, lr_action_set> m_ActionsForState;
        
    public:
        /// \brief Creates a new builder for the specified grammar
        lalr_builder(contextfree::grammar& gram);
        
        /// \brief Adds an initial state to this builder that will recognise the language specified by the supplied symbol
        ///
        /// To build a valid parser, you need to add at least one symbol. The builder will add a new state that recognises
        /// this language
        int add_initial_state(const contextfree::item_container& language);
        
        /// \brief Finishes building the parser (the LALR machine will contain a LALR parser after this call completes)
        void complete_parser();
        
        /// \brief Generates the lookaheads for the parser (when the machine has been built up as a LR(0) grammar)
        void complete_lookaheads();
        
        /// \brief The LALR state machine being built up by this object
        lalr_machine& machine() { return m_Machine; }
        
        /// \brief The LALR state machine being built up by this object
        const lalr_machine& machine() const { return m_Machine; }
        
        /// \brief The grammar used for this builder
        const contextfree::grammar& gram() const { return *m_Grammar; }
        
        /// \brief Adds a new action rewriter to this builder
        void add_rewriter(const action_rewriter_container& rewriter);
        
        /// \brief Replaces the rewriters that this builder will use
        void set_rewriters(const action_rewriter_list& list);
        
        /// \brief Creates the closure for a particular lalr state
        static void create_closure(closure_set& target, const lalr_state& state, const contextfree::grammar* gram);
        
    public:
        /// \brief Returns the number of states in the state machine
        inline int count_states() const { return m_Machine.count_states(); }
        
        /// \brief After the state machine has been completely built, returns the actions for the specified state
        ///
        /// If there are conflicts, this will return multiple actions for a single symbol.
        const lr_action_set& actions_for_state(int state) const;
        
        /// \brief Returns the items that the lookaheads are propagated to for a particular item in this state machine
        const std::set<lr_item_id>& propagations_for_item(int state, int item) const;
    };
}

#include "action_rewriter.h"

#endif
