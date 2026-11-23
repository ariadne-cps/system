/***************************************************************************
 *            composite_continuous_time_hybrid_automaton.hpp
 *
 *  Copyright  2004-20  Alberto Casagrande, Pieter Collins
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne System.  If not, see <https://www.gnu.org/licenses/>.
 */

/** @file system/composite_continuous_time_hybrid_automaton.hpp
 *  @brief Composition of hybrid automata.
 */

#ifndef ARIADNE_COMPOSITE_CONTINUOUS_TIME_HYBRID_AUTOMATON_HPP
#define ARIADNE_COMPOSITE_CONTINUOUS_TIME_HYBRID_AUTOMATON_HPP

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "function/function.hpp"
#include "hybrid/discrete_location.hpp"
#include "logging/logging.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/expression.hpp"
#include "system/discrete_event.hpp"
#include "system/continuous_time_hybrid_automaton.hpp"
#include "system/continuous_time_hybrid_automaton_interface.hpp"
#include "utility/lru_cache.hpp"

namespace Ariadne {

/** @brief HybridTime for composition of hybrid automata. */
class HybridTime;
/** @brief HybridSpace for composition of hybrid automata. */
class HybridSpace;

/** @brief DiscreteLocation for composition of hybrid automata. */
class DiscreteLocation;
/** @brief DiscreteMode for composition of hybrid automata. */
class DiscreteMode;
/** @brief ContinuousTimeHybridAutomaton for composition of hybrid automata. */
class ContinuousTimeHybridAutomaton;
/** @brief CompositeContinuousTimeHybridAutomaton for composition of hybrid automata. */
class CompositeContinuousTimeHybridAutomaton;

//! \ingroup SystemModule
//! \ingroup HybridAutomataSubModule
//! \brief A hybrid automaton, formed by running a finite number of ContinuousTimeHybridAutomaton classes in parallel.
//!
//! \sa \ref ContinuousTimeHybridAutomaton
class CompositeContinuousTimeHybridAutomaton
    : public ContinuousTimeHybridAutomatonInterface {
    static Writer<CompositeContinuousTimeHybridAutomaton> _default_writer;

  public:
    /** @brief Perform the system operation. */
    static Void set_default_writer(Writer<CompositeContinuousTimeHybridAutomaton> w) { _default_writer = w; }
    /** @brief Perform the system operation. */
    static Writer<CompositeContinuousTimeHybridAutomaton> default_writer() { return _default_writer; }

  public:
    typedef HybridTime TimeType;
    typedef List<ContinuousTimeHybridAutomaton>::ConstIterator ConstIterator;

  public:
    //!@{
    //! \name Constructors.

    //! \brief Default constructor creates "system" named automaton.
    CompositeContinuousTimeHybridAutomaton();
    //! \brief Constructor with name.
    CompositeContinuousTimeHybridAutomaton(Identifier name);
    //! \brief Convert a single atomic hybrid automaton to a composite hybrid automaton for analysis.
    CompositeContinuousTimeHybridAutomaton(const ContinuousTimeHybridAutomaton&);
    //! \brief Create the parallel composition of a list of atomic hybrid automata, with given name.
    CompositeContinuousTimeHybridAutomaton(
        Identifier name,
        const List<ContinuousTimeHybridAutomaton>&);
    //! \brief Create the parallel composition of a list of atomic hybrid automata, with composed name.
    CompositeContinuousTimeHybridAutomaton(const List<ContinuousTimeHybridAutomaton>&);

    //! \brief Construct dynamically-allocated copy. (Not currently implemented)
    CompositeContinuousTimeHybridAutomaton* clone() const { return new CompositeContinuousTimeHybridAutomaton(*this); }

    //! \brief Virtual destructor.
    virtual ~CompositeContinuousTimeHybridAutomaton() = default;

    //!@}

    //!@{
    //! \name Methods for querying the component automata.

    //! \brief The name of the automaton
    const Identifier& name() const { return _name; }

    //! \brief The number of component automata.
    Nat number_of_components() const;
    //! \brief The \a i<sup>th</sup> component automaton.
    const ContinuousTimeHybridAutomaton& component(Nat i) const;

    //! \brief Iterator begin
    ConstIterator begin() const { return _components.begin(); }
    //! \brief Iterator end
    ConstIterator end() const { return _components.end(); }

    //!@}

    //!@{
    //! \name Methods for finding the modes, transitions and events of the composite automaton.

    //! The mode corresponding to the given location.
    const DiscreteMode& mode(DiscreteLocation) const;
    //! \brief Tests if the automaton has a mode corresponding to the given location.
    Bool has_mode(DiscreteLocation) const;
    //! \brief Test if the hybrid automaton has a discrete mode corresponding to a subset of variables of the given location.
    Bool has_partial_mode(DiscreteLocation location) const;
    //! \brief Tests if the automaton has an invariant corresponding to the given location and event.
    Bool has_invariant(DiscreteLocation, DiscreteEvent) const;
    //! \brief Tests if the automaton has an invariant or transition corresponding to the given location and event.
    Bool has_guard(DiscreteLocation, DiscreteEvent) const;
    //! \brief Tests if the automaton has a transition corresponding to the given location and event.
    Bool has_transition(DiscreteLocation, DiscreteEvent) const;

    //!@}

    //!@{
    //! \name Methods fo rextracting the continuous dynamics of the automaton.

    //! \brief The target location when \a event occurs in the \a source location.
    DiscreteLocation target_location(DiscreteLocation source, DiscreteEvent event) const;

    //! \brief The continuous variables which are defined in the location.
    List<RealVariable> variables(DiscreteLocation) const;
    //! \brief The continuous variables which are state variables in the location.
    //! The state variables are those defined by a differential equation i.e. the dotted variables.
    List<RealVariable> state_variables(DiscreteLocation) const;
    //! \brief The dependent continuous variables which are not state variables in the location.
    //! These variables are defined by algebraic equations.
    List<RealVariable> auxiliary_variables(DiscreteLocation) const;
    //! \brief The continuous variables which are not state or auxiliary variables in the location,
    //! but are required in one of the dynamic equations, constraints or resets.
    Set<RealVariable> input_variables(DiscreteLocation) const;

    //! \brief The algebraic equations valid in the location.
    List<RealAssignment> auxiliary_assignments(DiscreteLocation location) const;
    //! \brief The algebraic equations valid in the location, ordered so that the defining equation for a variable
    //! occurs before any equation using that variable.
    List<RealAssignment> sorted_auxiliary_assignments(DiscreteLocation location) const;
    //! \brief The differential equations valid in the location.
    List<DottedRealAssignment> dynamic_assignments(DiscreteLocation location) const;
    //! \brief The reset equations used when the \a event occurs in the \a source location.
    List<PrimedRealAssignment> reset_assignments(DiscreteLocation source, DiscreteEvent event) const;
    //! \brief The invariant (time-can-progress predicates) corresponding to the given \a event.
    ContinuousPredicate invariant_predicate(DiscreteLocation location, DiscreteEvent event) const;
    //! \brief The guard (activation predicate) corresponding to the given \a event.
    ContinuousPredicate guard_predicate(DiscreteLocation location, DiscreteEvent event) const;
    //!@}

    //!@{
    //! \name Functions for conformance to ContinuousTimeHybridAutomatonInterface

    //! \brief The continuous state space for each location.
    virtual HybridSpace state_space() const;
    //! \brief The continuous space for each location.
    virtual HybridSpace state_auxiliary_space() const;
    //! \brief The continuous state space in the given location.
    RealSpace continuous_state_space(DiscreteLocation) const;
    //! \brief The space of continuous auxiliary variables in the given location.
    RealSpace continuous_auxiliary_space(DiscreteLocation) const;
    //! \brief The space of continuous state and auxiliary variables in the given location.
    RealSpace continuous_state_auxiliary_space(DiscreteLocation) const;
    //! \brief The dimension of the continuous state space in the given location.
    DimensionType dimension(DiscreteLocation) const;

    //! \brief The events which are active in the given location.
    Set<DiscreteEvent> events(DiscreteLocation) const;
    //! \brief The target for \a event from location \a source. Returns \a source if \a event is not present.
    DiscreteLocation target(DiscreteLocation location, DiscreteEvent event) const;

    //! \brief The function outputting the auxiliary variables \f$y=h(x)\f$ in the location.
    EffectiveVectorMultivariateFunction auxiliary_function(DiscreteLocation location) const;
    //! \brief The function outputting the differential equations \f$dx/dt =f(x)\f$ in the location.
    EffectiveVectorMultivariateFunction dynamic_function(DiscreteLocation location) const;
    //! \brief The reset function \f$x'=r(x)\f$ for the given event.
    EffectiveVectorMultivariateFunction reset_function(DiscreteLocation location, DiscreteEvent event) const;
    //! \brief The invariant function \f$i(x)\leq 0\f$ corresponding to the given event.
    EffectiveScalarMultivariateFunction invariant_function(DiscreteLocation location, DiscreteEvent event) const;
    //! \brief The guard function \f$g(x)\geq 0\f$ corresponding to the given event.
    EffectiveScalarMultivariateFunction guard_function(DiscreteLocation location, DiscreteEvent event) const;
    //! \brief The type of the event (urgent, permissive, impact etc).
    EventKind event_kind(DiscreteLocation location, DiscreteEvent event) const;

    //!@}

    //!@{
    //! \name Methods for checking the validity of the automaton.

    //! \brief Checks whether the equations valid in the location are valid.
    //!
    //! Includes a check for algebraic dependencies, over-defined variables, under-defined variables, and
    //! variables which should be defined in a reset but are not.
    Void check_mode(DiscreteLocation) const;
    //! \brief Runs check_mode() in any mode reachable under the discrete dynamics from the given initial locations.
    Void check_reachable_modes(const Set<DiscreteLocation>&) const;
    //!@}

    //!@{
    //! \name Discrete reachability analysis.

    //! \brief Performs a discrete reachability analysis from the given initial locations.
    Set<DiscreteLocation> discrete_reachability(const Set<DiscreteLocation>&) const;
    //!@}

    //! \brief Write to an output stream.
    OutputStream& _write(OutputStream&) const;

  protected:
    Identifier _name;

  private:
    List<ContinuousTimeHybridAutomaton> _components;
    mutable LRUCache<DiscreteLocation, DiscreteMode> _modes_cache;
    /** @brief Perform the system operation. */
    Void _cache_mode(DiscreteLocation) const;
};

/** @brief Perform the system operation. */
CompositeContinuousTimeHybridAutomaton parallel_composition(const List<ContinuousTimeHybridAutomaton>& components);

inline OutputStream& operator<<(OutputStream& os, const CompositeContinuousTimeHybridAutomaton& ha) {
    return ha._write(os);
}

/** @brief CompactCompositeHybridAutomatonWriter for composition of hybrid automata. */
class CompactCompositeHybridAutomatonWriter : public WriterInterface<CompositeContinuousTimeHybridAutomaton> {
    /** @brief Perform the system operation. */
    virtual OutputStream& _write(OutputStream& os, const CompositeContinuousTimeHybridAutomaton& ha) const final override;
};

/** @brief VerboseCompositeHybridAutomatonWriter for composition of hybrid automata. */
class VerboseCompositeHybridAutomatonWriter : public WriterInterface<CompositeContinuousTimeHybridAutomaton> {
    /** @brief Perform the system operation. */
    virtual OutputStream& _write(OutputStream& os, const CompositeContinuousTimeHybridAutomaton& ha) const final override;
};

} // namespace Ariadne

#endif // ARIADNE_CONTINUOUS_TIME_HYBRID_AUTOMATON_HPP
