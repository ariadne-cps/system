#include "ariadne-system.hpp"
static_assert(sizeof(Ariadne::NeuralValidatedEvaluator)>0);
int main() {
    Ariadne::DiscreteEvent event("tick");
    return event.name() == "tick" ? 0 : 1;
}
