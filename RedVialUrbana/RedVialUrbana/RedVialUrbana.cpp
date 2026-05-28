#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <utility>

using namespace std;

struct NodeData {
    double lat;
    double lon;
};

struct EdgeData {
    long long from_id;
    long long to_id;
    double distance_m;
    string fclass;
    int oneway;
    string maxspeed;
};

int contarAlcanzablesEn5km(int startNode, const vector<vector<pair<int, double>>>& graph) {
    int n = graph.size();
    vector<bool> visited(n, false);
    vector<double> dist(n, 0.0);
    queue<int> q;

    visited[startNode] = true;
    q.push(startNode);
    int count = 1;

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (const auto& edge : graph[u]) {
            int v = edge.first;
            double weight = edge.second;

            if (!visited[v] && dist[u] + weight <= 5000.0) {
                visited[v] = true;
                dist[v] = dist[u] + weight;
                q.push(v);
                count++;
            }
        }
    }

    return count;
}

int main() {
    ifstream nodesFile("nodes.csv");
    if (!nodesFile.is_open()) {
        cerr << "Error: No se pudo abrir el archivo nodes.csv" << endl;
        return 1;
    }

    string line;
    getline(nodesFile, line);

    unordered_map<long long, int> nodeIdToIndex;
    vector<NodeData> nodeData;

    int currentIndex = 0;
    while (getline(nodesFile, line)) {
        stringstream ss(line);
        string idStr, latStr, lonStr;

        getline(ss, idStr, ',');
        getline(ss, latStr, ',');
        getline(ss, lonStr, ',');

        if (idStr.empty() || latStr.empty() || lonStr.empty()) continue;

        try {
            long long originalId = stoll(idStr);
            nodeIdToIndex[originalId] = currentIndex;
            nodeData.push_back({ stod(latStr), stod(lonStr) });
            currentIndex++;
        }
        catch (...) {
            continue;
        }
    }
    nodesFile.close();

    int numNodes = currentIndex;
    cout << "Nodos cargados: " << numNodes << endl;

    ifstream edgesFile("edges.csv");
    if (!edgesFile.is_open()) {
        cerr << "Error: No se pudo abrir el archivo edges.csv" << endl;
        return 1;
    }

    getline(edgesFile, line);
    vector<EdgeData> edges;

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

        if (fromStr.empty() || toStr.empty() || distStr.empty()) continue;

        try {
            long long fromId = stoll(fromStr);
            long long toId = stoll(toStr);
            double distance = stod(distStr);
            int oneway = stoi(onewayStr);

            if (distance <= 0) continue;
            if (oneway != 0 && oneway != 1) continue;
            if (fclassStr.empty()) continue;

            if (nodeIdToIndex.find(fromId) != nodeIdToIndex.end() &&
                nodeIdToIndex.find(toId) != nodeIdToIndex.end()) {
                edges.push_back({ fromId, toId, distance, fclassStr, oneway, maxspeedStr });
            }
        }
        catch (...) {
            continue;
        }
    }
    edgesFile.close();

    int numEdges = edges.size();
    cout << "Aristas cargadas: " << numEdges << endl;

    vector<vector<pair<int, double>>> graph(numNodes);

    for (const auto& e : edges) {
        int u = nodeIdToIndex[e.from_id];
        int v = nodeIdToIndex[e.to_id];
        double weight = e.distance_m;

        graph[u].push_back({ v, weight });
        if (e.oneway == 0) {
            graph[v].push_back({ u, weight });
        }
    }

    cout << "Grafo construido con " << numNodes << " nodos y " << numEdges << " aristas (con bidireccionalidad)." << endl;

    cout << "--- PRUEBA RAPIDA ---" << endl;
    for (int i = 0; i < 5 && i < numNodes; i++) {
        cout << "Nodo " << i << " tiene " << graph[i].size() << " vecinos" << endl;
    }

    cout << "--- ALCANCE VEHICULAR (5 km) ---" << endl;
    int start = 0;
    int reachable = contarAlcanzablesEn5km(start, graph);
    cout << "Desde el nodo " << start << " se pueden alcanzar " << reachable << " nodos (incluyendo el propio) en máximo 5 km." << endl;

    cout << "Listo. El grafo esta cargado en memoria." << endl;
    cout << "Presiona Enter para salir..." << endl;
    cin.get();

    return 0;
}