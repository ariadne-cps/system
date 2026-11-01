// Mathematical model only. Solvers and solution histories live in evolution/.
#ifndef ARIADNE_SYSTEM_FIRST_ORDER_PDE_HPP
#define ARIADNE_SYSTEM_FIRST_ORDER_PDE_HPP

#include <stdexcept>
#include <utility>
#include "numeric/real.hpp"
#include "numeric/rational.hpp"
#include "algebra/matrix.hpp"
#include "algebra/diagonal_matrix.hpp"
#include "algebra/tensor.hpp"
#include "function/function.hpp"

namespace Ariadne {

//! A constant-coefficient first-order PDE:
//! A du/dt + sum_j B_j du/dx_j = f(u).
//! The model owns all its coefficients and characteristic decompositions.
//! A direction comprises B_j, its diagonal eigenvalue matrix D_j and
//! its eigenvector matrix T_j. Solvers may impose additional restrictions.
class FirstOrderPDE {
  public:
    //! Begin construction with a mass matrix and source term.
    FirstOrderPDE(Matrix<Real> const& mass_matrix,
                  EffectiveVectorMultivariateFunction const& source)
        : _A(mass_matrix), _f(source) {
        this->_check_mass_and_source();
    }

    //! Preserve the original five-argument construction, but OWN all data.
    FirstOrderPDE(Matrix<Real> const& mass_matrix,
                  Array<Matrix<Real>> const& spatial_matrices,
                  Array<DiagonalMatrix<Real>> const& eigenvalues,
                  Array<Matrix<Real>> const& eigenvectors,
                  EffectiveVectorMultivariateFunction const& source)
        : _A(mass_matrix), _Bs(spatial_matrices), _Ds(eigenvalues),
          _Ts(eigenvectors), _f(source) {
        this->validate();
    }

    //! Factory for homogeneous PDEs without a source term.
    static FirstOrderPDE homogeneous(Matrix<Real> const& mass_matrix,
                                     SizeType spatial_dimensions) {
        if (spatial_dimensions == 0u) {
            throw std::invalid_argument("FirstOrderPDE needs at least one spatial dimension");
        }
        return FirstOrderPDE(mass_matrix,
            EffectiveVectorMultivariateFunction::zeros(
                mass_matrix.row_size(), EuclideanDomain(spatial_dimensions+1)));
    }

    //! Add one characteristic direction; returns *this for fluent construction.
    FirstOrderPDE& add_direction(Matrix<Real> const& spatial_matrix,
                                 DiagonalMatrix<Real> const& eigenvalues,
                                 Matrix<Real> const& eigenvectors) {
        this->_check_direction(spatial_matrix,eigenvalues,eigenvectors);
        if (_Bs.size() >= _f.argument_size()-1u) {
            throw std::invalid_argument("Too many FirstOrderPDE spatial directions");
        }
        const SizeType count=_Bs.size();
        // Ariadne::Array has no push_back; construct the enlarged arrays
        // before changing the model, so failed allocation leaves it intact.
        Array<Matrix<Real>> new_Bs(count+1u,[&](SizeType i) {
            return i==count ? spatial_matrix : _Bs[i];
        });
        Array<DiagonalMatrix<Real>> new_Ds(count+1u,[&](SizeType i) {
            return i==count ? eigenvalues : _Ds[i];
        });
        Array<Matrix<Real>> new_Ts(count+1u,[&](SizeType i) {
            return i==count ? eigenvectors : _Ts[i];
        });
        _Bs=std::move(new_Bs);
        _Ds=std::move(new_Ds);
        _Ts=std::move(new_Ts);
        return *this;
    }

    //! Check completeness and dimensions before passing to a solver.
    void validate() const {
        this->_check_mass_and_source();
        if (_Bs.size() != _Ds.size() || _Bs.size() != _Ts.size()
            || _Bs.size() != _f.argument_size()-1u) {
            throw std::invalid_argument("FirstOrderPDE direction count does not match source domain");
        }
        for (SizeType i=0; i!=_Bs.size(); ++i) {
            this->_check_direction(_Bs[i],_Ds[i],_Ts[i]);
        }
    }

    SizeType number_of_components() const { return _A.row_size(); }
    SizeType spatial_dimension() const { return _Bs.size(); }
    Matrix<Real> const& mass_matrix() const { return _A; }
    Array<Matrix<Real>> const& spatial_matrices() const { return _Bs; }
    Array<DiagonalMatrix<Real>> const& characteristic_values() const { return _Ds; }
    Array<Matrix<Real>> const& characteristic_transformations() const { return _Ts; }
    EffectiveVectorMultivariateFunction const& source() const { return _f; }

  private:
    void _check_mass_and_source() const {
        if (_A.row_size()==0u || _A.column_size()!=_A.row_size()) {
            throw std::invalid_argument("FirstOrderPDE mass matrix must be nonempty and square");
        }
        if (_f.argument_size()<2u || _f.result_size()!=_A.row_size()) {
            throw std::invalid_argument("FirstOrderPDE source has incompatible domain or codomain");
        }
    }
    void _check_direction(Matrix<Real> const& B,
                          DiagonalMatrix<Real> const& D,
                          Matrix<Real> const& T) const {
        const SizeType n=this->number_of_components();
        if (B.row_size()!=n || B.column_size()!=n || D.size()!=n
            || T.row_size()!=n || T.column_size()!=n) {
            throw std::invalid_argument("FirstOrderPDE direction matrices have incompatible sizes");
        }
    }

    Matrix<Real> _A;
    Array<Matrix<Real>> _Bs;
    Array<DiagonalMatrix<Real>> _Ds;
    Array<Matrix<Real>> _Ts;
    EffectiveVectorMultivariateFunction _f;
};

} // namespace Ariadne
#endif // ARIADNE_SYSTEM_FIRST_ORDER_PDE_HPP
