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

#include "algebra/diagonal_matrix.hpp"
#include "algebra/matrix.hpp"
#include "function/function.hpp"
#include "numeric/rational.hpp"
#include "numeric/real.hpp"
#include "utility/array.hpp"

namespace Ariadne {

//! \brief A constant-coefficient first-order PDE:
//! \f$ A u_{,t} + \sum_j B_j u_{,x_j} = f(u). \f$
//! The model owns its coefficient matrices, characteristic decompositions,
//! and source function. Solvers and solution histories belong to Evolution.
class FirstOrderPDE {
  public:
    //! \brief Start a model with its mass matrix and source function.
    FirstOrderPDE(const Matrix<Real>& mass_matrix,
                  const EffectiveVectorMultivariateFunction& source);

    //! \brief Construct a complete model from coefficient and characteristic arrays.
    FirstOrderPDE(const Matrix<Real>& mass_matrix,
                  const Array<Matrix<Real>>& spatial_matrices,
                  const Array<DiagonalMatrix<Real>>& eigenvalues,
                  const Array<Matrix<Real>>& eigenvectors,
                  const EffectiveVectorMultivariateFunction& source);

    //! \brief Construct a homogeneous model for the given number of spatial dimensions.
    static FirstOrderPDE homogeneous(const Matrix<Real>& mass_matrix,
                                     SizeType spatial_dimensions);

    //! \brief Add a spatial direction and its characteristic decomposition.
    //! \details Returns *this for fluent construction.
    FirstOrderPDE& add_direction(const Matrix<Real>& spatial_matrix,
                                 const DiagonalMatrix<Real>& eigenvalues,
                                 const Matrix<Real>& eigenvectors);

    //! \brief Check completeness and consistency of matrix and source dimensions.
    void validate() const;

    /** @brief Perform the system operation. */
    SizeType number_of_components() const;
    /** @brief Perform the system operation. */
    SizeType spatial_dimension() const;

    /** @brief Perform the system operation. */
    const Matrix<Real>& mass_matrix() const;
    /** @brief Perform the system operation. */
    const Array<Matrix<Real>>& spatial_matrices() const;
    /** @brief Perform the system operation. */
    const Array<DiagonalMatrix<Real>>& characteristic_values() const;
    /** @brief Perform the system operation. */
    const Array<Matrix<Real>>& characteristic_transformations() const;
    /** @brief Perform the system operation. */
    const EffectiveVectorMultivariateFunction& source() const;

  private:
    /** @brief Perform the system operation. */
    void _check_mass_and_source() const;
    void _check_direction(const Matrix<Real>& spatial_matrix,
                          const DiagonalMatrix<Real>& eigenvalues,
                          const Matrix<Real>& eigenvectors) const;

    Matrix<Real> _A;
    Array<Matrix<Real>> _Bs;
    Array<DiagonalMatrix<Real>> _Ds;
    Array<Matrix<Real>> _Ts;
    EffectiveVectorMultivariateFunction _f;
};

} // namespace Ariadne

#endif // ARIADNE_SYSTEM_FIRST_ORDER_PDE_HPP
