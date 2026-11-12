/***************************************************************************
 *            system/system_function.hpp
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

/** @file system/system_function.hpp
 *  @brief Function builders for symbolic system dynamics.
 */

#ifndef ARIADNE_SYSTEM_SYSTEM_FUNCTION_HPP
#define ARIADNE_SYSTEM_SYSTEM_FUNCTION_HPP

#include "function/function.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/expression.hpp"
#include "symbolic/space.hpp"

namespace Ariadne {

//! Shared symbolic constructors for continuous, discrete and hybrid systems.
EffectiveVectorMultivariateFunction make_auxiliary_function(const Space<Real> &, const List<RealAssignment> &);
/** @brief Build the requested system function. */
EffectiveVectorMultivariateFunction make_dynamic_function(const Space<Real> &, const List<RealAssignment> &, const List<DottedRealAssignment> &);
/** @brief Build the requested system function. */
EffectiveVectorMultivariateFunction make_reset_function(const Space<Real> &, const List<RealAssignment> &, const List<PrimedRealAssignment> &);
/** @brief Build the requested system function. */
EffectiveScalarMultivariateFunction make_constraint_function(const Space<Real> &, const List<RealAssignment> &, const ContinuousPredicate &, Sign);

} // namespace Ariadne

#endif // ARIADNE_SYSTEM_SYSTEM_FUNCTION_HPP
