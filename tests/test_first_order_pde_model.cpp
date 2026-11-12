/***************************************************************************
 *            tests/test_first_order_pde_model.cpp
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

/** @file tests/test_first_order_pde_model.cpp
 *  @brief Test first order pde model behavior.
 */

// Verify safe ownership, incremental construction and dimension validation.
#include <stdexcept>

#include "ariadne-system.hpp"

using namespace Ariadne;

int main() {
    const Matrix<Real> A = Matrix<Real>::identity(2);
    const Matrix<Real> B = Matrix<Real>::identity(2);
    const DiagonalMatrix<Real> D(Array<Real>{1, -1});
    FirstOrderPDE model = FirstOrderPDE::homogeneous(A, 2);
    if (model.number_of_components() != 2u || model.spatial_dimension() != 0u) {
        return 1;
    }
    model.add_direction(B, D, B).add_direction(B, D, B);
    model.validate();
    if (model.spatial_dimension() != 2u || model.source().result_size() != 2u) {
        return 2;
    }
    // Copies must own their own source and decomposition arrays.
    auto copy = model;
    if (copy.spatial_matrices().size() != 2u || copy.characteristic_values().size() != 2u) {
        return 3;
    }
    bool rejected = false;
    try {
        model.add_direction(B, D, B);
    } catch (const std::invalid_argument &) {
        rejected = true;
    }
    if (!rejected) {
        return 4;
    }
    rejected = false;
    try {
        FirstOrderPDE incomplete = FirstOrderPDE::homogeneous(A, 2);
        incomplete.add_direction(B, D, B);
        incomplete.validate();
    } catch (const std::invalid_argument &) {
        rejected = true;
    }
    if (!rejected) {
        return 5;
    }
    rejected = false;
    try {
        model.add_direction(Matrix<Real>::identity(1), D, B);
    } catch (const std::invalid_argument &) {
        rejected = true;
    }
    if (!rejected) {
        return 6;
    }
    return 0;
}
