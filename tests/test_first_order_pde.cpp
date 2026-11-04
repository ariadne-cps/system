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

#include "ariadne-testing.hpp"

#include "ariadne-system.hpp"

using namespace Ariadne;

class TestFirstOrderPDE : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;
  public:
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
    ARIADNE_TESTING_EXPECT_EQUAL(model.number_of_components(), 2u);
    ARIADNE_TESTING_EXPECT_EQUAL(model.spatial_dimension(), 1u);
    ARIADNE_TESTING_EXPECT_EQUAL(model.mass_matrix().row_size(), 2u);
    ARIADNE_TESTING_EXPECT_EQUAL(model.mass_matrix().column_size(), 2u);
    ARIADNE_TESTING_EXPECT_EQUAL(model.spatial_matrices().size(), 1u);
    ARIADNE_TESTING_EXPECT_EQUAL(model.characteristic_values().size(), 1u);
    ARIADNE_TESTING_EXPECT_EQUAL(model.characteristic_transformations().size(), 1u);
    ARIADNE_TESTING_EXPECT_EQUAL(model.source().result_size(), 2u);
    ARIADNE_TESTING_EXPECT_NO_THROW(model.validate());
}

Void TestFirstOrderPDE::test_validation_errors()
{
    const Matrix<Real> A=Matrix<Real>::identity(2);
    const Matrix<Real> B=Matrix<Real>::identity(2);
    const DiagonalMatrix<Real> D(Array<Real>{1,-1});
    const EffectiveVectorMultivariateFunction source2=
        EffectiveVectorMultivariateFunction::zeros(2,EuclideanDomain(2));

    // Zero spatial dimensions are rejected.
    ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, FirstOrderPDE::homogeneous(A,0u));
    // A nonsquare mass matrix is rejected.
    ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, FirstOrderPDE(Matrix<Real>(2u,3u),
        Array<Matrix<Real>>{B}, Array<DiagonalMatrix<Real>>{D},
        Array<Matrix<Real>>{B}, source2));
    // A source with the wrong codomain is rejected.
    ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, FirstOrderPDE(A, Array<Matrix<Real>>{B},
        Array<DiagonalMatrix<Real>>{D}, Array<Matrix<Real>>{B},
        EffectiveVectorMultivariateFunction::zeros(3,EuclideanDomain(2))));
    // Direction matrices of the wrong size are rejected.
    ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, FirstOrderPDE(A, Array<Matrix<Real>>{Matrix<Real>::identity(1)},
        Array<DiagonalMatrix<Real>>{D}, Array<Matrix<Real>>{B}, source2));
    // Empty mass matrix is rejected.
    ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, FirstOrderPDE(Matrix<Real>(0u,0u),
        Array<Matrix<Real>>{B}, Array<DiagonalMatrix<Real>>{D},
        Array<Matrix<Real>>{B}, source2));
    // Source domain with too few arguments is rejected.
    ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, FirstOrderPDE(A, Array<Matrix<Real>>{B},
        Array<DiagonalMatrix<Real>>{D}, Array<Matrix<Real>>{B},
        EffectiveVectorMultivariateFunction::zeros(2,EuclideanDomain(1u))));
    // Spatial matrix with matching rows but wrong columns.
    ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, FirstOrderPDE(A, Array<Matrix<Real>>{Matrix<Real>(2u,3u)},
        Array<DiagonalMatrix<Real>>{D}, Array<Matrix<Real>>{B}, source2));
    // Eigenvalue matrix of the wrong size.
    ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, FirstOrderPDE(A, Array<Matrix<Real>>{B},
        Array<DiagonalMatrix<Real>>{DiagonalMatrix<Real>(Array<Real>{1})},
        Array<Matrix<Real>>{B}, source2));
    // Eigenvector matrix with wrong rows / wrong columns.
    ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, FirstOrderPDE(A, Array<Matrix<Real>>{B},
        Array<DiagonalMatrix<Real>>{D}, Array<Matrix<Real>>{Matrix<Real>(3u,2u)},
        source2));
    ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, FirstOrderPDE(A, Array<Matrix<Real>>{B},
        Array<DiagonalMatrix<Real>>{D}, Array<Matrix<Real>>{Matrix<Real>(2u,3u)},
        source2));
    // Mismatched spatial/eigenvector counts with equal eigenvalue count.
    ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, FirstOrderPDE(A, Array<Matrix<Real>>{B},
        Array<DiagonalMatrix<Real>>{D}, Array<Matrix<Real>>{}, source2));
    // Direction count must match the source domain.
    ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, {
        FirstOrderPDE model=FirstOrderPDE::homogeneous(A,2);
        model.add_direction(B,D,B);
        model.add_direction(B,D,B);
        model.add_direction(B,D,B);
        model.validate();
    });
}




int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_first_order_pde");
    ARIADNE_TESTING_ADD_TEST(TestFirstOrderPDE, test_full_constructor_and_accessors);
    ARIADNE_TESTING_ADD_TEST(TestFirstOrderPDE, test_validation_errors);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc, argv);
}
