/***************************************************************************
 *            hybrid_mode_registry.hpp
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

/** @file system/hybrid_mode_registry.hpp
 *  @brief Shared time-independent storage and location matching for hybrid modes.
 */

#ifndef ARIADNE_HYBRID_MODE_REGISTRY_HPP
#define ARIADNE_HYBRID_MODE_REGISTRY_HPP

#include <optional>

#include "hybrid/discrete_location.hpp"
#include "system/discrete_event.hpp"
#include "utility/container.hpp"

namespace Ariadne {

//! \ingroup SystemModule
//! \brief Reusable registry of modes and their discrete locations.
//! \details The registry only implements matching and storage. Concrete
//! systems validate their dynamics, guards, resets and event semantics.
template<class Mode>
class HybridModeRegistry {
  protected:
    Map<DiscreteLocation, Mode> _modes;

    /** @brief Structural result of checking a proposed labelled transition.
     *  @details Only mode existence and uniqueness of the outgoing event
     *  are tested. Guards, resets and event timing remain owner-specific.
     */
    enum class TransitionStatus { VALID, MISSING_SOURCE, MISSING_TARGET, DUPLICATE_EVENT };

    /** @brief Validate a transition against the current registry.
     *  @details Check source, target and existing event in that order.
     *  This preserves the diagnostics precedence of the continuous model.
     *  The mode type must expose has_transition(DiscreteEvent).
     */
    TransitionStatus _transition_status(DiscreteLocation source, DiscreteEvent event, DiscreteLocation target) const {
        if (!this->has_location(source))
            return TransitionStatus::MISSING_SOURCE;
        if (!this->has_location(target))
            return TransitionStatus::MISSING_TARGET;
        const Mode* source_mode = this->_find_mode(source);
        if (source_mode->has_transition(event))
            return TransitionStatus::DUPLICATE_EVENT;
        return TransitionStatus::VALID;
    }

    /** @brief Find the first stored location indistinguishable from a candidate.
     *  @details The owner must reject this candidate before adding a mode.
     */
    std::optional<DiscreteLocation> _first_indistinguishable_mode(DiscreteLocation candidate) const {
        for (typename Map<DiscreteLocation, Mode>::ConstIterator it = _modes.begin();
             it != _modes.end(); ++it) {
            if (!are_distinguishable(candidate, it->first))
                return it->first;
        }
        return std::nullopt;
    }

    /** @brief Find the first stored mode whose location is a restriction of the query.
     *  @details This preserves the existing behaviour for queries containing
     *  additional location coordinates.
     */
    const Mode* _find_mode(DiscreteLocation query) const {
        for (typename Map<DiscreteLocation, Mode>::ConstIterator it = _modes.begin();
             it != _modes.end(); ++it) {
            if (is_restriction(it->first, query))
                return &it->second;
        }
        return nullptr;
    }

  public:
    /** @brief Set of registered locations. */
    Set<DiscreteLocation> locations() const { return _modes.keys(); }

    /** @brief Whether this exact location is present as a registry key. */
    Bool has_location(DiscreteLocation location) const { return _modes.has_key(location); }

    /** @brief Whether a stored mode has the same discrete location as the query. */
    Bool has_mode(DiscreteLocation query) const {
        for (typename Map<DiscreteLocation, Mode>::ConstIterator it = _modes.begin();
             it != _modes.end(); ++it) {
            if (are_same(it->first, query))
                return true;
        }
        return false;
    }

    /** @brief Whether a stored mode is compatible with an extended location query. */
    Bool has_partial_mode(DiscreteLocation query) const { return _find_mode(query) != nullptr; }

    /** @brief Read-only map of registered modes, keyed by discrete location. */
    const Map<DiscreteLocation, Mode>& modes() const { return _modes; }
};

} // namespace Ariadne

#endif // ARIADNE_HYBRID_MODE_REGISTRY_HPP
