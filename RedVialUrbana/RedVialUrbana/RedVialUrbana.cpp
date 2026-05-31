#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <utility>
#include <algorithm>

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

void encontrarComponentesConexas(const vector<vector<pair<int, double>>>& graph) {
    int n = graph.size();
    vector<bool> visited(n, false);

    int totalComponentes = 0;
    int componenteGigante = 0;

    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            totalComponentes++;
            int size = 0;
            queue<int> q;
            q.push(i);
            visited[i] = true;

            while (!q.empty()) {
                int u = q.front();
                q.pop();
                size++;

                for (const auto& edge : graph[u]) {
                    int v = edge.first;
                    if (!visited[v]) {
                        visited[v] = true;
                        q.push(v);
                    }
                }
            }

            if (size > componenteGigante) {
                componenteGigante = size;
            }
        }
    }

    cout << "--- COMPONENTES CONEXAS ---" << endl;
    cout << "Total de islas: " << totalComponentes << endl;
    cout << "Tamaño de la red principal: " << componenteGigante << " nodos" << endl;
}

struct DSU {
    vector<int> parent, rank;
    DSU(int n) {
        parent.resize(n);
        rank.resize(n, 0);
        for (int i = 0; i < n; i++) parent[i] = i;
    }
    int find(int x) {
        if (parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    }
    bool unite(int x, int y) {
        int rx = find(x), ry = find(y);
        if (rx == ry) return false;
        if (rank[rx] < rank[ry]) parent[rx] = ry;
        else if (rank[rx] > rank[ry]) parent[ry] = rx;
        else { parent[ry] = rx; rank[rx]++; }
        return true;
    }
};

struct EdgeKruskal {
    int u, v;
    double w;
    bool operator<(const EdgeKruskal& other) const { return w < other.w; }
};

void construirMST(const vector<vector<pair<int, double>>>& graph, const vector<int>& nodosComponente) {
    unordered_map<int, int> globalToLocal;
    for (int i = 0; i < nodosComponente.size(); i++) {
        globalToLocal[nodosComponente[i]] = i;
    }

    int n = nodosComponente.size();
    vector<EdgeKruskal> edges;

    for (int uGlobal : nodosComponente) {
        int uLocal = globalToLocal[uGlobal];
        for (const auto& edge : graph[uGlobal]) {
            int vGlobal = edge.first;
            double weight = edge.second;
            if (globalToLocal.find(vGlobal) != globalToLocal.end()) {
                int vLocal = globalToLocal[vGlobal];
                if (uLocal < vLocal) {
                    edges.push_back({ uLocal, vLocal, weight });
                }
            }
        }
    }

    sort(edges.begin(), edges.end());

    DSU dsu(n);
    double totalWeight = 0;
    int edgesTaken = 0;

    cout << "Construyendo MST sobre la componente gigante" << endl;
    for (const auto& e : edges) {
        if (dsu.unite(e.u, e.v)) {
            totalWeight += e.w;
            edgesTaken++;
            if (edgesTaken == n - 1) break;
        }
    }

    cout << "Total de aristas en el MST: " << edgesTaken << endl;
    cout << "Peso total del MST: " << totalWeight / 1000.0 << " km" << endl;
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
            int oneway = 0;
            if (onewayStr == "T" || onewayStr == "t" || onewayStr == "true" || onewayStr == "yes" || onewayStr == "1") {
                oneway = 1;
            }

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
        graph[v].push_back({ u, weight });
    }

    cout << "Grafo construido con " << numNodes << " nodos y " << numEdges << " aristas" << endl;

    cout << "--- PRUEBA RAPIDA ---" << endl;
    for (int i = 0; i < 5 && i < numNodes; i++) {
        cout << "Nodo " << i << " tiene " << graph[i].size() << " vecinos" << endl;
    }       

    encontrarComponentesConexas(graph);

    vector<int> nodosComponenteGigante;
    {
        int n = graph.size();
        vector<bool> visited(n, false);
        int maxSize = 0;
        int maxStart = 0;

        for (int i = 0; i < n; i++) {
            if (!visited[i]) {
                int size = 0;
                queue<int> q;
                q.push(i);
                visited[i] = true;
                while (!q.empty()) {
                    int u = q.front();
                    q.pop();
                    size++;
                    for (const auto& edge : graph[u]) {
                        int v = edge.first;
                        if (!visited[v]) {
                            visited[v] = true;
                            q.push(v);
                        }
                    }
                }
                if (size > maxSize) {
                    maxSize = size;
                    maxStart = i;
                }
            }
        }

        visited.assign(n, false);
        queue<int> q;
        q.push(maxStart);
        visited[maxStart] = true;
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            nodosComponenteGigante.push_back(u);
            for (const auto& edge : graph[u]) {
                int v = edge.first;
                if (!visited[v]) {
                    visited[v] = true;
                    q.push(v);
                }
            }
        }
        cout << "Nodos en la componente gigante: " << nodosComponenteGigante.size() << endl;
    }

    construirMST(graph, nodosComponenteGigante);

    cout << "--- ALCANCE VEHICULAR (5 km) ---" << endl;
    int start = 0;
    int reachable = contarAlcanzablesEn5km(start, graph);
    cout << "Desde el nodo " << start << " se pueden alcanzar " << reachable << " nodos en máximo 5 km." << endl;

    cout << "El grafo esta cargado en memoria" << endl;
    cin.get();

    return 0;
}