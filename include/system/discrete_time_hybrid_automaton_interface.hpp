/***************************************************************************
 *            discrete_time_hybrid_automaton_interface.hpp
 *
 *  Copyright  2026  Luca Geretti
 *
 ****************************************************************************/

/*
 *  This file is part of Ariadne System.
 *
 *  Ariadne System is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Ariadne System is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne System. If not, see <https://www.gnu.org/licenses/>.
 */

/** @file system/discrete_time_hybrid_automaton_interface.hpp
 *  @brief Native hybrid automata over integer time.
 */

#ifndef ARIADNE_DISCRETE_TIME_HYBRID_AUTOMATON_INTERFACE_HPP
#define ARIADNE_DISCRETE_TIME_HYBRID_AUTOMATON_INTERFACE_HPP

#include "system/hybrid_automaton_interface.hpp"
#include "system/difference_algebraic_equation.hpp"
#include "system/difference_algebraic_inclusion.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/expression.hpp"

namespace Ariadne {

//! \ingroup SystemModule
//! \brief A transition may either prevent progression when enabled, or permit it.
//! \details URGENT has an implicit progression invariant equal to the
//! complement of its guard. PERMISSIVE has a separately declared invariant.
enum class DiscreteTimeTransitionKind { URGENT, PERMISSIVE };

//! \ingroup SystemModule
//! \brief Structural and symbolic data for a hybrid system on integer time.
//! \details The common interface describes the graph; this specialization
//! describes per-mode difference equations or inclusions, guarded transitions and progression
//! invariants. At a state, a dynamics successor is possible when every progression
//! invariant holds. Every enabled guard independently admits a reset successor.
//! Hence for permissive events a dynamics step and jump can both be possible.
//! Both a dynamics step and a transition reset take one integer tick:
//! every successor of a state at index n belongs to index n+1.
//! Guards and progression invariants are evaluated at the source state n,
//! before either an update or a reset. Each enabled event yields a separate
//! successor; simultaneous events are never silently prioritized or
//! combined into a reset chain. There are no zero-time transitions.
//! Numeric evaluation and inclusion image computation remain evolver duties.
//! This interface does not implement set-valued guard evaluation or evolution.
class DiscreteTimeHybridAutomatonInterface : public HybridAutomatonInterface {
  public:
    using TimeType = Integer;
    using RealType = Real;

    virtual ~DiscreteTimeHybridAutomatonInterface() = default;
    virtual DiscreteTimeHybridAutomatonInterface* clone() const = 0;

    //! \brief True when the mode contains a deterministic difference equation.
    virtual Bool has_equation(DiscreteLocation location) const = 0;
    //! \brief True when the mode contains a bounded-input difference inclusion.
    virtual Bool has_inclusion(DiscreteLocation location) const = 0;
    //! \brief Deterministic declaration; throws for an inclusion mode.
    virtual const DifferenceAlgebraicEquation& equation(DiscreteLocation location) const = 0;
    //! \brief Set-valued declaration; throws for a deterministic mode.
    virtual const DifferenceAlgebraicInclusion& inclusion(DiscreteLocation location) const = 0;
    //! \brief Source dynamics function; inclusion arguments include free inputs.
    virtual const EffectiveVectorMultivariateFunction& update_function(DiscreteLocation location) const = 0;
    //! \brief Auxiliary output function for either dynamics declaration.
    virtual const EffectiveVectorMultivariateFunction& auxiliary_function(DiscreteLocation location) const = 0;
    //! \brief Continuous state space of either dynamics family.
    virtual RealSpace state_space(DiscreteLocation location) const = 0;
    //! \brief Whether a transition restricts time progression when enabled.
    virtual DiscreteTimeTransitionKind transition_kind(DiscreteLocation location, DiscreteEvent event) const = 0;
    //! \brief Predicate enabling the reset transition at the current state.
    virtual ContinuousPredicate guard_predicate(DiscreteLocation location, DiscreteEvent event) const = 0;
    //! \brief Predicate allowing progression for this event at the current state.
    //! \details Urgent: complement of guard. Permissive: explicitly supplied.
    virtual ContinuousPredicate invariant_predicate(DiscreteLocation location, DiscreteEvent event) const = 0;
    //! \brief Total reset assignments from source state to target state.
    virtual List<PrimedRealAssignment> reset_assignments(DiscreteLocation location, DiscreteEvent event) const = 0;
};

} // namespace Ariadne

#endif // ARIADNE_DISCRETE_TIME_HYBRID_AUTOMATON_INTERFACE_HPP
