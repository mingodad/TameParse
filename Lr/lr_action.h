//
//  lr_action.h
//  Parse
//
//  Created by Andrew Hunter on 04/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LR_LR_ACTION_H
#define _LR_LR_ACTION_H

#include <set>

#include "ContextFree/item.h"
#include "ContextFree/rule.h"
#include "Util/container.h"

namespace lr {
    ///
    /// \brief Description of an action in a LR parser
    ///
    class lr_action {
    public:
        /// \brief Types of LR action
        enum action_type {
            /// \brief If the terminal is seen, then it is placed on the stack and the next terminal is read
            act_shift,
            
            /// \brief If the terminal is seen, then the parser reduces by the specified rule
            /// 
            /// That is, pops the items in the rule, then pushes the nonterminal that the rule reduces to, and finally looks up the goto action
            /// for the resulting nonterminal in the state on top of the stack.
            act_reduce,
            
            /// \brief Works as for reduce, except that the parser does not perform this action if the symbol won't be shifted after the reduce
            ///
            /// If there is a weak reduce and a reduce action for a given symbol, then the weak reduce action is tried first. The parser should
            /// look at the state that will be reached by popping the stack and see where the goto leads to. If it would produce another reduction,
            /// it should continue looking there. If it would produce a shift action, then it should perform this reduction. If it would produce an
            /// error, then it should try other actions.
            ///
            /// This can be used to resolve reduce/reduce conflicts and hence allow a LALR parser to parse full LR(1) grammars. It is also useful
            /// if you want to support the concept of 'weak' lexical symbols (whose meaning depends on context), as a weak reduction is only possible
            /// if the lookahead symbol is a valid part of the language.
            act_weakreduce,
            
            /// \brief Identical to 'reduce', except the target symbol is the root of the language
            act_accept,
            
            /// \brief If a phrase has been reduced to this nonterminal symbol, then goto to the specified state
            act_goto,
        };
        
    private:
        /// \brief The type of action that this represents
        action_type m_Type;
        
        /// \brief The item that this refers to. For shift or reduce actions this will be a terminal. For goto actions, this will be a nonterminal
        contextfree::item_container m_Item;
        
        /// \brief The state to enter if this item is seen
        int m_NextState;
        
        /// \brief The rule that this refers to.
        contextfree::rule_container m_Rule;
        
    public:
        /// \brief Creates a shift or goto action (with no rule)
        lr_action(action_type type, const contextfree::item_container& item, int nextState);
        
        /// \brief Creates a reduce action (with a rule to reduce)
        lr_action(action_type type, const contextfree::item_container& item, int nextState, const contextfree::rule_container& rule);
        
        /// \brief Copy constructor
        lr_action(const lr_action& copyFrom);
        
        /// \brief Copy constructor, modifies the item for an action
        lr_action(const lr_action& copyFrom, const contextfree::item_container& newItem);
        
        /// \brief Orders this action
        inline bool operator<(const lr_action& compareTo) const {
            if (m_Item < compareTo.m_Item)              return true;
            if (m_Type < compareTo.m_Type)              return true;
            if (m_NextState < compareTo.m_NextState)    return true;
            if (m_Rule < compareTo.m_Rule)              return true;
            
            return false;
        }
        
        /// \brief Determines whether or not this action is the same as another
        inline bool operator==(const lr_action& compareTo) const {
            return m_Type == compareTo.m_Type && m_NextState == compareTo.m_NextState && m_Item == compareTo.m_Item && m_Rule == compareTo.m_Rule;
        }
        
        /// \brief The type of action that this represents
        inline action_type type() const { return m_Type; }
        
        /// \brief The item that this refers to. For shift or reduce actions this will be a terminal. For goto actions, this will be a nonterminal
        inline const contextfree::item_container& item() const { return m_Item; }
        
        /// \brief The state to enter if this item is seen
        inline int next_state() const { return m_NextState; }
        
        /// \brief The rule that this refers to.
        inline const contextfree::rule_container& rule() const { return m_Rule; }
        
        /// \brief Clones an existing action
        inline lr_action* clone() const {
            return new lr_action(*this);
        }
        
        /// \brief Compares two actions
        inline static bool compare(const lr_action* a, const lr_action* b) {
            if (a == b) return false;
            if (!a) return true;
            if (!b) return false;
            
            return *a < *b;
        }
    };
    
    /// \brief LR action container
    typedef util::container<lr_action> lr_action_container;
    
    /// \brief Set of LR actions
    typedef std::set<lr_action_container> lr_action_set;
}

#endif