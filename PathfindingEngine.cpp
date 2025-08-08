#include "PathfindingEngine.h"
#include <QDebug>
#include <algorithm>
#include <cmath>
#include <unordered_set>

PathfindingEngine::PathfindingEngine(QObject *parent)
    : QObject(parent), rng(std::random_device{}())
{}

void PathfindingEngine::setNodes(const QVariantList& nodeList)
{
    nodes.clear();

    for (const QVariant& nodeVariant : nodeList) {
        QVariantMap nodeMap = nodeVariant.toMap();

        QString elementId = nodeMap["elementId"].toString();
        double x = nodeMap["x"].toDouble();
        double y = nodeMap["y"].toDouble();
        double elevation = nodeMap["elevation"].toDouble();
        QString type = nodeMap["type"].toString();
        int points = nodeMap["points"].toInt();

        nodes[elementId] = Node(elementId, x, y, elevation, type, points);
    }

    qDebug() << "Loaded" << nodes.size() << "nodes";
}

void PathfindingEngine::setConnections(const QVariantMap& connectionMap)
{
    connections.clear();

    for (auto it = connectionMap.begin(); it != connectionMap.end(); ++it) {
        QString nodeId = it.key();
        QVariantList connectionList = it.value().toList();

        std::vector<Connection> nodeConnections;

        for (const QVariant& connectionVariant : connectionList) {
            QVariantMap connMap = connectionVariant.toMap();

            QString targetId = connMap["targetId"].toString();
            double cost = connMap["cost"].toDouble();
            double distance = connMap["distance"].toDouble();

            // Find target node ID by index
            // int targetIndex = connMap["targetIndex"].toInt();
            // We need to map the index back to element ID
            // This is a bit tricky since we're working with the original node order
            // For now, we'll assume the connections are properly set up

            nodeConnections.emplace_back(targetId, cost, distance);
        }

        connections[nodeId] = nodeConnections;
    }
    qDebug() << "Loaded connections for" << connections.size() << "nodes";
}

double PathfindingEngine::calculateHeuristic(const QString& nodeId1, const QString& nodeId2)
{
    if (!nodeExists(nodeId1) || !nodeExists(nodeId2)) {
        return std::numeric_limits<double>::max();
    }

    const Node& node1 = nodes[nodeId1];
    const Node& node2 = nodes[nodeId2];

    // Euclidean distance with elevation consideration
    double dx = node1.x - node2.x;
    double dy = node1.y - node2.y;
    double dz = node1.elevation - node2.elevation;

    return std::sqrt(dx * dx + dy * dy + dz * dz * 0.1); // Weight elevation less
}

QVariantList PathfindingEngine::findPath(const QString& startNodeId, const QString& endNodeId)
{
    if (!nodeExists(startNodeId) || !nodeExists(endNodeId)) {
        qDebug() << "Invalid start or end node";
        return QVariantList();
    }

    std::priority_queue<AStarNode, std::vector<AStarNode>, AStarNodeComparator> openSet;
    std::unordered_set<QString> closedSet;
    std::unordered_map<QString, QString> cameFrom;
    std::unordered_map<QString, double> gScore;

    // Initialize
    gScore[startNodeId] = 0.0;
    openSet.emplace(startNodeId, 0.0, calculateHeuristic(startNodeId, endNodeId));

    while (!openSet.empty()) {
        AStarNode current = openSet.top();
        openSet.pop();

        if (current.nodeId == endNodeId) {
            // Path found
            std::vector<QString> path = reconstructPath(cameFrom, endNodeId);
            path.insert(path.begin(), startNodeId);
            return convertPathToVariantList(path);
        }

        if (closedSet.count(current.nodeId)) {
            continue;
        }

        closedSet.insert(current.nodeId);

        // Check all neighbors
        if (connections.count(current.nodeId)) {
            for (const Connection& conn : connections[current.nodeId]) {
                if (closedSet.count(conn.targetId)) {
                    continue;
                }

                double tentativeGScore = gScore[current.nodeId] + conn.cost;

                if (!gScore.count(conn.targetId) || tentativeGScore < gScore[conn.targetId]) {
                    cameFrom[conn.targetId] = current.nodeId;
                    gScore[conn.targetId] = tentativeGScore;

                    double hScore = calculateHeuristic(conn.targetId, endNodeId);
                    openSet.emplace(conn.targetId, tentativeGScore, hScore);
                }
            }
        }
    }

    qDebug() << "No path found between" << startNodeId << "and" << endNodeId;
    return QVariantList();
}

QVariantList PathfindingEngine::findOptimalCollectionRoute(const QString& startNodeId, const QVariantList& targetNodes)
{
    if (!nodeExists(startNodeId) || targetNodes.isEmpty()) {
        return QVariantList();
    }

    std::vector<QString> targets;
    for (const QVariant& target : targetNodes) {
        QString targetId = target.toString();
        if (nodeExists(targetId)) {
            targets.push_back(targetId);
        }
    }

    if (targets.empty()) {
        return QVariantList();
    }

    // Use Genetic Algorithm to solve TSP
    const int populationSize = 100;
    const int generations = 500;
    const double mutationRate = 0.1;
    const double elitePercentage = 0.2;

    std::vector<Individual> population = initializePopulation(startNodeId, targets, populationSize);

    for (int generation = 0; generation < generations; ++generation) {
        // Calculate fitness for all individuals
        for (Individual& individual : population) {
            individual.fitness = calculateRouteFitness(individual.route);
        }

        // Sort by fitness (lower is better)
        std::sort(population.begin(), population.end(),
                  [](const Individual& a, const Individual& b) {
                      return a.fitness < b.fitness;
                  });

        // Create new population
        std::vector<Individual> newPopulation;

        // Keep elite individuals
        int eliteCount = static_cast<int>(populationSize * elitePercentage);
        for (int i = 0; i < eliteCount; ++i) {
            newPopulation.push_back(population[i]);
        }

        // Fill rest with crossover and mutation
        while (newPopulation.size() < populationSize) {
            std::vector<Individual> parents = selection(population, 2);
            Individual offspring = crossover(parents[0], parents[1]);
            mutate(offspring, mutationRate);
            newPopulation.push_back(offspring);
        }

        population = newPopulation;
    }

    // Find best solution
    double bestFitness = std::numeric_limits<double>::max();
    Individual bestIndividual;

    for (const Individual& individual : population) {
        double fitness = calculateRouteFitness(individual.route);
        if (fitness < bestFitness) {
            bestFitness = fitness;
            bestIndividual = individual;
        }
    }

    // Convert to full path with A* between waypoints
    QVariantList fullPath;

    for (size_t i = 0; i < bestIndividual.route.size() - 1; ++i) {
        QVariantList segmentPath = findPath(bestIndividual.route[i], bestIndividual.route[i + 1]);

        // Add segment path (excluding the last node to avoid duplicates)
        for (int j = 0; j < segmentPath.size() - 1; ++j) {
            fullPath.append(segmentPath[j]);
        }
    }

    // Add the final destination
    if (!bestIndividual.route.empty()) {
        fullPath.append(bestIndividual.route.back());
    }

    qDebug() << "Optimal route found with fitness:" << bestFitness;

    return fullPath;
}

void PathfindingEngine::clearPath()
{
    // This method can be used to clear any cached paths if needed
    qDebug() << "Path cleared";
}

std::vector<QString> PathfindingEngine::reconstructPath(const std::unordered_map<QString, QString>& cameFrom, const QString& current)
{
    std::vector<QString> path;
    QString currentNode = current;

    while (cameFrom.count(currentNode)) {
        path.push_back(currentNode);
        currentNode = cameFrom.at(currentNode);
    }

    std::reverse(path.begin(), path.end());
    return path;
}

QVariantList PathfindingEngine::convertPathToVariantList(const std::vector<QString>& path)
{
    QVariantList result;

    for (const QString& nodeId : path) {
        if (nodeExists(nodeId)) {
            QVariantMap nodeData;
            const Node& node = nodes[nodeId];
            nodeData["elementId"] = node.elementId;
            nodeData["x"] = node.x;
            nodeData["y"] = node.y;
            nodeData["elevation"] = node.elevation;
            nodeData["type"] = node.type;
            nodeData["points"] = node.points;

            result.append(nodeData);
        }
    }

    return result;
}

std::vector<Individual> PathfindingEngine::initializePopulation(const QString& startNodeId,
                                                                const std::vector<QString>& targets,
                                                                int populationSize)
{
    std::vector<Individual> population;

    for (int i = 0; i < populationSize; ++i) {
        Individual individual;
        individual.route.push_back(startNodeId);

        // Create random permutation of targets
        std::vector<QString> shuffledTargets = targets;
        std::shuffle(shuffledTargets.begin(), shuffledTargets.end(), rng);

        for (const QString& target : shuffledTargets) {
            individual.route.push_back(target);
        }

        population.push_back(individual);
    }

    return population;
}

double PathfindingEngine::calculateRouteFitness(const std::vector<QString>& route)
{
    if (route.size() < 2) {
        return std::numeric_limits<double>::max();
    }

    double totalDistance = 0.0;

    for (size_t i = 0; i < route.size() - 1; ++i) {
        // For simplicity, use heuristic distance
        // In practice, you might want to use actual A* distance
        double distance = calculateHeuristic(route[i], route[i + 1]);
        totalDistance += distance;
    }

    return totalDistance;
}

double PathfindingEngine::calculateTotalDistance(const std::vector<QString>& route)
{
    return calculateRouteFitness(route);
}

Individual PathfindingEngine::crossover(const Individual& parent1, const Individual& parent2)
{
    Individual offspring;

    if (parent1.route.size() != parent2.route.size()) {
        return parent1; // Fallback
    }

    offspring.route.push_back(parent1.route[0]); // Start node

    // Order crossover (OX)
    int size = parent1.route.size() - 1; // Exclude start node
    int start = 1 + (rng() % (size - 1));
    int end = start + (rng() % (size - start));

    std::unordered_set<QString> included;

    // Copy segment from parent1
    for (int i = start; i <= end; ++i) {
        offspring.route.push_back(parent1.route[i]);
        included.insert(parent1.route[i]);
    }

    // Fill remaining from parent2
    for (int i = 1; i < parent2.route.size(); ++i) {
        if (included.find(parent2.route[i]) == included.end()) {
            offspring.route.push_back(parent2.route[i]);
        }
    }

    return offspring;
}

void PathfindingEngine::mutate(Individual& individual, double mutationRate)
{
    if (individual.route.size() < 3) return; // Need at least start + 2 targets

    if (std::uniform_real_distribution<double>(0.0, 1.0)(rng) < mutationRate) {
        // Swap two random positions (excluding start)
        int pos1 = 1 + (rng() % (individual.route.size() - 1));
        int pos2 = 1 + (rng() % (individual.route.size() - 1));

        std::swap(individual.route[pos1], individual.route[pos2]);
    }
}

std::vector<Individual> PathfindingEngine::selection(const std::vector<Individual>& population, int selectionSize)
{
    std::vector<Individual> selected;

    // Tournament selection
    for (int i = 0; i < selectionSize; ++i) {
        int tournamentSize = 3;
        Individual best = population[rng() % population.size()];

        for (int j = 1; j < tournamentSize; ++j) {
            Individual candidate = population[rng() % population.size()];
            if (candidate.fitness < best.fitness) {
                best = candidate;
            }
        }

        selected.push_back(best);
    }

    return selected;
}

bool PathfindingEngine::nodeExists(const QString& nodeId) const
{
    return nodes.count(nodeId) > 0;
}

std::vector<QString> PathfindingEngine::getShortestPathBetween(const QString& start, const QString& end)
{
    QVariantList path = findPath(start, end);
    std::vector<QString> result;

    for (const QVariant& nodeVariant : path) {
        QVariantMap nodeMap = nodeVariant.toMap();
        result.push_back(nodeMap["elementId"].toString());
    }

    return result;
}

// Replace the problematic section in findOptimalBallCollectionRoute method

QVariantList PathfindingEngine::findOptimalBallCollectionRoute(const QString& startNodeId,
                                                               const QString& releaseNodeId,
                                                               int carryCapacity)
{
    if (!nodeExists(startNodeId) || !nodeExists(releaseNodeId)) {
        qDebug() << "Invalid start or release node";
        return QVariantList();
    }

    std::vector<QString> allBalls = getCollectibleBallNodes();
    if (allBalls.empty()) {
        qDebug() << "No collectible balls found";
        return QVariantList();
    }

    qDebug() << "Found" << allBalls.size() << "balls, capacity:" << carryCapacity;

    // Limit the search space to prevent combinatorial explosion
    int maxBallsToConsider = std::min(carryCapacity, std::min(8, (int)allBalls.size()));

    // If we have too many balls, select the closest ones to start with
    if (allBalls.size() > maxBallsToConsider) {
        std::vector<std::pair<double, QString>> ballDistances;
        for (const QString& ballId : allBalls) {
            double distance = calculateHeuristic(startNodeId, ballId);
            ballDistances.emplace_back(distance, ballId);
        }

        // Sort by distance and take the closest ones
        std::sort(ballDistances.begin(), ballDistances.end());
        allBalls.clear();
        for (int i = 0; i < maxBallsToConsider; ++i) {
            allBalls.push_back(ballDistances[i].second);
        }
    }

    qDebug() << "Considering" << allBalls.size() << "balls for optimization";

    // Generate combinations more efficiently
    std::vector<std::vector<QString>> ballCombinations;

    // Use iterative approach for smaller combinations to avoid stack overflow
    for (int size = 1; size <= std::min(carryCapacity, (int)allBalls.size()); ++size) {
        if (size <= 6) { // Only generate combinations up to reasonable size
            generateCombinations(allBalls, size, ballCombinations);
        }
    }

    qDebug() << "Generated" << ballCombinations.size() << "combinations";

    // Find the best combination
    double bestValue = 0.0;
    std::vector<QString> bestCombination;

    for (const auto& combination : ballCombinations) {
        try {
            // Calculate route value more efficiently
            double routeValue = calculateSimpleRouteValue(startNodeId, combination, releaseNodeId);

            if (routeValue > bestValue) {
                bestValue = routeValue;
                bestCombination = combination;
            }
        } catch (const std::exception& e) {
            qDebug() << "Error calculating route value:" << e.what();
            continue;
        }
    }

    if (bestCombination.empty()) {
        qDebug() << "No valid combination found";
        return QVariantList();
    }

    qDebug() << "Best combination has" << bestCombination.size() << "balls with value:" << bestValue;

    // Generate final optimized route
    QVariantList ballsToCollect;
    for (const QString& ballId : bestCombination) {
        ballsToCollect.append(QVariant(ballId));
    }

    QVariantList optimizedPath;

    try {
        // Use a simpler pathfinding approach for the final route
        optimizedPath = findSimpleCollectionRoute(startNodeId, ballsToCollect);

        // Add release area to the end
        if (!optimizedPath.empty() && nodeExists(releaseNodeId)) {
            QVariantMap releaseData;
            const Node& releaseNode = nodes[releaseNodeId];
            releaseData["elementId"] = releaseNode.elementId;
            releaseData["x"] = releaseNode.x;
            releaseData["y"] = releaseNode.y;
            releaseData["elevation"] = releaseNode.elevation;
            releaseData["type"] = releaseNode.type;
            releaseData["points"] = releaseNode.points;

            optimizedPath.append(releaseData);
        }
    } catch (const std::exception& e) {
        qDebug() << "Error generating final route:" << e.what();
        return QVariantList();
    }

    qDebug() << "Final route generated with" << optimizedPath.size() << "nodes";
    return optimizedPath;
}

// Helper method to generate combinations more safely
void PathfindingEngine::generateCombinations(const std::vector<QString>& items,
                                             int size,
                                             std::vector<std::vector<QString>>& combinations)
{
    if (size > items.size() || size <= 0) {
        return;
    }

    std::vector<bool> selector(items.size(), false);
    std::fill(selector.begin(), selector.begin() + size, true);

    int count = 0;
    const int maxCombinations = 1000; // Limit to prevent memory issues

    do {
        if (count >= maxCombinations) {
            qDebug() << "Reached maximum combinations limit";
            break;
        }

        std::vector<QString> combination;
        for (size_t i = 0; i < items.size(); ++i) {
            if (selector[i]) {
                combination.push_back(items[i]);
            }
        }
        combinations.push_back(combination);
        count++;
    } while (std::prev_permutation(selector.begin(), selector.end()));
}

double PathfindingEngine::calculateSimpleRouteValue(const QString& startNodeId,
                                                    const std::vector<QString>& ballIds,
                                                    const QString& releaseNodeId)
{
    if (ballIds.empty()) {
        return 0.0;
    }

    // Calculate total points
    int totalPoints = 0;
    for (const QString& ballId : ballIds) {
        if (nodeExists(ballId)) {
            totalPoints += nodes[ballId].points;
        }
    }

    // Estimate total distance (simplified)
    double totalDistance = 0.0;

    // Distance from start to first ball (use closest ball as approximation)
    double minDistanceToStart = std::numeric_limits<double>::max();
    for (const QString& ballId : ballIds) {
        double distance = calculateHeuristic(startNodeId, ballId);
        minDistanceToStart = std::min(minDistanceToStart, distance);
    }
    totalDistance += minDistanceToStart;

    // Approximate distance between balls (use average)
    double avgBallDistance = 0.0;
    if (ballIds.size() > 1) {
        for (size_t i = 0; i < ballIds.size() - 1; ++i) {
            avgBallDistance += calculateHeuristic(ballIds[i], ballIds[i + 1]);
        }
        avgBallDistance /= (ballIds.size() - 1);
        totalDistance += avgBallDistance * (ballIds.size() - 1);
    }

    // Distance from last ball to release (use closest ball as approximation)
    double minDistanceToRelease = std::numeric_limits<double>::max();
    for (const QString& ballId : ballIds) {
        double distance = calculateHeuristic(ballId, releaseNodeId);
        minDistanceToRelease = std::min(minDistanceToRelease, distance);
    }
    totalDistance += minDistanceToRelease;

    if (totalDistance <= 0.0) {
        return 0.0;
    }

    return totalPoints / totalDistance;
}

QVariantList PathfindingEngine::findSimpleCollectionRoute(const QString& startNodeId,
                                                          const QVariantList& ballsToCollect)
{
    if (ballsToCollect.empty()) {
        return QVariantList();
    }

    // Convert to vector for easier manipulation
    std::vector<QString> balls;
    for (const QVariant& ballVariant : ballsToCollect) {
        balls.push_back(ballVariant.toString());
    }

    // Use nearest neighbor heuristic instead of full genetic algorithm
    std::vector<QString> route;
    route.push_back(startNodeId);

    std::vector<QString> remaining = balls;
    QString current = startNodeId;

    while (!remaining.empty()) {
        // Find closest remaining ball
        double minDistance = std::numeric_limits<double>::max();
        int bestIndex = 0;

        for (size_t i = 0; i < remaining.size(); ++i) {
            double distance = calculateHeuristic(current, remaining[i]);
            if (distance < minDistance) {
                minDistance = distance;
                bestIndex = i;
            }
        }

        // Add closest ball to route
        route.push_back(remaining[bestIndex]);
        current = remaining[bestIndex];
        remaining.erase(remaining.begin() + bestIndex);
    }

    // Convert back to QVariantList
    return convertPathToVariantList(route);
}

std::vector<QString> PathfindingEngine::getCollectibleBallNodes() const
{
    std::vector<QString> balls;

    for (const auto& nodePair : nodes) {
        const Node& node = nodePair.second;
        if (node.type == "green_ball" ||
            node.type == "black_striped_ball" ||
            node.type == "star_ball" ||
            node.type == "comm_tow") {
            balls.push_back(node.elementId);
        }
    }

    return balls;
}

int PathfindingEngine::calculateTotalPoints(const std::vector<QString>& nodes) const
{
    int totalPoints = 0;
    for (const QString& nodeId : nodes) {
        if (nodeExists(nodeId)) {
            totalPoints += this->nodes.at(nodeId).points;
        }
    }
    return totalPoints;
}

double PathfindingEngine::calculateRouteValue(const std::vector<QString>& route, const QString& releaseNodeId)
{
    if (route.size() < 2) {
        return 0.0;
    }

    // Calculate total points
    int totalPoints = 0;
    for (const QString& nodeId : route) {
        if (nodeExists(nodeId) && nodeId != releaseNodeId) {
            totalPoints += nodes[nodeId].points;
        }
    }

    // Calculate total distance
    double totalDistance = 0.0;
    for (size_t i = 0; i < route.size() - 1; ++i) {
        totalDistance += calculateHeuristic(route[i], route[i + 1]);
    }

    if (totalDistance == 0.0) {
        return 0.0;
    }

    return totalPoints / totalDistance; // Points per distance unit
}
