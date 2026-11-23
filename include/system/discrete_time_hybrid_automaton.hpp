/***************************************************************************
 *            discrete_time_hybrid_automaton.hpp
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

/** @file system/discrete_time_hybrid_automaton.hpp
 *  @brief Native hybrid automaton with integer-time algebraic equations.
 */

#ifndef ARIADNE_DISCRETE_TIME_HYBRID_AUTOMATON_HPP
#define ARIADNE_DISCRETE_TIME_HYBRID_AUTOMATON_HPP

#include <memory>
#include <stdexcept>

#include "system/discrete_time_hybrid_automaton_interface.hpp"
#include "system/hybrid_mode_registry.hpp"
#include "system/hybrid_mode_structure.hpp"

namespace Ariadne {

//! \ingroup SystemModule
//! \brief Per-location discrete-time dynamics with transition semantics.
//! \details A progression invariant is stored for every transition.
//! Urgent invariants are derived as the complement of the guard; permissive
//! invariants must be supplied explicitly and may overlap the guard.
class DiscreteTimeHybridMode : public HybridModeStructure {
    friend class DiscreteTimeHybridAutomaton;

  private:
    std::shared_ptr<const DifferenceAlgebraicEquation> _equation;
    Map<DiscreteEvent, ContinuousPredicate> _guards;
    Map<DiscreteEvent, ContinuousPredicate> _invariants;
    Map<DiscreteEvent, DiscreteTimeTransitionKind> _kinds;
    Map<DiscreteEvent, List<PrimedRealAssignment>> _resets;

  public:
    DiscreteTimeHybridMode() = default;
    DiscreteTimeHybridMode(DiscreteLocation location, const DifferenceAlgebraicEquation& equation)
        : HybridModeStructure(location), _equation(std::make_shared<const DifferenceAlgebraicEquation>(equation)) {}

    const DifferenceAlgebraicEquation& equation() const {
        if (!_equation)
            throw std::logic_error("DiscreteTimeHybridMode has no discrete-time equations");
        return *_equation;
    }

    const IteratedMap& iterated_map() const { return equation().iterated_map(); }
};

//! \ingroup SystemModule
//! \brief A native, non-composite hybrid system with integer time.
//! \details Independent of continuous-time flow and event crossing semantics.
//! Guard testing and map/reset image computation belong to discrete evolvers.
class DiscreteTimeHybridAutomaton
    : public DiscreteTimeHybridAutomatonInterface, protected HybridModeRegistry<DiscreteTimeHybridMode> {
  private:
    Identifier _name;

    Void _new_transition(DiscreteLocation source, DiscreteEvent event, DiscreteLocation target,
                         const List<PrimedRealAssignment>& reset, ContinuousPredicate guard,
                         ContinuousPredicate invariant, DiscreteTimeTransitionKind kind);

  public:
    DiscreteTimeHybridAutomaton();
    explicit DiscreteTimeHybridAutomaton(Identifier name);
    ~DiscreteTimeHybridAutomaton() override = default;

    DiscreteTimeHybridAutomaton* clone() const override;
    const Identifier& name() const override { return _name; }

    //! \brief Add a distinguishable location with explicit discrete algebraic equations.
    Void new_mode(DiscreteLocation location, const DifferenceAlgebraicEquation& equation);

    //! \brief Add an urgent transition with implicit invariant equal to !guard.
    Void new_urgent_transition(DiscreteLocation source, DiscreteEvent event, DiscreteLocation target,
                               const List<PrimedRealAssignment>& reset, ContinuousPredicate guard);

    //! \brief Add a permissive transition and an explicit progression invariant.
    //! \details When guard and invariant overlap, both jump and map progression
    //! are allowed as separate nondeterministic alternatives.
    Void new_permissive_transition(DiscreteLocation source, DiscreteEvent event, DiscreteLocation target,
                                   const List<PrimedRealAssignment>& reset, ContinuousPredicate guard,
                                   ContinuousPredicate invariant);

    const DiscreteTimeHybridMode& mode(DiscreteLocation location) const;

    Set<DiscreteLocation> locations() const { return HybridModeRegistry<DiscreteTimeHybridMode>::locations(); }
    Bool has_location(DiscreteLocation location) const { return HybridModeRegistry<DiscreteTimeHybridMode>::has_location(location); }
    Bool has_mode(DiscreteLocation location) const override { return HybridModeRegistry<DiscreteTimeHybridMode>::has_mode(location); }
    Bool has_partial_mode(DiscreteLocation location) const { return HybridModeRegistry<DiscreteTimeHybridMode>::has_partial_mode(location); }

    Set<DiscreteEvent> events(DiscreteLocation location) const override;
    Bool has_transition(DiscreteLocation source, DiscreteEvent event) const override;
    DiscreteLocation target(DiscreteLocation source, DiscreteEvent event) const override;
    DimensionType dimension(DiscreteLocation location) const override;

    const DifferenceAlgebraicEquation& equation(DiscreteLocation location) const override;
    const IteratedMap& iterated_map(DiscreteLocation location) const override;
    RealSpace state_space(DiscreteLocation location) const override;
    DiscreteTimeTransitionKind transition_kind(DiscreteLocation location, DiscreteEvent event) const override;
    ContinuousPredicate guard_predicate(DiscreteLocation location, DiscreteEvent event) const override;
    ContinuousPredicate invariant_predicate(DiscreteLocation location, DiscreteEvent event) const override;
    List<PrimedRealAssignment> reset_assignments(DiscreteLocation location, DiscreteEvent event) const override;
};

} // namespace Ariadne

#endif // ARIADNE_DISCRETE_TIME_HYBRID_AUTOMATON_HPP
