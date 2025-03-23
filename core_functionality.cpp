#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <cmath>
#include <random>
#include <algorithm>


using namespace std;


class Map {
private:
    vector<vector<pair<int, double>>> graph; 
    int numNodes;
public:
    Map(int nodes) : numNodes(nodes) {
        graph.resize(nodes);
    }
    void addEdge(int from, int to, double weight) {
        graph[from].push_back({to, weight});
        graph[to].push_back({from, weight}); 
    }
    void updateTraffic() {
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<> dis(0.8, 1.2); 
        for (int i = 0; i < numNodes; i++) {
            for (auto& edge : graph[i]) {
                edge.second *= dis(gen); 
            }
        }
    }
    vector<int> findOptimalRoute(int start, int end) {
        vector<double> dist(numNodes, numeric_limits<double>::infinity());
        vector<int> prev(numNodes, -1);
        priority_queue<pair<double, int>, vector<pair<double, int>>, greater<>> pq;
        dist[start] = 0;
        pq.push({0, start});
        while (!pq.empty()) {
            int u = pq.top().second;
            pq.pop();
            if (u == end) break;
            for (auto [v, weight] : graph[u]) {
                double alt = dist[u] + weight;
                if (alt < dist[v]) {
                    dist[v] = alt;
                    prev[v] = u;
                    pq.push({alt, v});
                }
            }
        }
        vector<int> path;
        for (int at = end; at != -1; at = prev[at]) {
            path.push_back(at);
        }
        reverse(path.begin(), path.end());
        return path;
    }
};



class Agent {
public:
    int id;
    int location; 
    vector<int> assignedOrders; 
    bool available;
    Agent(int _id, int _loc) : id(_id), location(_loc), available(true) {}
};


class Order {
public:
    int id;
    int location; 
    int timeSlot; 
    Order(int _id, int _loc, int _slot) : id(_id), location(_loc), timeSlot(_slot) {}
};


class Forecast {
private:
    double alpha = 0.5;
    double forecast = 10.0; 
public:
    void updateForecast(double actualDemand) {
        forecast = alpha * actualDemand + (1 - alpha) * forecast;
    }
    double getForecast() const {
        return forecast;
    }
};


class Pricing {
private:
    double basePrice = 100.0; 
    double k = 0.5;
public:
    double getPrice(double demand, double capacity) {
        return basePrice * (1 + k * (demand / capacity));
    }
};


int main() {
    Map cityMap(25);
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int node = i * 5 + j;
            if (j < 4) cityMap.addEdge(node, node + 1, 10.0); 
            if (i < 4) cityMap.addEdge(node, node + 5, 10.0); 
        }
    }
    vector<Agent> agents;
    for (int i = 0; i < 5; i++) {
        agents.emplace_back(i, 0); 
    }
    Forecast demandForecast;
    Pricing deliveryPricing;
    vector<Order> orders;
    int orderId = 0;
    double totalDeliveryTime = 0.0;
    double totalCost = 0.0;
    int deliveryCount = 0;


    for (int hour = 0; hour < 24; hour++) {
        cout << "\nHour " << hour << ":\n";
        cityMap.updateTraffic();
        double forecastedDemand = demandForecast.getForecast();
        cout << "Forecasted Demand: " << forecastedDemand << " orders\n";


        double capacity = agents.size();
        double price = deliveryPricing.getPrice(forecastedDemand, capacity);
        cout << "Delivery Slot Price: ₹" << price << "\n"; 
        random_device rd;
        mt19937 gen(rd());
        poisson_distribution<> poisson(forecastedDemand);
        int numOrders = poisson(gen);
        for (int i = 0; i < numOrders; i++) {
            uniform_int_distribution<> locDis(0, 24);
            orders.emplace_back(orderId++, locDis(gen), hour);
        }
        for (auto& order : orders) {
            if (order.timeSlot != hour) continue; 
            for (auto& agent : agents) {
                if (agent.available) {
                    agent.assignedOrders.push_back(order.id);
                    agent.available = false;
                    vector<int> route = cityMap.findOptimalRoute(agent.location, order.location);
                    double routeTime = route.size() * 10.0 / 60.0; 
                    totalDeliveryTime += routeTime;
                    totalCost += 100.0; 
                    deliveryCount++;
                    agent.location = order.location; 
                    agent.available = true; 
                    break;
                }
            }
        }
        demandForecast.updateForecast(numOrders);
    }
    double avgDeliveryTime = totalDeliveryTime / deliveryCount;
    double avgCost = totalCost / deliveryCount;
    cout << "\nSimulation Results:\n";
    cout << "Average Delivery Time: " << avgDeliveryTime << " hours\n";
    cout << "Average Logistics Cost: ₹" << avgCost << "\n"; 
    return 0;
}
