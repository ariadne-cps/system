/***************************************************************************
 *            dynamics/differential_inclusion.hpp
 *
 *  Copyright  2018-20  Luca Geretti
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

/** @file system/differential_inclusion.hpp
 *  @brief Differential inclusions for continuous dynamics.
 */

#ifndef ARIADNE_DIFFERENTIAL_INCLUSION_HPP
#define ARIADNE_DIFFERENTIAL_INCLUSION_HPP

#include "function/function.hpp"
#include "geometry/set_interface.hpp"
#include "symbolic/expression.decl.hpp"
#include "symbolic/identifier.hpp"

#include "expression-set/expression_set.hpp"

namespace Ariadne {

/** @brief Grid for differential inclusions for continuous dynamics. */
class Grid;

/** @brief MissingInputException for differential inclusions for continuous dynamics. */
class MissingInputException : public std::runtime_error {
  public:
    /** @brief Construct the object. */
    MissingInputException(const String& str) : std::runtime_error(str) {}
};

/** @brief UnusedInputException for differential inclusions for continuous dynamics. */
class UnusedInputException : public std::runtime_error {
  public:
    /** @brief Construct the object. */
    UnusedInputException(const String& str) : std::runtime_error(str) {}
};

//! \brief A differential inclusion in Euclidean space.
class DifferentialInclusion {
  public:
    //! \brief The type used to represent time.
    typedef Real TimeType;
    //! \brief The type used to represent real numbers.
    typedef Real RealType;
    //! \brief The type used to describe the state space.
    typedef EuclideanSpace StateSpaceType;

  public:
    //! \brief Construct from expressions with dotted \a dynamics and a given range for the \a inputs.
    DifferentialInclusion(const DottedRealAssignments& dynamics, const RealVariablesBox& inputs);
    /** @brief Destroy the object. */
    virtual ~DifferentialInclusion() = default;
    /** @brief Perform the system operation. */
    virtual DifferentialInclusion* clone() const { return new DifferentialInclusion(*this); }

    /** @brief Return the requested system property. */
    SizeType dimension() const { return _function.result_size(); }
    /** @brief Perform the system operation. */
    SizeType number_of_inputs() const { return _inputs.size(); }
    /** @brief Perform the system operation. */
    RealSpace state_space() const;
    /** @brief Perform the system operation. */
    Grid grid() const;

    //! \brief If the dynamics is affine in the inputs.
    Bool is_input_affine() const;
    //! \brief If the inputs are additive (includes the case of constant multipliers).
    Bool is_input_additive() const;

    /** @brief Return the requested system property. */
    const EffectiveVectorMultivariateFunction& function() const { return _function; }
    /** @brief Perform the system operation. */
    const BoxDomainType& inputs() const { return _inputs; }

    //! \brief Return the dynamics component obtained by setting the input radius to zero.
    EffectiveVectorMultivariateFunction noise_independent_component() const;
    //! \brief Return the dynamics components given by the derivatives for each input.
    Vector<EffectiveVectorMultivariateFunction> input_derivatives() const;

    friend OutputStream& operator<<(OutputStream& os, const DifferentialInclusion& vf) {
        return os << "DifferentialInclusion( " << vf.function() << ", " << vf.inputs() << " )";
    }

  private:
    /** @brief Perform the system operation. */
    Void _transform_and_assign(const EffectiveVectorMultivariateFunction& function, const BoxDomainType& inputs);

  private:
    EffectiveVectorMultivariateFunction _function;
    BoxDomainType _inputs;
    List<Identifier> _variable_names;
};

} // namespace Ariadne

#endif // ARIADNE_DIFFERENTIAL_INCLUSION_HPP
