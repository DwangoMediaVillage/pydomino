#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "domino.hpp"

namespace py = pybind11;

PYBIND11_MODULE(pydomino, mod) {
  py::class_<domino::Aligner>(mod, "Aligner")
      .def(py::init<std::string>())
      .def("align", &domino::Aligner::align_phonemes)
      .def("release", &domino::Aligner::release);
}
