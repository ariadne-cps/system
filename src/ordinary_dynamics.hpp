/***************************************************************************
 *            src/ordinary_dynamics.hpp
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne System.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ARIADNE_ORDINARY_DYNAMICS_INTERNAL_HPP
#define ARIADNE_ORDINARY_DYNAMICS_INTERNAL_HPP

#include <optional>

#include "expression-set/expression_set.hpp"

namespace Ariadne::Detail {
RealSpace variables_space(const RealVariablesBox& variables);
BoxDomainType variables_domain(const RealVariablesBox& variables);
RealSpace ordinary_argument_space(const RealSpace& states,
                                  const std::optional<RealVariable>& time, const RealSpace& parameters,
                                  const RealSpace& inputs, const RealSpace& auxiliary = RealSpace());
} // namespace Ariadne::Detail
#endif
