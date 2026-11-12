/***************************************************************************
 *            system/first_order_pde.hpp
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

/** @file system/first_order_pde.hpp
 *  @brief Constant-coefficient first-order partial differential equations.
 */

#ifndef ARIADNE_SYSTEM_FIRST_ORDER_PDE_HPP
#define ARIADNE_SYSTEM_FIRST_ORDER_PDE_HPP

#include "numeric/real.hpp"
#include "numeric/rational.hpp"
#include "utility/array.hpp"
#include "algebra/matrix.hpp"
#include "algebra/diagonal_matrix.hpp"
#include "function/function.hpp"

namespace Ariadne {

//! \brief A constant-coefficient first-order PDE:
//! \f$ A u_{,t} + \sum_j B_j u_{,x_j} = f(u). \f$
//! The model owns its coefficient matrices, characteristic decompositions,
//! and source function. Solvers and solution histories belong to Evolution.
class FirstOrderPDE
{
  public:
    //! \brief Start a model with its mass matrix and source function.
    FirstOrderPDE(Matrix<Real> const& mass_matrix,
                  EffectiveVectorMultivariateFunction const& source);

    //! \brief Construct a complete model from coefficient and characteristic arrays.
    FirstOrderPDE(Matrix<Real> const& mass_matrix,
                  Array<Matrix<Real>> const& spatial_matrices,
                  Array<DiagonalMatrix<Real>> const& eigenvalues,
                  Array<Matrix<Real>> const& eigenvectors,
                  EffectiveVectorMultivariateFunction const& source);

    //! \brief Construct a homogeneous model for the given number of spatial dimensions.
    static FirstOrderPDE homogeneous(Matrix<Real> const& mass_matrix,
                                     SizeType spatial_dimensions);

    //! \brief Add a spatial direction and its characteristic decomposition.
    //! \details Returns *this for fluent construction.
    FirstOrderPDE& add_direction(Matrix<Real> const& spatial_matrix,
                                 DiagonalMatrix<Real> const& eigenvalues,
                                 Matrix<Real> const& eigenvectors);

    //! \brief Check completeness and consistency of matrix and source dimensions.
    void validate() const;

    /** @brief Perform the system operation. */
    SizeType number_of_components() const;
    /** @brief Perform the system operation. */
    SizeType spatial_dimension() const;

    /** @brief Perform the system operation. */
    Matrix<Real> const& mass_matrix() const;
    /** @brief Perform the system operation. */
    Array<Matrix<Real>> const& spatial_matrices() const;
    /** @brief Perform the system operation. */
    Array<DiagonalMatrix<Real>> const& characteristic_values() const;
    /** @brief Perform the system operation. */
    Array<Matrix<Real>> const& characteristic_transformations() const;
    /** @brief Perform the system operation. */
    EffectiveVectorMultivariateFunction const& source() const;

  private:
    /** @brief Perform the system operation. */
    void _check_mass_and_source() const;
    void _check_direction(Matrix<Real> const& spatial_matrix,
                          DiagonalMatrix<Real> const& eigenvalues,
                          Matrix<Real> const& eigenvectors) const;

    Matrix<Real> _A;
    Array<Matrix<Real>> _Bs;
    Array<DiagonalMatrix<Real>> _Ds;
    Array<Matrix<Real>> _Ts;
    EffectiveVectorMultivariateFunction _f;
};

} // namespace Ariadne

#endif // ARIADNE_SYSTEM_FIRST_ORDER_PDE_HPP
