#ifndef ARIADNE_SYSTEM_HYBRID_PAVING_HPP
#define ARIADNE_SYSTEM_HYBRID_PAVING_HPP

#include "hybrid/hybrid_paving.hpp"

namespace Ariadne {

class HybridAutomatonInterface;

//! Extend a hybrid state paving with the auxiliary variables of a system.
HybridBinaryTreePaving extend_auxiliary(HybridBinaryTreePaving const& hgtp,
                                      HybridAutomatonInterface const& ha);

} // namespace Ariadne

#endif // ARIADNE_SYSTEM_HYBRID_PAVING_HPP
