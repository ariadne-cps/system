/***************************************************************************
 *            dynamics/vector_field.hpp
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

/** @file system/vector_field.hpp
 *  @brief Continuous-time vector fields and dynamical systems.
 */

#ifndef ARIADNE_VECTOR_FIELD_HPP
#define ARIADNE_VECTOR_FIELD_HPP

#include <memory>

#include "function/function.hpp"
#include "geometry/set_interface.hpp"
#include "symbolic/expression.decl.hpp"

#include "symbolic/variable.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/space.hpp"


namespace Ariadne {

//! \brief A vector field in Euclidean space.
class VectorField
{
  public:
    //! \brief The type used to represent time.
    typedef Real TimeType;
    //! \brief The type used to represent real numbers.
    typedef Real RealType;
    //! \brief The type used to describe the state space.
    typedef EuclideanSpace StateSpaceType;
  public:
    /** @brief Construct the object. */
    VectorField(const List<DottedRealAssignment>&);
    /** @brief Construct the object. */
    VectorField(const List<DottedRealAssignment>&, List<RealAssignment> const&);

    /** @brief Return the requested system property. */
    const EffectiveVectorMultivariateFunction& dynamic_function() const { return this->_dynamic_function; }
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
    VectorField* clone() const { return new VectorField(*this); }
    /** @brief Destroy the object. */
    ~VectorField() = default;
    /** @brief Return the requested system property. */
    DimensionType dimension() const { return this->_dynamic_function.result_size(); }
    /** @brief Return the requested system property. */
    const EffectiveVectorMultivariateFunction& function() const { return _dynamic_function; }

    friend OutputStream& operator<<(OutputStream& os, VectorField const& vector_field);
  private:
    List<DottedRealAssignment> _dynamics;
    List<RealAssignment> _auxiliary;
    EffectiveVectorMultivariateFunction _dynamic_function;
    EffectiveVectorMultivariateFunction _auxiliary_function;
};

} // namespace Ariadne

#endif // ARIADNE_VECTOR_FIELD_HPP
