/***************************************************************************
 *            dynamics/iterated_map.cpp
 *
 *  Copyright  2008-20  Alberto Casagrande, Pieter Collins
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

/** @file src/iterated_map.cpp
 *  @brief Implement discrete-time iterated maps.
 */

#include "system/iterated_map.hpp"

#include "algebra/vector.hpp"
#include "function/constraint.hpp"
#include "function/function.hpp"
#include "function/functional.hpp"
#include "logging/logging.hpp"
#include "symbolic/assignment.hpp"
#include "utility/array.hpp"
#include "utility/macros.hpp"
#include "utility/stlio.hpp"
#include "utility/tuple.hpp"

namespace Ariadne {

EffectiveVectorMultivariateFunction make_auxiliary_function(
    const Space<Real>& state_space,
    const List<RealAssignment>& algebraic);

EffectiveVectorMultivariateFunction make_reset_function(
    const Space<Real>& space,
    const List<RealAssignment>& algebraic,
    const List<PrimedRealAssignment>& differential);

IteratedMap::IteratedMap(const List<PrimedRealAssignment>& updates)
    : IteratedMap(updates, List<RealAssignment>()) {}

IteratedMap::IteratedMap(const List<PrimedRealAssignment>& updates, const List<RealAssignment>& auxiliary)
    : _updates(updates), _auxiliary(auxiliary), _update_function(make_reset_function(left_hand_sides(updates), auxiliary, updates)), _auxiliary_function(make_auxiliary_function(left_hand_sides(updates), auxiliary)) {
}

RealSpace IteratedMap::state_space() const {
    return RealSpace(left_hand_sides(this->_updates));
}

RealSpace IteratedMap::auxiliary_space() const {
    return RealSpace(left_hand_sides(this->_auxiliary));
}

RealSpace IteratedMap::state_auxiliary_space() const {
    return join(state_space(), auxiliary_space());
}

OutputStream& operator<<(OutputStream& os, const IteratedMap& map) {
    os << "IteratedMap( update_function = " << map.update_function() << ", "
                                                                        "auxiliary_function = "
       << map.auxiliary_function() << ", "
                                      "updates = "
       << map._updates << ", "
                          "auxiliary = "
       << map._auxiliary << ")";
    return os;
}

} // namespace Ariadne

