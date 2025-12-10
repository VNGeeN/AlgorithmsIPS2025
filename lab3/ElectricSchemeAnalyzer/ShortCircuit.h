#ifndef SHORT_CIRCUIT_H
#define SHORT_CIRCUIT_H

#include <string>
#include <vector>

struct ShortCircuit {
    std::vector<std::string> path;
    double pathResistance;
    double totalResistance;
    
    ShortCircuit() : pathResistance(0.0), totalResistance(0.0) {}
    ShortCircuit(const std::vector<std::string>& p, double pr, double tr)
        : path(p), pathResistance(pr), totalResistance(tr) {}
};

#endif // SHORT_CIRCUIT_H