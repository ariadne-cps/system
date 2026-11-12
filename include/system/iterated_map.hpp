/***************************************************************************
 *            dynamics/iterated_map.hpp
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

/** @file system/iterated_map.hpp
 *  @brief Discrete-time maps over continuous spaces.
 */

#ifndef ARIADNE_ITERATED_MAP_HPP
#define ARIADNE_ITERATED_MAP_HPP

#include <memory>

#include "function/function.hpp"
#include "geometry/set_interface.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/space.hpp"
#include "symbolic/variable.hpp"

namespace Ariadne {

//! \brief An iterated function system.
class IteratedMap {
  public:
    //! \brief The type used to represent time.
    typedef Integer TimeType;
    //! \brief The type used to represent real numbers.
    typedef Real RealType;
    //! \brief The state space
    typedef EuclideanSpace StateSpaceType;

  public:
    /** @brief Construct the object. */
    IteratedMap(const List<PrimedRealAssignment>&);
    /** @brief Construct the object. */
    IteratedMap(const List<PrimedRealAssignment>&, const List<RealAssignment>&);

    /** @brief Perform the system operation. */
    const EffectiveVectorMultivariateFunction& update_function() const { return this->_update_function; }
    /** @brief Perform the system operation. */
    const EffectiveVectorMultivariateFunction& auxiliary_function() const { return this->_auxiliary_function; }
    /** @brief Perform the system operation. */
    const EffectiveVectorMultivariateFunction& auxiliary_mapping() const { return this->_auxiliary_function; }

    /** @brief Perform the system operation. */
    RealSpace state_space() const;
    /** @brief Perform the system operation. */
    RealSpace auxiliary_space() const;
    /** @brief Perform the system operation. */
    RealSpace state_auxiliary_space() const;

    /** @brief Perform the system operation. */
    IteratedMap* clone() const { return new IteratedMap(*this); }
    /** @brief Destroy the object. */
    ~IteratedMap() = default;
    /** @brief Return the requested system property. */
    DimensionType dimension() const { return this->_update_function.result_size(); }
    /** @brief Return the requested system property. */
    const EffectiveVectorMultivariateFunction& function() const { return _update_function; }

    friend OutputStream& operator<<(OutputStream& os, const IteratedMap& map);

  private:
    List<PrimedRealAssignment> _updates;
    List<RealAssignment> _auxiliary;
    EffectiveVectorMultivariateFunction _update_function;
    EffectiveVectorMultivariateFunction _auxiliary_function;
};

} // namespace Ariadne

#endif // ARIADNE_ITERATED_MAP_HPP
