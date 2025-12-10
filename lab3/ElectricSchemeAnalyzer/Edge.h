#ifndef EDGE_H
#define EDGE_H

#include <string>

struct Edge {
    std::string from;
    std::string to;
    double resistance;
    bool isSource;
    
    Edge(const std::string& f, const std::string& t, double r, bool src = false);
};

#endif // EDGE_H