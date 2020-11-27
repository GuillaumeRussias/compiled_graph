// cppimport

//linker
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
namespace py = pybind11;
//string
#include <iostream>
#include <cassert>
#define assertm(exp, msg) assert(((void)msg, exp))
//vecteur et algorithme de tri
#include <algorithm>
#include <vector>
#include <queue>

using namespace std;

const int day = 3600*24; //nombre de secondes en 1 jour

class myClass
{
public:
    int number;

    myClass();
    void addOne();

    int getNumber();
};

class edge {
    string type; //"walk" or "common" -> mandatorry to separate transfers(no schedule cosntraints) from common transportation (scheduled)
    vector<int> departure_time; //" departure times . empty if type=="walk" . time in seconds
    vector<int> arrival_time; //" arrival times . empty if type=="walk" . time in seconds
    int transfers_cost; // cost in seconds of the transfers (define in condtructor iff type==walk, define after mission() iff type==common)
    int selected_mission;  //selected mission (define after mission() iff type==common)
    void mission(int time);//compute the appropriate mission i of that edge (min time_arrival[i]) s.t (time < time departure[i])  
public :
    edge(const int& size_departure_time, const int& size_arrival_time); //common constructor
    edge(int cost); //walk constructor
    void set_time(const int index,const int& t_departure, const int& t_arrival);
    string repr();
    int cost(); //time independent : totaly ignore edges of type common , works with distance graph 
    int cost(int time); //time dependent : works with both gtfs,old graph but takes more time to compute.
    };


class vertex { 
    int i;
    vector<vertex*> neighbours;
    vector<edge*> edges;
public :
    int time;
    bool visited;
    vertex* predecessor;

    vertex(int index);
    void push_neihghbour(vertex* neighbour);
    void push_edge(edge* edge);
    unsigned number_neighbour();//returns size od neighbours
    int get_index();//returns this->i;
    vertex* get_neighbour(const int& j); // safe access to the jth neighbour of this vertex
    int cost_of_travel(const int& j); //returns the cost of the edge between this vertex and the its jth neighbour. time independent
    int cost_of_travel(const int& j, int time); //returns the cost of the edge between this vertex and the its jth neighbour. time dependent
    edge* operator[](const int& j);//safe access to the edge (if it exists) between this and j !return_value_policy::reference! we want c++ to be in charge of the destruction of this object
};


class graph {
    vector<vertex*> v_list;
    vector<edge*> e_list;
    void push_walk_edge(int departure_index, int arrival_index, int cost); //push a single walk edge
    void push_common_edge(int departure_index, int arrival_index, const int& size_departure_time, const int& size_arrival_time); //push a single common edge !care departure_time and arrivam_time are initialised but with no value!
public :
    //tools functions
    graph(const int &size_v); //constructor
    ~graph();//destructor
    void build_common_edges(py::array_t<int> departure_index, py::array_t<int> arrival_index, py::array_t<int> departure_time, py::array_t<int> arrival_time); //construct all common edges
    void build_walk_edges(py::array_t<int> departure_index, py::array_t<int> arrival_index, py::array_t<int> cost); //construct all walk edges
    vertex* operator[](const int& i); //safe access to the ith vertex of the graph !return_value_policy::reference! we want c++ to be in charge of the destruction of this object
    void initialised(); //set (visited,time) at (false,7*day)

    //algorithms : carefull : graph needs to be re-initialised (visited, time) after the execution of these 4 algorithms
    void basic_djikstra(const int& start_vertex_index); //basic djikstra 
    void time_djikstra(const int& start_vertex_index, int t); //time dependant djikstra
    void stop_basic_djikstra(const int& start_vertex_index, const int& end_vertex_index); //basic djikstra , with stop condition 
    void stop_time_djikstra(const int& start_vertex_index, const int& end_vertex_index, int t); //time dependant djikstra , with stop condition

    //user interface
    vector<int> path_finder(const int& start_vertex_index, const int& end_vertex_index); //returns path from start to end (index of vertex) trow exception if no path is find , graph re-initialised after execution
    vector<int> path_finder_time(const int& start_vertex_index, const int& end_vertex_index, int t); //TIME . returns path from start to end (index of vertex) trow exception if no path is find , graph re-initialised after execution

};

class comparetime {
public:
    bool operator()( vertex* A, vertex* B ) {
        return (A->time >= B->time);
    }
};
