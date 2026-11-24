/***************************************************************************
 *            hybrid_automaton_interface.hpp
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

/** @file system/hybrid_automaton_interface.hpp
 *  @brief Time-independent structural contract shared by hybrid automata.
 */

#ifndef ARIADNE_HYBRID_AUTOMATON_INTERFACE_HPP
#define ARIADNE_HYBRID_AUTOMATON_INTERFACE_HPP

#include "function/function.hpp"
#include "hybrid/discrete_location.hpp"
#include "system/discrete_event.hpp"

namespace Ariadne {

//! \ingroup SystemModule
//! \brief Common structural operations independent of the time model.
//! \details Dynamics, guard semantics and time belong to derived interfaces.
class HybridAutomatonInterface {
  public:
    virtual ~HybridAutomatonInterface() = default;

    //! \brief Name of the automaton.
    virtual const Identifier& name() const = 0;

    //! \brief Whether a location identifies a valid mode.
    virtual Bool has_mode(DiscreteLocation location) const = 0;

    //! \brief Events available at the location.
    virtual Set<DiscreteEvent> events(DiscreteLocation location) const = 0;

    //! \brief Whether the event has a transition from this location.
    virtual Bool has_transition(DiscreteLocation source, DiscreteEvent event) const = 0;

    //! \brief Target of the named transition.
    virtual DiscreteLocation target(DiscreteLocation source, DiscreteEvent event) const = 0;

    //! \brief Physical state dimension in this location.
    virtual DimensionType dimension(DiscreteLocation location) const = 0;
};

} // namespace Ariadne

#endif // ARIADNE_HYBRID_AUTOMATON_INTERFACE_HPP
