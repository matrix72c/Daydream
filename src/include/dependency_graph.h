#include <iostream>
#include <unordered_map>
#include <vector>

struct Task {
    std::string Type;
    unsigned long long Start;
    unsigned long long End;
    unsigned int Thread;
    unsigned int Process;
    unsigned int Stream;
    unsigned int Correlation;
    std::string Note;
    bool operator<(const Task& t) const { return Correlation < t.Correlation; }
    bool operator==(const Task& t) const {
        return Start == t.Start && End == t.End;
    }
};

namespace std {
template <>
struct hash<Task> {
    std::size_t operator()(const Task& t) const {
        return std::hash<unsigned long long>()(t.Start) ^
               std::hash<unsigned long long>()(t.End);
    }
};
}  // namespace std

class Graph {
   public:
    ~Graph() {}
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;
    static Graph& get_instance() {
        static Graph instance;
        return instance;
    }
    void construct_dependency_graph(const char*);
    void replay_log();

   private:
    Graph() {}
    std::unordered_map<Task, std::vector<Task>> dependency_graph;
};