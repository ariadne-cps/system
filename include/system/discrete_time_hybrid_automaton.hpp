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
#include <variant>

#include "system/discrete_time_hybrid_automaton_interface.hpp"
#include "system/hybrid_mode_registry.hpp"
#include "system/hybrid_mode_structure.hpp"

namespace Ariadne {

//! \ingroup SystemModule
//! \brief Per-location deterministic or set-valued discrete-time dynamics.
//! \details Exactly one dynamics declaration is stored in each mode.\n//! A progression invariant is stored for every transition.
//! Urgent invariants are derived as the complement of the guard; permissive
//! invariants must be supplied explicitly and may overlap the guard.
class DiscreteTimeHybridMode : public HybridModeStructure {
    friend class DiscreteTimeHybridAutomaton;

  private:
    using EquationHandle = std::shared_ptr<const DifferenceAlgebraicEquation>;
    using InclusionHandle = std::shared_ptr<const DifferenceAlgebraicInclusion>;
    std::variant<std::monostate, EquationHandle, InclusionHandle> _dynamics;
    Map<DiscreteEvent, ContinuousPredicate> _guards;
    Map<DiscreteEvent, ContinuousPredicate> _invariants;
    Map<DiscreteEvent, DiscreteTimeTransitionKind> _kinds;
    Map<DiscreteEvent, List<PrimedRealAssignment>> _resets;

  public:
    DiscreteTimeHybridMode() = default;
    DiscreteTimeHybridMode(DiscreteLocation location, const DifferenceAlgebraicEquation& equation)
        : HybridModeStructure(location),
          _dynamics(std::in_place_type<EquationHandle>,
                    std::make_shared<const DifferenceAlgebraicEquation>(equation)) {}
    DiscreteTimeHybridMode(DiscreteLocation location, const DifferenceAlgebraicInclusion& inclusion)
        : HybridModeStructure(location),
          _dynamics(std::in_place_type<InclusionHandle>,
                    std::make_shared<const DifferenceAlgebraicInclusion>(inclusion)) {}

    Bool has_equation() const { return std::holds_alternative<EquationHandle>(_dynamics); }
    Bool has_inclusion() const { return std::holds_alternative<InclusionHandle>(_dynamics); }

    const DifferenceAlgebraicEquation& equation() const {
        if (const auto* handle = std::get_if<EquationHandle>(&_dynamics))
            return **handle;
        throw std::logic_error("DiscreteTimeHybridMode does not contain a difference equation");
    }
    const DifferenceAlgebraicInclusion& inclusion() const {
        if (const auto* handle = std::get_if<InclusionHandle>(&_dynamics))
            return **handle;
        throw std::logic_error("DiscreteTimeHybridMode does not contain a difference inclusion");
    }

    DimensionType dimension() const { return has_equation() ? equation().dimension() : inclusion().dimension(); }
    RealSpace state_space() const { return has_equation() ? equation().state_space() : inclusion().state_space(); }
    RealSpace state_auxiliary_space() const {
        return has_equation() ? equation().state_auxiliary_space() : inclusion().state_auxiliary_space();
    }
    const EffectiveVectorMultivariateFunction& update_function() const {
        return has_equation() ? equation().update_function() : inclusion().update_function();
    }
    const EffectiveVectorMultivariateFunction& auxiliary_function() const {
        return has_equation() ? equation().auxiliary_function() : inclusion().auxiliary_function();
    }
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

    //! \brief Add a distinguishable location with deterministic difference equations.
    Void new_mode(DiscreteLocation location, const DifferenceAlgebraicEquation& equation);
    //! \brief Add a distinguishable location with bounded-input difference inclusion.
    Void new_mode(DiscreteLocation location, const DifferenceAlgebraicInclusion& inclusion);

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

    Bool has_equation(DiscreteLocation location) const override;
    Bool has_inclusion(DiscreteLocation location) const override;
    const DifferenceAlgebraicEquation& equation(DiscreteLocation location) const override;
    const DifferenceAlgebraicInclusion& inclusion(DiscreteLocation location) const override;
    const EffectiveVectorMultivariateFunction& update_function(DiscreteLocation location) const override;
    const EffectiveVectorMultivariateFunction& auxiliary_function(DiscreteLocation location) const override;
    RealSpace state_space(DiscreteLocation location) const override;
    DiscreteTimeTransitionKind transition_kind(DiscreteLocation location, DiscreteEvent event) const override;
    ContinuousPredicate guard_predicate(DiscreteLocation location, DiscreteEvent event) const override;
    ContinuousPredicate invariant_predicate(DiscreteLocation location, DiscreteEvent event) const override;
    List<PrimedRealAssignment> reset_assignments(DiscreteLocation location, DiscreteEvent event) const override;
};

} // namespace Ariadne

#endif // ARIADNE_DISCRETE_TIME_HYBRID_AUTOMATON_HPP
