// Verify safe ownership, incremental construction and dimension validation.
#include <stdexcept>
#include "ariadne-system.hpp"

using namespace Ariadne;

int main() {
    const Matrix<Real> A=Matrix<Real>::identity(2);
    const Matrix<Real> B=Matrix<Real>::identity(2);
    const DiagonalMatrix<Real> D(Array<Real>{1,-1});
    FirstOrderPDE model=FirstOrderPDE::homogeneous(A,2);
    if (model.number_of_components()!=2u || model.spatial_dimension()!=0u) { return 1; }
    model.add_direction(B,D,B).add_direction(B,D,B);
    model.validate();
    if (model.spatial_dimension()!=2u || model.source().result_size()!=2u) { return 2; }
    // Copies must own their own source and decomposition arrays.
    auto copy=model;
    if (copy.spatial_matrices().size()!=2u || copy.characteristic_values().size()!=2u) { return 3; }
    bool rejected=false;
    try { model.add_direction(B,D,B); }
    catch (std::invalid_argument const&) { rejected=true; }
    if (!rejected) { return 4; }
    rejected=false;
    try {
        FirstOrderPDE incomplete=FirstOrderPDE::homogeneous(A,2);
        incomplete.add_direction(B,D,B);
        incomplete.validate();
    } catch (std::invalid_argument const&) { rejected=true; }
    if (!rejected) { return 5; }
    rejected=false;
    try { model.add_direction(Matrix<Real>::identity(1),D,B); }
    catch (std::invalid_argument const&) { rejected=true; }
    if (!rejected) { return 6; }
    return 0;
}
