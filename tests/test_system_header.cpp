#include "ariadne-system.hpp"
int main() {
    Ariadne::DiscreteEvent event("tick");
    return event.name() == "tick" ? 0 : 1;
}
