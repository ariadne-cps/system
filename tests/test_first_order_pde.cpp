/***************************************************************************
 *            test_first_order_pde.cpp
 *
 *  Copyright  2026  Luca Geretti
 *
 ****************************************************************************/

/*
 *  This file is part of Threading.
 *
 *  Threading is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Threading is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Threading.  If not, see <https://www.gnu.org/licenses/>.
 */

// Exercises the full FirstOrderPDE constructor and accessors plus all
// validation error paths.

#include <stdexcept>

#include "macro/testing.hpp"

#include "ariadne-system.hpp"

using namespace Ariadne;

class TestFirstOrderPDE {
  public:
    Void test();
  private:
    Void test_full_constructor_and_accessors();
    Void test_validation_errors();
};

Void TestFirstOrderPDE::test_full_constructor_and_accessors()
{
    const Matrix<Real> A=Matrix<Real>::identity(2);
    const Matrix<Real> B=Matrix<Real>::identity(2);
    const DiagonalMatrix<Real> D(Array<Real>{1,-1});
    const EffectiveVectorMultivariateFunction source=
        EffectiveVectorMultivariateFunction::zeros(2,EuclideanDomain(2));

    FirstOrderPDE model(A, Array<Matrix<Real>>{B},
                        Array<DiagonalMatrix<Real>>{D},
                        Array<Matrix<Real>>{B}, source);
    ARIADNE_TEST_EQUALS(model.number_of_components(), 2u);
    ARIADNE_TEST_EQUALS(model.spatial_dimension(), 1u);
    ARIADNE_TEST_EQUALS(model.mass_matrix().row_size(), 2u);
    ARIADNE_TEST_EQUALS(model.mass_matrix().column_size(), 2u);
    ARIADNE_TEST_EQUALS(model.spatial_matrices().size(), 1u);
    ARIADNE_TEST_EQUALS(model.characteristic_values().size(), 1u);
    ARIADNE_TEST_EQUALS(model.characteristic_transformations().size(), 1u);
    ARIADNE_TEST_EQUALS(model.source().result_size(), 2u);
    ARIADNE_TEST_EXECUTE(model.validate());
}

Void TestFirstOrderPDE::test_validation_errors()
{
    const Matrix<Real> A=Matrix<Real>::identity(2);
    const Matrix<Real> B=Matrix<Real>::identity(2);
    const DiagonalMatrix<Real> D(Array<Real>{1,-1});
    const EffectiveVectorMultivariateFunction source2=
        EffectiveVectorMultivariateFunction::zeros(2,EuclideanDomain(2));

    // Zero spatial dimensions are rejected.
    ARIADNE_TEST_THROWS(FirstOrderPDE::homogeneous(A,0u), std::invalid_argument);
    // A nonsquare mass matrix is rejected.
    ARIADNE_TEST_THROWS(FirstOrderPDE(Matrix<Real>(2u,3u),
        Array<Matrix<Real>>{B}, Array<DiagonalMatrix<Real>>{D},
        Array<Matrix<Real>>{B}, source2), std::invalid_argument);
    // A source with the wrong codomain is rejected.
    ARIADNE_TEST_THROWS(FirstOrderPDE(A, Array<Matrix<Real>>{B},
        Array<DiagonalMatrix<Real>>{D}, Array<Matrix<Real>>{B},
        EffectiveVectorMultivariateFunction::zeros(3,EuclideanDomain(2))),
        std::invalid_argument);
    // Direction matrices of the wrong size are rejected.
    ARIADNE_TEST_THROWS(FirstOrderPDE(A, Array<Matrix<Real>>{Matrix<Real>::identity(1)},
        Array<DiagonalMatrix<Real>>{D}, Array<Matrix<Real>>{B}, source2),
        std::invalid_argument);
    // Empty mass matrix is rejected.
    ARIADNE_TEST_THROWS(FirstOrderPDE(Matrix<Real>(0u,0u),
        Array<Matrix<Real>>{B}, Array<DiagonalMatrix<Real>>{D},
        Array<Matrix<Real>>{B}, source2), std::invalid_argument);
    // Source domain with too few arguments is rejected.
    ARIADNE_TEST_THROWS(FirstOrderPDE(A, Array<Matrix<Real>>{B},
        Array<DiagonalMatrix<Real>>{D}, Array<Matrix<Real>>{B},
        EffectiveVectorMultivariateFunction::zeros(2,EuclideanDomain(1u))),
        std::invalid_argument);
    // Spatial matrix with matching rows but wrong columns.
    ARIADNE_TEST_THROWS(FirstOrderPDE(A, Array<Matrix<Real>>{Matrix<Real>(2u,3u)},
        Array<DiagonalMatrix<Real>>{D}, Array<Matrix<Real>>{B}, source2),
        std::invalid_argument);
    // Eigenvalue matrix of the wrong size.
    ARIADNE_TEST_THROWS(FirstOrderPDE(A, Array<Matrix<Real>>{B},
        Array<DiagonalMatrix<Real>>{DiagonalMatrix<Real>(Array<Real>{1})},
        Array<Matrix<Real>>{B}, source2), std::invalid_argument);
    // Eigenvector matrix with wrong rows / wrong columns.
    ARIADNE_TEST_THROWS(FirstOrderPDE(A, Array<Matrix<Real>>{B},
        Array<DiagonalMatrix<Real>>{D}, Array<Matrix<Real>>{Matrix<Real>(3u,2u)},
        source2), std::invalid_argument);
    ARIADNE_TEST_THROWS(FirstOrderPDE(A, Array<Matrix<Real>>{B},
        Array<DiagonalMatrix<Real>>{D}, Array<Matrix<Real>>{Matrix<Real>(2u,3u)},
        source2), std::invalid_argument);
    // Mismatched spatial/eigenvector counts with equal eigenvalue count.
    ARIADNE_TEST_THROWS(FirstOrderPDE(A, Array<Matrix<Real>>{B},
        Array<DiagonalMatrix<Real>>{D}, Array<Matrix<Real>>{}, source2),
        std::invalid_argument);
    // Direction count must match the source domain.
    ARIADNE_TEST_THROWS({
        FirstOrderPDE model=FirstOrderPDE::homogeneous(A,2);
        model.add_direction(B,D,B);
        model.add_direction(B,D,B);
        model.add_direction(B,D,B);
        model.validate();
    }, std::invalid_argument);
}

Void TestFirstOrderPDE::test()
{
    ARIADNE_TEST_CALL(test_full_constructor_and_accessors());
    ARIADNE_TEST_CALL(test_validation_errors());
}

Int main()
{
    ARIADNE_TEST_CALL(TestFirstOrderPDE().test());
    return ARIADNE_TEST_FAILURES;
}
