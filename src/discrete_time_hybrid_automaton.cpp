/***************************************************************************
 *            discrete_time_hybrid_automaton.cpp
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

/** @file src/discrete_time_hybrid_automaton.cpp
 *  @brief Native integer-time hybrid automaton declarations and validation.
 */

#include "system/discrete_time_hybrid_automaton.hpp"

#include <stdexcept>

#include "utility/container.hpp"

namespace Ariadne {

DiscreteTimeHybridAutomaton::DiscreteTimeHybridAutomaton()
    : _name("automaton") {}

DiscreteTimeHybridAutomaton::DiscreteTimeHybridAutomaton(Identifier name)
    : _name(name) {}

DiscreteTimeHybridAutomaton* DiscreteTimeHybridAutomaton::clone() const {
    return new DiscreteTimeHybridAutomaton(*this);
}

Void DiscreteTimeHybridAutomaton::new_mode(DiscreteLocation location, const DifferenceAlgebraicEquation& equation) {
    if (this->_first_indistinguishable_mode(location))
        throw std::invalid_argument("Indistinguishable discrete-time hybrid mode location");
    this->_modes.insert(location, DiscreteTimeHybridMode(location, equation));
}

Void DiscreteTimeHybridAutomaton::new_mode(DiscreteLocation location, const DifferenceAlgebraicInclusion& inclusion) {
    if (this->_first_indistinguishable_mode(location))
        throw std::invalid_argument("Indistinguishable discrete-time hybrid mode location");
    this->_modes.insert(location, DiscreteTimeHybridMode(location, inclusion));
}

const DiscreteTimeHybridMode& DiscreteTimeHybridAutomaton::mode(DiscreteLocation location) const {
    const DiscreteTimeHybridMode* result = this->_find_mode(location);
    if (result == nullptr)
        throw std::out_of_range("Discrete-time hybrid mode does not exist");
    return *result;
}

Void DiscreteTimeHybridAutomaton::_new_transition(
    DiscreteLocation source, DiscreteEvent event, DiscreteLocation target,
    const List<PrimedRealAssignment>& reset, ContinuousPredicate guard,
    ContinuousPredicate invariant, DiscreteTimeTransitionKind kind) {

    const TransitionStatus status = this->_transition_status(source, event, target);
    if (status == TransitionStatus::MISSING_SOURCE)
        throw std::out_of_range("Discrete-time hybrid transition source does not exist");
    if (status == TransitionStatus::MISSING_TARGET)
        throw std::out_of_range("Discrete-time hybrid transition target does not exist");
    if (status == TransitionStatus::DUPLICATE_EVENT)
        throw std::invalid_argument("Duplicate discrete-time hybrid transition event");

    const DiscreteTimeHybridMode& source_mode_declaration = this->mode(source);
    const DiscreteTimeHybridMode& target_mode_declaration = this->mode(target);
    // Input symbols are not event variables in this initial contract; guards
    // and resets may depend on state and declared auxiliary outputs only.
    const Set<UntypedVariable> available_variables(make_set(source_mode_declaration.state_auxiliary_space().variables()));

    if (!subset(guard.arguments(), available_variables))
        throw std::invalid_argument("Discrete-time hybrid guard depends on undeclared source variables");
    if (!subset(invariant.arguments(), available_variables))
        throw std::invalid_argument("Discrete-time hybrid progression invariant depends on undeclared source variables");

    const List<RealVariable> assigned = left_hand_sides(reset);
    const Set<RealVariable> target_variables(make_set(target_mode_declaration.state_space().variables()));
    if (!unique_elements(assigned) || Set<RealVariable>(assigned) != target_variables)
        throw std::invalid_argument("Discrete-time hybrid reset must assign every target state variable exactly once");
    for (const PrimedRealAssignment& assignment: reset)
        if (!subset(assignment.rhs.arguments(), available_variables))
            throw std::invalid_argument("Discrete-time hybrid reset depends on undeclared source variables");

    DiscreteTimeHybridMode& source_mode = this->_modes.value(source);
    source_mode.add_transition(event, target);
    source_mode._resets.insert(event, reset);
    source_mode._guards.insert(event, guard);
    source_mode._invariants.insert(event, invariant);
    source_mode._kinds.insert(event, kind);
}

Void DiscreteTimeHybridAutomaton::new_urgent_transition(
    DiscreteLocation source, DiscreteEvent event, DiscreteLocation target,
    const List<PrimedRealAssignment>& reset, ContinuousPredicate guard) {
    this->_new_transition(source, event, target, reset, guard, !guard, DiscreteTimeTransitionKind::URGENT);
}

Void DiscreteTimeHybridAutomaton::new_permissive_transition(
    DiscreteLocation source, DiscreteEvent event, DiscreteLocation target,
    const List<PrimedRealAssignment>& reset, ContinuousPredicate guard,
    ContinuousPredicate invariant) {
    this->_new_transition(source, event, target, reset, guard, invariant, DiscreteTimeTransitionKind::PERMISSIVE);
}

Set<DiscreteEvent> DiscreteTimeHybridAutomaton::events(DiscreteLocation location) const {
    return this->mode(location).transition_events();
}

Bool DiscreteTimeHybridAutomaton::has_transition(DiscreteLocation source, DiscreteEvent event) const {
    return this->has_partial_mode(source) && this->mode(source).has_transition(event);
}

DiscreteLocation DiscreteTimeHybridAutomaton::target(DiscreteLocation source, DiscreteEvent event) const {
    return this->has_transition(source, event)
        ? this->mode(source).transition_target(event, source)
        : source;
}

DimensionType DiscreteTimeHybridAutomaton::dimension(DiscreteLocation location) const {
    return this->mode(location).dimension();
}

Bool DiscreteTimeHybridAutomaton::has_equation(DiscreteLocation location) const {
    return this->mode(location).has_equation();
}

Bool DiscreteTimeHybridAutomaton::has_inclusion(DiscreteLocation location) const {
    return this->mode(location).has_inclusion();
}

const DifferenceAlgebraicEquation& DiscreteTimeHybridAutomaton::equation(DiscreteLocation location) const {
    return this->mode(location).equation();
}

const DifferenceAlgebraicInclusion& DiscreteTimeHybridAutomaton::inclusion(DiscreteLocation location) const {
    return this->mode(location).inclusion();
}

const IteratedMap& DiscreteTimeHybridAutomaton::iterated_map(DiscreteLocation location) const {
    return this->mode(location).iterated_map();
}

const EffectiveVectorMultivariateFunction& DiscreteTimeHybridAutomaton::update_function(DiscreteLocation location) const {
    return this->mode(location).update_function();
}

const EffectiveVectorMultivariateFunction& DiscreteTimeHybridAutomaton::auxiliary_function(DiscreteLocation location) const {
    return this->mode(location).auxiliary_function();
}

RealSpace DiscreteTimeHybridAutomaton::state_space(DiscreteLocation location) const {
    return this->mode(location).state_space();
}

DiscreteTimeTransitionKind DiscreteTimeHybridAutomaton::transition_kind(
    DiscreteLocation location, DiscreteEvent event) const {
    return this->mode(location)._kinds[event];
}

ContinuousPredicate DiscreteTimeHybridAutomaton::guard_predicate(
    DiscreteLocation location, DiscreteEvent event) const {
    return this->mode(location)._guards[event];
}

ContinuousPredicate DiscreteTimeHybridAutomaton::invariant_predicate(
    DiscreteLocation location, DiscreteEvent event) const {
    return this->mode(location)._invariants[event];
}

List<PrimedRealAssignment> DiscreteTimeHybridAutomaton::reset_assignments(
    DiscreteLocation location, DiscreteEvent event) const {
    return this->mode(location)._resets[event];
}

} // namespace Ariadne
