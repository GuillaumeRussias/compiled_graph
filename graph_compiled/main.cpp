// cppimport
#include <pybind11/pybind11.h>
#include "graph.h"

namespace py = pybind11;

PYBIND11_MODULE(fast_graph, m){
    m.doc() = R"pbdoc(
        Pybind11 example plugin

    )pbdoc";

    py::class_<myClass>(m, "myClass")
        .def(py::init<>())
        .def("addOne", &myClass::addOne)
        .def("getNumber", &myClass::getNumber)
        ;
    py::class_<edge>(m, "edge")
        ;
    py::class_<vertex>(m, "vertex")
            .def("__getitem__", &vertex::operator[], py::return_value_policy::reference)
        ;
    py::class_<graph>(m, "graph")
            .def(py::init<const int&>())
            .def("build_common_edges", &graph::build_common_edges)
            .def("build_walk_edges", &graph::build_walk_edges)
            .def("__getitem__", &graph::operator[], py::return_value_policy::reference)
            .def("basic_path_finder",&graph::path_finder)
            .def("time_path_finder",&graph::path_finder_time)
            ;
}