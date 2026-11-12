/***************************************************************************
 *            system/first_order_pde.cpp
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

/** @file src/first_order_pde.cpp
 *  @brief Implement first-order partial differential equations.
 */

#include "system/first_order_pde.hpp"

#include <stdexcept>
#include <utility>

namespace Ariadne {

FirstOrderPDE::FirstOrderPDE(
    const Matrix<Real>& mass_matrix,
    const EffectiveVectorMultivariateFunction& source)
    : _A(mass_matrix), _f(source) {
    this->_check_mass_and_source();
}

FirstOrderPDE::FirstOrderPDE(
    const Matrix<Real>& mass_matrix,
    const Array<Matrix<Real>>& spatial_matrices,
    const Array<DiagonalMatrix<Real>>& eigenvalues,
    const Array<Matrix<Real>>& eigenvectors,
    const EffectiveVectorMultivariateFunction& source)
    : _A(mass_matrix), _Bs(spatial_matrices), _Ds(eigenvalues),
      _Ts(eigenvectors), _f(source) {
    this->validate();
}

FirstOrderPDE
    FirstOrderPDE::homogeneous(const Matrix<Real>& mass_matrix,
                               SizeType spatial_dimensions) {
    if (spatial_dimensions == 0u) {
        throw std::invalid_argument("FirstOrderPDE needs at least one spatial dimension");
    }
    return FirstOrderPDE(mass_matrix,
                         EffectiveVectorMultivariateFunction::zeros(
                             mass_matrix.row_size(), EuclideanDomain(spatial_dimensions + 1u)));
}

FirstOrderPDE&
    FirstOrderPDE::add_direction(const Matrix<Real>& spatial_matrix,
                                 const DiagonalMatrix<Real>& eigenvalues,
                                 const Matrix<Real>& eigenvectors) {
    this->_check_direction(spatial_matrix, eigenvalues, eigenvectors);
    if (_Bs.size() >= _f.argument_size() - 1u) {
        throw std::invalid_argument("Too many FirstOrderPDE spatial directions");
    }
    const SizeType count = _Bs.size();
    // Array has no push_back; prepare the complete new arrays before replacing
    // the existing data. No borrowed references to caller-owned objects.
    Array<Matrix<Real>> new_Bs(count + 1u, [&](SizeType i) {
        return i == count ? spatial_matrix : _Bs[i];
    });
    Array<DiagonalMatrix<Real>> new_Ds(count + 1u, [&](SizeType i) {
        return i == count ? eigenvalues : _Ds[i];
    });
    Array<Matrix<Real>> new_Ts(count + 1u, [&](SizeType i) {
        return i == count ? eigenvectors : _Ts[i];
    });
    _Bs = std::move(new_Bs);
    _Ds = std::move(new_Ds);
    _Ts = std::move(new_Ts);
    return *this;
}

void FirstOrderPDE::validate() const {
    this->_check_mass_and_source();
    if (_Bs.size() != _Ds.size() || _Bs.size() != _Ts.size() || _Bs.size() != _f.argument_size() - 1u) {
        throw std::invalid_argument("FirstOrderPDE direction count does not match source domain");
    }
    for (SizeType i = 0; i != _Bs.size(); ++i) {
        this->_check_direction(_Bs[i], _Ds[i], _Ts[i]);
    }
}

SizeType FirstOrderPDE::number_of_components() const { return _A.row_size(); }
SizeType FirstOrderPDE::spatial_dimension() const { return _Bs.size(); }

const Matrix<Real>& FirstOrderPDE::mass_matrix() const { return _A; }
const Array<Matrix<Real>>& FirstOrderPDE::spatial_matrices() const { return _Bs; }
const Array<DiagonalMatrix<Real>>& FirstOrderPDE::characteristic_values() const { return _Ds; }
const Array<Matrix<Real>>& FirstOrderPDE::characteristic_transformations() const { return _Ts; }
const EffectiveVectorMultivariateFunction& FirstOrderPDE::source() const { return _f; }

void FirstOrderPDE::_check_mass_and_source() const {
    if (_A.row_size() == 0u || _A.column_size() != _A.row_size()) {
        throw std::invalid_argument("FirstOrderPDE mass matrix must be nonempty and square");
    }
    if (_f.argument_size() < 2u || _f.result_size() != _A.row_size()) {
        throw std::invalid_argument("FirstOrderPDE source has incompatible domain or codomain");
    }
}

void FirstOrderPDE::_check_direction(const Matrix<Real>& B,
                                     const DiagonalMatrix<Real>& D,
                                     const Matrix<Real>& T) const {
    const SizeType n = this->number_of_components();
    if (B.row_size() != n || B.column_size() != n || D.size() != n || T.row_size() != n || T.column_size() != n) {
        throw std::invalid_argument("FirstOrderPDE direction matrices have incompatible sizes");
    }
}

} // namespace Ariadne
