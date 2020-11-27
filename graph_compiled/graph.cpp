// cppimport

#include "graph.h"

myClass::myClass() {
    number = 1;
}
void myClass::addOne() {
    number = number + 1;
}

int myClass::getNumber() {
    return number;
}

vertex::vertex(int index) {
    time = 7*day;
    visited = false;
    i = index;
}
void vertex::push_neihghbour(vertex* neighbour) {
    neighbours.push_back(neighbour);
}
void vertex::push_edge(edge* edge) {
    edges.push_back(edge);
}
edge* vertex::operator[](const int& j){
    for (unsigned k = 0; k < neighbours.size(); k++) {
        if (neighbours[k]->i == j) {
            return edges.at(k);
        }
    }
    throw invalid_argument("These vertices are not linked");
}

vertex* vertex::get_neighbour(const int& j){
    return neighbours.at(j);
}

unsigned vertex::number_neighbour(){
    return neighbours.size();
}

int vertex::get_index(){
    return i;
}

int vertex::cost_of_travel(const int& j) {
    return edges.at(j)->cost();
}
int vertex::cost_of_travel(const int& j, int time) {
    return edges.at(j)->cost(time);
}


edge::edge(const int& size_departure_time, const int& size_arrival_time) {
    assertm(size_departure_time == size_arrival_time, "On an edge size of departure and arrival times must be equals");
    departure_time = vector<int>(size_departure_time);
    arrival_time = vector<int>(size_arrival_time);
    type = "common";
}
edge::edge(int cost) {
    type = "walk";
    transfers_cost = cost;
}
void edge::set_time(const int index, const int& t_departure, const int& t_arrival) {
    if (index >= int(departure_time.size()) || index < 0 || index >= int(arrival_time.size())) throw out_of_range("can't set departure and arrival time because the index is out of range");
    departure_time[index] = t_departure;
    arrival_time[index] = t_arrival;
}

string edge::repr(){
    return "Edge of type : " + type;
}

int edge::cost() { 
    if (type == "walk")return transfers_cost;
    return day;
}

int edge::cost(int time){
    if (type == "common") mission(time);
    return transfers_cost;
}

void edge::mission(int time){
    if (type == "walk") throw invalid_argument("this edge must be a common_edge, not a walking edge");
    int cost;
    int min = 100*day;
    int i_min = -1;
    for (unsigned i = 0; i < departure_time.size(); i++) {
        if (departure_time[i] < time) cost = arrival_time[i] - time + day;
        else cost = arrival_time[i] - time;
        if (cost <= min) {
            min = cost;
            i_min = int(i);
        }
    }
    assertm(i_min != -1,"no minimum found, function does not work properly");
    transfers_cost = min;
    selected_mission = i_min;
}

graph::graph(const int& size_v) {
    v_list = vector<vertex*>(size_v);
    for (int i = 0; i < size_v; i++) {
        v_list[i] = new vertex(i);
    }
}
graph::~graph() {
    cout << "appel detructeur" << endl;
    for (unsigned i = 0; i < v_list.size(); i++) {
        delete v_list[i];
    }
    for (unsigned i = 0; i < e_list.size(); i++) {
        delete e_list[i];
    }
}

void graph::push_common_edge(int departure_index, int arrival_index, const int& size_departure_time, const int& size_arrival_time) {
    if (departure_index >= int(v_list.size()) || departure_index < 0 || arrival_index >=int( v_list.size())|| arrival_index < 0) throw out_of_range("can't push an edge with vertices not in the graph");
    e_list.push_back(new edge(size_departure_time, size_arrival_time));
    v_list[departure_index]->push_neihghbour(v_list[arrival_index]);
    v_list[departure_index]->push_edge(e_list.back());
}
void graph::push_walk_edge(int departure_index, int arrival_index, int cost) {
    if (departure_index >= int(v_list.size()) || departure_index < 0 || arrival_index >= int(v_list.size()) || arrival_index < 0) throw out_of_range("can't push an edge with vertices not in the graph");
    e_list.push_back(new edge(cost));
    v_list[departure_index]->push_neihghbour(v_list[arrival_index]);
    v_list[departure_index]->push_edge(e_list.back());
}
void graph::build_common_edges(py::array_t<int> departure_index, py::array_t<int> arrival_index, py::array_t<int> departure_time, py::array_t<int> arrival_time){    
    auto departure = departure_index.unchecked<1>();
    auto arrival = arrival_index.unchecked<1>();
    auto departure_t = departure_time.unchecked<2>();
    auto arrival_t = arrival_time.unchecked<2>();
    int max_times = departure_t.shape(1);
    assertm((departure.shape(0) == arrival.shape(0) && departure_t.shape(0) == arrival_t.shape(0) && arrival.shape(0) == departure_t.shape(0)),"departure_index ,arrival_index,departure_time,arrival_time must have same shape(0)" );
    for (int i = 0; i < departure.shape(0); i++) {
        push_common_edge(int(departure(i)), int(arrival(i)), max_times, max_times);
        for (int j = 0; j < max_times; j++) {
            e_list.back()->set_time(j, int(departure_t(i, j)), int(arrival_t(i, j)));
        }
    }
}
void graph::build_walk_edges(py::array_t<int> departure_index, py::array_t<int> arrival_index, py::array_t<int> cost) {
    auto departure = departure_index.unchecked<1>();
    auto arrival = arrival_index.unchecked<1>();
    auto cost_w = cost.unchecked<1>();
    assertm((departure.shape(0) == arrival.shape(0) && cost.shape(0) == arrival.shape(0)), "departure_index, arrival_index,cost must have same shape(0)");
    for (int i = 0; i < departure.shape(0); i++) {
        push_walk_edge(int(departure(i)), int(arrival(i)), int(cost_w(i)));
    }
}
vertex* graph::operator[](const int &i){
    try { return v_list.at(i); }
    catch (out_of_range) { throw invalid_argument("This vertex is not in the graph"); } //conversion d'erreur , on veut une ValueError en pyhton = invalid_argument en cpp
}

void graph::initialised(){
    for (unsigned i = 0; i < v_list.size(); i++) {
        v_list[i]->time = 7 * day;
        v_list[i]->visited = false;
    }
}

//alogorithms

void graph::basic_djikstra(const int& start_vertex_index) { // time independent
    //initialisation
    vertex* top = v_list[start_vertex_index];
    vertex* neighbour;
    int cost;
    top->time = 0;
    priority_queue<vertex*, vector<vertex*>, comparetime> PQ; //definition de la file de prioritee avec le foncteur comparetime
    PQ.push(top);
    //boucle principale sur la taille de la file de prioritee
    while (PQ.size() > 0) {
        //on retire le premier element de la file de priorite
        top = PQ.top();
        PQ.pop();
        top->visited = true;

        //on explore ses voisins
        for (unsigned i = 0; i < top->number_neighbour(); i++) {
            neighbour = top->get_neighbour(i);
            if (neighbour->visited == false) { // si ce voisin n'est pas deja visite, (ie est deja sorti de la file de priorite et est donc deja atteint par l'algorithme
                cost = top->time + top->cost_of_travel(i); // calcul du cout pour aller chez ce voisin depuis l'origine si le trajet passe par top . Dj basique independant du temps
                if (cost <= neighbour->time) {
                    neighbour->time = cost;
                    neighbour->predecessor = top;
                }
                PQ.push(neighbour);//on met ce voisin dans la file
            }
        }
    }
}

void graph::time_djikstra(const int& start_vertex_index, int t) { // time dependent
    //initialisation
    vertex* top = v_list[start_vertex_index];
    vertex* neighbour;
    int cost;
    top->time = t;
    priority_queue<vertex*, vector<vertex*>, comparetime> PQ; //definition de la file de prioritee avec le foncteur comparetime
    PQ.push(top);
    //boucle principale sur la taille de la file de prioritee
    while (PQ.size() > 0) {
        //on retire le premier element de la file de priorite
        top = PQ.top();
        PQ.pop();
        top->visited = true;

        //on explore ses voisins
        for (unsigned i = 0; i < top->number_neighbour(); i++) {
            neighbour = top->get_neighbour(i);
            if (neighbour->visited == false) { // si ce voisin n'est pas deja visite, (ie est deja sorti de la file de priorite et est donc deja atteint par l'algorithme
                cost = top->time + top->cost_of_travel(i,top->time); // calcul du cout pour aller chez ce voisin depuis l'origine si le trajet passe par top . dependant du temps
                if (cost <= neighbour->time) {
                    neighbour->time = cost;
                    neighbour->predecessor = top;
                }
                PQ.push(neighbour);//on met ce voisin dans la file
            }
        }
    }
}
void graph::stop_basic_djikstra(const int& start_vertex_index, const int& end_vertex_index){
    vertex* top = v_list[start_vertex_index];
    vertex* neighbour;
    int cost;
    top->time = 0;
    priority_queue<vertex*, vector<vertex*>, comparetime> PQ; //definition de la file de prioritee avec le foncteur comparetime
    PQ.push(top);
    //boucle principale sur la taille de la file de prioritee
    while (PQ.size() && top->get_index()!= end_vertex_index) {
        //on retire le premier element de la file de priorite
        top = PQ.top();
        PQ.pop();
        top->visited = true;

        //on explore ses voisins
        for (unsigned i = 0; i < top->number_neighbour(); i++) {
            neighbour = top->get_neighbour(i);
            if (neighbour->visited == false) { // si ce voisin n'est pas deja visite, (ie est deja sorti de la file de priorite et est donc deja atteint par l'algorithme
                cost = top->time + top->cost_of_travel(i); // calcul du cout pour aller chez ce voisin depuis l'origine si le trajet passe par top . Dj basique independant du temps
                if (cost <= neighbour->time) {
                    neighbour->time = cost;
                    neighbour->predecessor = top;
                }
                PQ.push(neighbour);//on met ce voisin dans la file
            }
        }
    }
}
void graph::stop_time_djikstra(const int& start_vertex_index, const int& end_vertex_index, int t ){
    vertex* top = v_list[start_vertex_index];
    vertex* neighbour;
    int cost;
    top->time = t;
    priority_queue<vertex*, vector<vertex*>, comparetime> PQ; //definition de la file de prioritee avec le foncteur comparetime
    PQ.push(top);   
    //boucle principale sur la taille de la file de prioritee
    while (PQ.size() > 0 && top->get_index() != end_vertex_index) {
        //on retire le premier element de la file de priorite
        top = PQ.top();
        PQ.pop();
        top->visited = true;

        //on explore ses voisins
        for (unsigned i = 0; i < top->number_neighbour(); i++) {
            neighbour = top->get_neighbour(i);
            if (neighbour->visited == false) { // si ce voisin n'est pas deja visite, (ie est deja sorti de la file de priorite et est donc deja atteint par l'algorithme
                cost = top->time + top->cost_of_travel(i, top->time); // calcul du cout pour aller chez ce voisin depuis l'origine si le trajet passe par top . dependant du temps
                if (cost <= neighbour->time) {
                    neighbour->time = cost;
                    neighbour->predecessor = top;
                }
                PQ.push(neighbour);//on met ce voisin dans la file
            }
        }
    }
}
vector<int> graph::path_finder(const int& start_vertex_index, const int& end_vertex_index) {
    vector<int> path;
    stop_basic_djikstra(start_vertex_index, end_vertex_index);
    int index = end_vertex_index;
    while (index != start_vertex_index) {
        if (v_list[index]->visited == false) throw invalid_argument("no path found between start and end");
        path.push_back(index);
        index = v_list[index]->predecessor->get_index();
    }
    path.push_back(index);
    reverse(path.begin(), path.end());
    cout << "arrival_time " << v_list[end_vertex_index]->time << endl;
    initialised();
    return path;

}
vector<int> graph::path_finder_time(const int& start_vertex_index, const int& end_vertex_index, int t){
    vector<int> path;
    stop_time_djikstra(start_vertex_index, end_vertex_index,t);
    int index = end_vertex_index;
    while (index != start_vertex_index) {
        if (v_list[index]->visited == false) throw invalid_argument("no path found between start and end");
        path.push_back(index);
        index = v_list[index]->predecessor->get_index();
    }
    path.push_back(index);
    reverse(path.begin(), path.end());
    cout << "arrival_time " << v_list[end_vertex_index]->time << endl;
    initialised();
    return path;
}