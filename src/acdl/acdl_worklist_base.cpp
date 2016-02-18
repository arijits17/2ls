/*******************************************************************\

Module: ACDL Worklist Management Base

Author: Rajdeep Mukherjee, Peter Schrammel

 \*******************************************************************/

#include <util/find_symbols.h>
#include "acdl_worklist_base.h"

#define DEBUG


#ifdef DEBUG
#include <iostream>
#endif

/*******************************************************************\

Function: acdl_worklist_baset::push_into_assertion_list()

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/

void
acdl_worklist_baset::push_into_assertion_list (assert_listt &aexpr,
				  const acdl_domaint::statementt &statement)
{
  aexpr.push_back(statement);
}


/*******************************************************************	\

Function: acdl_worklist_baset::check_statement()

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/

bool
acdl_worklist_baset::check_statement (const exprt &expr,
                               const acdl_domaint::varst &vars)
{

  std::set<symbol_exprt> symbols;
  // find all variables in a statement
  find_symbols (expr, symbols);
  // check if vars appears in the symbols set,
  // if there is a non-empty intersection, then insert the
  // equality statement in the worklist
  for (acdl_domaint::varst::const_iterator it = vars.begin ();
      it != vars.end (); it++)
  {
    if (symbols.find (*it) != symbols.end ())
    {
      // find_symbols here may be required to 
      // find transitive dependencies
      // in which case make vars non-constant
      //find_symbols(expr, vars);
      return true;
    }
  }
  return false;
}

/*******************************************************************\

Function: acdl_worklist_baset::push_into_worklist()

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/

void
acdl_worklist_baset::push (const acdl_domaint::statementt &statement)
{
#if 1 // list implementation
  for(worklistt::const_iterator it = worklist.begin();
      it != worklist.end(); ++it)
    if(statement == *it)
      return;
  worklist.push_back(statement);
#else // set implementation
  worklist.insert(statement);
#endif
}

/*******************************************************************\

Function: acdl_worklist_baset::check_var_liveness()

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/

acdl_domaint::varst acdl_worklist_baset::check_var_liveness (acdl_domaint::varst &vars)
{
  
  for(acdl_domaint::varst::const_iterator it = 
    vars.begin(); it != vars.end(); ++it)
  {
    acdl_domaint::varst::iterator it1 = live_variables.find(*it);
    if(it1 == live_variables.end()) vars.erase(*it);
  }
  return vars;
}


/*******************************************************************\

Function: acdl_worklist_baset::remove_live_variables()

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/

void acdl_worklist_baset::remove_live_variables 
  (const local_SSAt &SSA, const acdl_domaint::statementt &statement)
{
#ifdef DEBUG
        std::cout << "Popped Statement for live variables: " 
        << from_expr (SSA.ns, "", statement) << std::endl;
#endif
  
  // remove variables in statement from live variables
  acdl_domaint::varst del_vars;
  find_symbols(statement, del_vars);  
#ifdef DEBUG
  std::cout << "Variables in Popped Statement: "; 
  for(acdl_domaint::varst::const_iterator it = 
    del_vars.begin(); it != del_vars.end(); ++it)
      std::cout << from_expr (SSA.ns, "", *it) << " ";
  std::cout << " " << std::endl;
#endif

#ifdef DEBUG   
  std::cout << "Live variables list are as follows: ";
  for(acdl_domaint::varst::const_iterator it = 
    live_variables.begin(); it != live_variables.end(); ++it)
   std::cout << from_expr(SSA.ns, "", *it); 
   std::cout << " " << std::endl;
#endif
  bool found = false;
  for(acdl_domaint::varst::const_iterator it = 
    del_vars.begin(); it != del_vars.end(); ++it) {
   for(std::list<acdl_domaint::statementt>::const_iterator 
      it1 = worklist.begin(); it1 != worklist.end(); ++it1)
   {
     acdl_domaint::varst find_vars;
     find_symbols(*it1, find_vars);
     acdl_domaint::varst::const_iterator it2 = find_vars.find(*it);   
     if(it2 != find_vars.end()) {found = true; break; }
   }  
   if(found == false)   
   {
#ifdef DEBUG
        std::cout << "Deleted live variables are: " 
        << from_expr (SSA.ns, "", *it) << std::endl;
#endif
       live_variables.erase(*it);
   }
  }
}


/*******************************************************************\

Function: acdl_worklist_baset::pop_from_worklist()

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/

const acdl_domaint::statementt
acdl_worklist_baset::pop ()
{
#if 1
  const acdl_domaint::statementt statement = worklist.front();
  worklist.pop_front();
#else
  worklistt::iterator it = worklist.begin ();
  const exprt statement = *it;
  worklist.erase (it);
#endif
  return statement;
}

/*******************************************************************\

Function: acdl_worklist_baset::update_worklist()

  Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/


void
acdl_worklist_baset::update (const local_SSAt &SSA,
                               const acdl_domaint::varst &vars,
                               const acdl_domaint::statementt &current_statement)
{
  live_variables.insert(vars.begin(),vars.end());
  
  // dependency analysis loop for equalities
  for (local_SSAt::nodest::const_iterator n_it = SSA.nodes.begin ();
      n_it != SSA.nodes.end (); n_it++)
  {
    for (local_SSAt::nodet::equalitiest::const_iterator e_it =
        n_it->equalities.begin (); e_it != n_it->equalities.end (); e_it++)
    {
      // the statement has already been processed, so no action needed
      if(*e_it == current_statement) continue;

      if (check_statement (*e_it, vars)) {
        push(*e_it);
        acdl_domaint::varst equal_vars;
        find_symbols(*e_it, equal_vars);
        live_variables.insert(equal_vars.begin(), equal_vars.end());    
        #ifdef DEBUG
        std::cout << "Push: " << from_expr (SSA.ns, "", *e_it) << std::endl;
        #endif
      }
    }
    for (local_SSAt::nodet::constraintst::const_iterator c_it =
        n_it->constraints.begin (); c_it != n_it->constraints.end (); c_it++)
    {
      if(*c_it == current_statement) continue;
      if (check_statement (*c_it, vars)) {
        push(*c_it);
        acdl_domaint::varst constraint_vars;
        find_symbols(*c_it, constraint_vars);
        live_variables.insert(constraint_vars.begin(), constraint_vars.end());    
        
        #ifdef DEBUG
        std::cout << "Push: " << from_expr (SSA.ns, "", *c_it) << std::endl;
        #endif
      }
    }
    for (local_SSAt::nodet::assertionst::const_iterator a_it =
        n_it->assertions.begin (); a_it != n_it->assertions.end (); a_it++)
    {
      // for now, store the decision variable as variables 
      // that appear only in properties
      // find all variables in an assert statement
      //assert_listt alist;
      push_into_assertion_list(alist, *a_it);
           
      if(*a_it == current_statement) continue;
      if (check_statement (*a_it, vars)) {
        push(not_exprt (*a_it));
        acdl_domaint::varst assert_vars;
        find_symbols(*a_it, assert_vars);
        live_variables.insert(assert_vars.begin(), assert_vars.end());    
        
        #ifdef DEBUG
        std::cout << "Push: " << from_expr (SSA.ns, "", not_exprt(*a_it)) << std::endl;
        #endif
      }
    }
  }
 
 // remove variables of popped statement from live variables
 remove_live_variables(SSA, current_statement); 

#ifdef DEBUG   
   std::cout << "The content of the updated worklist is as follows: " << std::endl;
    for(std::list<acdl_domaint::statementt>::const_iterator 
      it = worklist.begin(); it != worklist.end(); ++it)
	  std::cout << "Updated Worklist Element::" << from_expr(SSA.ns, "", *it) << std::endl;
#endif    


#ifdef DEBUG   
  std::cout << "The updated live variables after removal are as follows: ";
  for(acdl_domaint::varst::const_iterator it = 
    live_variables.begin(); it != live_variables.end(); ++it)
   std::cout << from_expr(SSA.ns, "", *it); 
   std::cout << " " << std::endl;
#endif

}


/*******************************************************************

 Function: acdl_worklist_baset::select_vars()

 Inputs:

 Outputs:

 Purpose:

 \*******************************************************************/

void
acdl_worklist_baset::select_vars (const exprt &statement, acdl_domaint::varst &vars)
{
#if 0 //TODO: this was an attempt to implement a forward iteration strategy,
      //      but we would also need to consider execution order 
  // If it is an equality, then select the lhs for post-condition computation
  exprt lhs;
  if (statement.id () == ID_equal)
  {
    lhs = to_equal_expr (statement).lhs ();
    if (lhs.id () == ID_symbol)
    {
      vars.insert (to_symbol_expr (lhs));
    }
    else //TODO: more complex lhs
      assert(false);
  }
  else // for constraints
#endif
  {
    find_symbols(statement,vars);
  }
}
