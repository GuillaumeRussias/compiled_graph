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
edge* vertex::operator[](int j){
    for (unsigned k = 0; k < neighbours.size(); k++) {
        if (neighbours[k]->i == j) {
            return edges.at(k);
        }
    }
    throw invalid_argument("These vertices are not linked");
}

vertex* vertex::get_neighbour(int j){
    return neighbours.at(j);
}

unsigned vertex::number_neighbour(){
    return neighbours.size();
}

int vertex::get_index(){
    return i;
}

int vertex::cost_of_travel(int j) {
    return edges.at(j)->cost();
}
int vertex::cost_of_travel(int j, int time) {
    return edges.at(j)->cost(time);
}


edge::edge(int departure_t, int arrival_t) {
    departure_time.push_back(departure_t);
    arrival_time.push_back(arrival_t);
    type = "scheduled";
}

void edge::push_time(int t_departure, int t_arrival) {
    departure_time.push_back(t_departure);
    arrival_time.push_back(t_arrival);
}
edge::edge(int cost) {
    type = "free";
    transfers_cost = cost;
}

int edge::cost() { 
    if (type == "free")return transfers_cost;
    return day;
}

int edge::cost(int time){
    if (type == "scheduled") mission(time);
    return transfers_cost;
}

void edge::mission(int time){
    if (type == "free") throw invalid_argument("this edge must be a scheduled_edge, not a free edge");
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

graph::graph(int size_v) {
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

void graph::push_scheduled_edge(int departure_index, int arrival_index, int departure_time, int arrival_time) {
    if (departure_index >= int(v_list.size()) || departure_index < 0 || arrival_index >=int( v_list.size())|| arrival_index < 0) throw out_of_range("can't push an edge with vertices not in the graph");
    e_list.push_back(new edge(departure_time, arrival_time));
    v_list[departure_index]->push_neihghbour(v_list[arrival_index]);
    v_list[departure_index]->push_edge(e_list.back());
    e_list.back()->key = e_list.size()-1;
}
void graph::push_free_edge(int departure_index, int arrival_index, int cost) {
    if (departure_index >= int(v_list.size()) || departure_index < 0 || arrival_index >= int(v_list.size()) || arrival_index < 0) throw out_of_range("can't push an edge with vertices not in the graph");
    e_list.push_back(new edge(cost));
    v_list[departure_index]->push_neihghbour(v_list[arrival_index]);
    v_list[departure_index]->push_edge(e_list.back());
    e_list.back()->key = e_list.size() - 1;
}
void graph::build_scheduled_edges(py::array_t<int> departure_index, py::array_t<int> arrival_index, py::array_t<int> departure_time, py::array_t<int> arrival_time){ 
    int dep,arr;
    auto departure = departure_index.unchecked<1>();
    auto arrival = arrival_index.unchecked<1>();
    auto departure_t = departure_time.unchecked<1>();
    auto arrival_t = arrival_time.unchecked<1>();
    assertm((departure.shape(0) == arrival.shape(0) && departure_t.shape(0) == arrival_t.shape(0) && arrival.shape(0) == departure_t.shape(0)),"departure_index ,arrival_index,departure_time,arrival_time must have same shape(0)" );
    for (int i = 0; i < departure.shape(0); i++) {
        dep = int(departure(i));
        arr = int(arrival(i));
        try {
            this->operator[](dep)->operator[](arr)->push_time(int (departure_t(i)),int(arrival_t(i))); //on ajoute les horaires departs arrivee si l'edge est deja definie'
        }
        catch (invalid_argument) {
            push_scheduled_edge(dep, arr, int(departure_t(i)), int(arrival_t(i))); //sinon on cree une nouvelle edge
        }
    }
}
void graph::build_free_edges(py::array_t<int> departure_index, py::array_t<int> arrival_index, py::array_t<int> cost) {
    auto departure = departure_index.unchecked<1>();
    auto arrival = arrival_index.unchecked<1>();
    auto cost_w = cost.unchecked<1>();
    assertm((departure.shape(0) == arrival.shape(0) && cost.shape(0) == arrival.shape(0)), "departure_index, arrival_index,cost must have same shape(0)");
    for (int i = 0; i < departure.shape(0); i++) {
        push_free_edge(int(departure(i)), int(arrival(i)), int(cost_w(i)));
    }
}
vertex* graph::operator[](int i){
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

void graph::basic_djikstra(int start_vertex_index) { // time independent
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

void graph::time_djikstra(int start_vertex_index, int t) { // time dependent
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
void graph::stop_basic_djikstra(int start_vertex_index, int end_vertex_index){
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
void graph::stop_time_djikstra(int start_vertex_index, int end_vertex_index, int t ){
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
vector<int> graph::path_finder(int start_vertex_index, int end_vertex_index) {
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
vector<int> graph::path_finder_time(int start_vertex_index, int end_vertex_index, int t){
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
