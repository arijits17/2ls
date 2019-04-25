/*******************************************************************\

Module: Predicate abstraction domain

Author: Peter Schrammel

\*******************************************************************/

/// \file
/// Predicate abstraction domain

#ifndef CPROVER_2LS_DOMAINS_PREDABS_DOMAIN_H
#define CPROVER_2LS_DOMAINS_PREDABS_DOMAIN_H

#include <set>

#include <util/std_expr.h>
#include <util/arith_tools.h>
#include <util/ieee_float.h>

#include "domain.h"

class predabs_domaint:public domaint
{
public:
  typedef unsigned rowt;
  typedef exprt row_exprt; // predicate
  typedef constant_exprt row_valuet; // true/false
  typedef std::set<rowt> row_sett;

  class templ_valuet:public domaint::valuet, public std::vector<row_valuet>
  {
  };

  typedef struct
  {
    guardt pre_guard;
    guardt post_guard;
    row_exprt expr;
    exprt aux_expr;
    kindt kind;
  } template_rowt;

  typedef std::vector<template_rowt> templatet;

  predabs_domaint(
    unsigned _domain_number,
    replace_mapt &_renaming_map,
    const namespacet _ns):
    domaint(_domain_number, _renaming_map, _ns)
  {
  }

  const exprt initialize_solver(
    const local_SSAt &SSA,
    const exprt &precondition,
    template_generator_baset &template_generator);

  // initialize value
  virtual void initialize(valuet &value);

  virtual void solver_iter_init(valuet &value);

  virtual bool has_something_to_solve();

  std::vector<exprt> get_required_smt_values(size_t row);
  void set_smt_values(std::vector<exprt> got_values, size_t row);

  bool edit_row(const rowt &row, valuet &inv, bool improved);

  void post_edit();

  exprt to_pre_constraints(valuet &_value);

  void make_not_post_constraints(
    valuet &_value,
    exprt::operandst &cond_exprs);

  bool handle_unsat(valuet &value, bool improved);

  exprt make_permanent(valuet &value);

  // value -> constraints
  exprt get_row_constraint(const rowt &row, const row_valuet &row_value);
  exprt get_row_pre_constraint(const rowt &row, const row_valuet &row_value);
  exprt get_row_post_constraint(const rowt &row, const row_valuet &row_value);
  exprt get_row_pre_constraint(const rowt &row, const templ_valuet &value);
  exprt get_row_post_constraint(const rowt &row, const templ_valuet &value);

  // set, get value
  row_valuet get_row_value(const rowt &row, const templ_valuet &value);
  void set_row_value(
    const rowt &row,
    const row_valuet &row_value,
    valuet &_value);

  // printing
  virtual void output_value(
    std::ostream &out,
    const valuet &value,
    const namespacet &ns) const;
  virtual void output_domain(std::ostream &out, const namespacet &ns) const;

  // projection
  virtual void project_on_vars(
    valuet &value,
    const var_sett &vars,
    exprt &result);

  unsigned template_size();

  // generating templates
  template_rowt &add_template_row(
    const exprt& expr,
    const exprt& pre_guard,
    const exprt& post_guard,
    const exprt& aux_expr,
    kindt kind);

  void get_row_set(row_sett &rows);

protected:
  templatet templ;
  exprt value;

public:
  typedef std::set<unsigned> worklistt;
  worklistt::iterator e_it;
  worklistt todo_preds;
  worklistt todo_notpreds;
};

#endif
