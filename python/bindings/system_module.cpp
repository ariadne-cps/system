#include "pybind11.hpp"

void system_submodule(pybind11::module& module);

void system_module(pybind11::module& module) {
    system_submodule(module);
}
