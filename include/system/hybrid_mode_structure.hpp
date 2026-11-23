/***************************************************************************
 *            hybrid_mode_structure.hpp
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

/** @file system/hybrid_mode_structure.hpp
 *  @brief Time-independent location and labelled transition topology for a hybrid mode.
 */

#ifndef ARIADNE_HYBRID_MODE_STRUCTURE_HPP
#define ARIADNE_HYBRID_MODE_STRUCTURE_HPP

#include "hybrid/discrete_location.hpp"
#include "system/discrete_event.hpp"
#include "utility/container.hpp"

namespace Ariadne {

//! \ingroup SystemModule
//! \brief Time-independent mode location and labelled transition targets.
//! \details Guards, resets, invariant predicates, event kinds and dynamics
//! remain in specialised models; composing this topology does not compose them.
class HybridModeStructure {
  protected:
    DiscreteLocation _location;
    Map<DiscreteEvent, DiscreteLocation> _targets;

  public:
    /** @brief Construct an empty structural mode. */
    HybridModeStructure() = default;

    /** @brief Construct a mode with the given location. */
    explicit HybridModeStructure(DiscreteLocation location) : _location(location) {}

    /** @brief The mode's discrete location. */
    DiscreteLocation location() const { return _location; }

    /** @brief Whether this mode has a transition bearing the given event. */
    Bool has_transition(DiscreteEvent event) const { return _targets.has_key(event); }

    /** @brief Labels of transitions outgoing from the mode. */
    Set<DiscreteEvent> transition_events() const { return _targets.keys(); }

    /** @brief The target of an event, or the supplied location when absent.
     *  @details The caller supplies the fallback to preserve its own location
     *  projection semantics in composite or partially specified queries.
     */
    DiscreteLocation transition_target(DiscreteEvent event, DiscreteLocation fallback) const {
        return _targets.has_key(event) ? _targets[event] : fallback;
    }

    /** @brief Record an already validated transition target.
     *  @details Event uniqueness and reset validity are checked by the owner.
     */
    Void add_transition(DiscreteEvent event, DiscreteLocation target) {
        _targets.insert(event, target);
    }

    /** @brief Adjoin the topology of a component in a product mode.
     *  @details Overlapping event labels combine their targets by location
     *  join, matching the established product-automaton construction.
     */
    Void adjoin_structure(const HybridModeStructure& component) {
        _location.adjoin(component._location);
        for (Map<DiscreteEvent, DiscreteLocation>::ConstIterator it = component._targets.begin();
             it != component._targets.end(); ++it) {
            DiscreteLocation& target = _targets[it->first];
            target = join(target, it->second);
        }
    }
};

} // namespace Ariadne

#endif // ARIADNE_HYBRID_MODE_STRUCTURE_HPP
