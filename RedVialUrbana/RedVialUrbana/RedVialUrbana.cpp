#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

// Estructura para guardar los datos de un nodo (latitud, longitud)
struct NodeData {
    double lat;
    double lon;
};

// Estructura para guardar los datos de una arista
struct EdgeData {
    long long from_id;
    long long to_id;
    double distance_m;
    string fclass;
    int oneway;
    string maxspeed;
};

int main() {
    // --------------------------------
    // 1. LEER EL ARCHIVO nodes.csv
    // --------------------------------
    ifstream nodesFile("nodes.csv");
    if (!nodesFile.is_open()) {
        cerr << "Error: No se pudo abrir el archivo nodes.csv" << endl;
        return 1;
    }

    string line;
    // Saltar la primera línea (encabezados)
    getline(nodesFile, line);

    // Mapa para convertir el ID original (long long) a un índice entero (0,1,2,...)
    unordered_map<long long, int> nodeIdToIndex;
    vector<NodeData> nodeData; // Guarda lat/lon para cada índice

    int currentIndex = 0;
    while (getline(nodesFile, line)) {
        stringstream ss(line);
        string idStr, latStr, lonStr;

        getline(ss, idStr, ',');
        getline(ss, latStr, ',');
        getline(ss, lonStr, ',');

        long long originalId = stoll(idStr);
        nodeIdToIndex[originalId] = currentIndex;
        nodeData.push_back({ stod(latStr), stod(lonStr) });
        currentIndex++;
    }
    nodesFile.close();

    int numNodes = currentIndex;
    cout << "Nodos cargados: " << numNodes << endl;

    // --------------------------------
    // 2. LEER EL ARCHIVO edges.csv
    // --------------------------------
    ifstream edgesFile("edges.csv");
    if (!edgesFile.is_open()) {
        cerr << "Error: No se pudo abrir el archivo edges.csv" << endl;
        return 1;
    }

    // Saltar la primera línea (encabezados)
    getline(edgesFile, line);

    vector<EdgeData> edges; // Guardamos temporalmente todas las aristas

    while (getline(edgesFile, line)) {
        stringstream ss(line);
        string osmIdStr, fromStr, toStr, distStr, fclassStr, onewayStr, maxspeedStr;

        getline(ss, osmIdStr, ',');
        getline(ss, fromStr, ',');
        getline(ss, toStr, ',');
        getline(ss, distStr, ',');
        getline(ss, fclassStr, ',');
        getline(ss, onewayStr, ',');
        getline(ss, maxspeedStr, ',');

        long long fromId = stoll(fromStr);
        long long toId = stoll(toStr);
        double distance = stod(distStr);
        int oneway = stoi(onewayStr);

        // Solo guardamos la arista si ambos nodos existen en nuestro mapa
        if (nodeIdToIndex.find(fromId) != nodeIdToIndex.end() &&
            nodeIdToIndex.find(toId) != nodeIdToIndex.end()) {

            edges.push_back({ fromId, toId, distance, fclassStr, oneway, maxspeedStr });
        }
    }
    edgesFile.close();

    int numEdges = edges.size();
    cout << "Aristas cargadas : " << numEdges << endl;

    // --------------------------------
    // 3. CONSTRUIR EL GRAFO (Lista de adyacencia)
    // --------------------------------
    // El grafo es no dirigido para simplificar (aunque tengamos oneway). 
    // Usaremos vector<vector<pair<int, double>>> para la lista de adyacencia.
    vector<vector<pair<int, double>>> graph(numNodes);

    for (const auto& e : edges) {
        int u = nodeIdToIndex[e.from_id];
        int v = nodeIdToIndex[e.to_id];
        double weight = e.distance_m;

        // Agregar arista u -> v
        graph[u].push_back({ v, weight });
        // Si no es oneway, agregar v -> u también
        if (e.oneway == 0) {
            graph[v].push_back({ u, weight });
        }
    }

    cout << "Grafo construido con " << numNodes << " nodos y " << numEdges << " aristas (con bidireccionalidad)." << endl;

    // --------------------------------
    // 4. PRUEBA RÁPIDA: Mostrar los primeros 5 nodos y su grado
    // --------------------------------
    cout << "--- PRUEBA RAPIDA ---" << endl;
    for (int i = 0; i < 5 && i < numNodes; i++) {
        cout << "Nodo " << i << " tiene " << graph[i].size() << " vecinos" << endl;
    }

    cout << "Listo. El grafo esta cargado en memoria." << endl;

    // Pausa para que la consola no se cierre rápido
    cout << "Presiona Enter para salir..." << endl;
    cin.get();

    return 0;
}