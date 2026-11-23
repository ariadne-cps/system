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
//! describes per-mode discrete algebraic equations, guarded transitions and progression
//! invariants. At a state, a map successor is possible when every progression
//! invariant holds. Every enabled guard independently admits a reset successor.
//! Hence for permissive events mapping and jumping can both be possible.
//! Jump time accounting, zero-time chains and scheduling are deliberately\n//! left to be specified before implementing discrete hybrid evolvers.
//! This interface does not implement set-valued guard evaluation or evolution.
class DiscreteTimeHybridAutomatonInterface : public HybridAutomatonInterface {
  public:
    using TimeType = Integer;
    using RealType = Real;

    virtual ~DiscreteTimeHybridAutomatonInterface() = default;
    virtual DiscreteTimeHybridAutomatonInterface* clone() const = 0;

    //! \brief Declarative discrete-time equations for the given mode.
    virtual const DifferenceAlgebraicEquation& equation(DiscreteLocation location) const = 0;
    //! \brief Operational map obtained from the declared discrete-time equations.
    virtual const IteratedMap& iterated_map(DiscreteLocation location) const { return equation(location).iterated_map(); }
    //! \brief Continuous state space of a discrete mode.
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
