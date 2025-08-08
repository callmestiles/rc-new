#pragma once

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QPointF>
#include <vector>
#include <unordered_map>
#include <queue>
#include <random>

struct Node {
    QString elementId;
    double x, y;
    double elevation;
    QString type;
    int points;

    Node() : x(0), y(0), elevation(0), points(0) {}
    Node(const QString& id, double x, double y, double elev, const QString& t, int p = 0)
        : elementId(id), x(x), y(y), elevation(elev), type(t), points(p) {}
};

struct Connection {
    QString targetId;
    double cost;
    double distance;

    Connection(const QString& target, double c, double d)
        : targetId(target), cost(c), distance(d) {}
};

struct AStarNode {
    QString nodeId;
    double gCost;  // Distance from start
    double hCost;  // Heuristic distance to goal
    double fCost() const { return gCost + hCost; }
    QString parentId;

    AStarNode(const QString& id, double g, double h, const QString& parent = "")
        : nodeId(id), gCost(g), hCost(h), parentId(parent) {}
};

struct AStarNodeComparator {
    bool operator()(const AStarNode& a, const AStarNode& b) const {
        return a.fCost() > b.fCost();  // Min heap
    }
};

struct Individual {
    std::vector<QString> route;
    double fitness;

    Individual() : fitness(0.0) {}
    Individual(const std::vector<QString>& r) : route(r), fitness(0.0) {}
};

class PathfindingEngine : public QObject
{
    Q_OBJECT

public:
    explicit PathfindingEngine(QObject *parent = nullptr);

    Q_INVOKABLE void setNodes(const QVariantList& nodes);
    Q_INVOKABLE void setConnections(const QVariantMap& connections);
    Q_INVOKABLE QVariantList findPath(const QString& startNodeId, const QString& endNodeId);
    Q_INVOKABLE QVariantList findOptimalCollectionRoute(const QString& startNodeId, const QVariantList& targetNodes);
    Q_INVOKABLE QVariantList findOptimalBallCollectionRoute(const QString& startNodeId,
                                                const QString& releaseNodeId,
                                                int carryCapacity = 8);
    Q_INVOKABLE double calculateRouteValue(const std::vector<QString>& route, const QString& releaseNodeId);
    Q_INVOKABLE void clearPath();

signals:
    void pathCalculated(const QVariantList& path);
    void optimalRouteCalculated(const QVariantList& route);

private:
    std::unordered_map<QString, Node> nodes;
    std::unordered_map<QString, std::vector<Connection>> connections;
    std::mt19937 rng;

    // A* Algorithm methods
    double calculateHeuristic(const QString& nodeId1, const QString& nodeId2);
    std::vector<QString> reconstructPath(const std::unordered_map<QString, QString>& cameFrom,
                                         const QString& current);
    QVariantList convertPathToVariantList(const std::vector<QString>& path);

    // Genetic Algorithm methods
    std::vector<Individual> initializePopulation(const QString& startNodeId,
                                                 const std::vector<QString>& targets,
                                                 int populationSize);
    double calculateRouteFitness(const std::vector<QString>& route);
    double calculateTotalDistance(const std::vector<QString>& route);
    Individual crossover(const Individual& parent1, const Individual& parent2);
    void mutate(Individual& individual, double mutationRate);
    std::vector<Individual> selection(const std::vector<Individual>& population, int selectionSize);

    // Helper methods
    bool nodeExists(const QString& nodeId) const;
    std::vector<QString> getShortestPathBetween(const QString& start, const QString& end);
    std::vector<QString> getCollectibleBallNodes() const;
    int calculateTotalPoints(const std::vector<QString>& nodes) const;

    void generateCombinations(const std::vector<QString>& items,
                              int size,
                              std::vector<std::vector<QString>>& combinations);
    double calculateSimpleRouteValue(const QString& startNodeId,
                                     const std::vector<QString>& ballIds,
                                     const QString& releaseNodeId);
    QVariantList findSimpleCollectionRoute(const QString& startNodeId,
                                           const QVariantList& ballsToCollect);
};
