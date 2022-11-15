#include "dependency_graph.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

Task csv_parser(std::string line) {
    Task task;
    std::string::size_type pos1, pos2;
    pos1 = line.find("\"");
    pos2 = line.find("\"", pos1 + 1);
    task.Type = line.substr(pos1 + 1, pos2 - pos1 - 1);
    pos2++;
    pos1 = line.find(",", pos2 + 1);
    task.Start = std::stoull(line.substr(pos2 + 1, pos1 - pos2 - 1));
    pos2 = line.find(",", pos1 + 1);
    task.End = std::stoull(line.substr(pos1 + 1, pos2 - pos1 - 1));
    pos1 = line.find(",", pos2 + 1);
    task.Thread = std::stoul(line.substr(pos2 + 1, pos1 - pos2 - 1));
    pos2 = line.find(",", pos1 + 1);
    task.Process = std::stoul(line.substr(pos1 + 1, pos2 - pos1 - 1));
    pos1 = line.find(",", pos2 + 1);
    task.Stream = std::stoul(line.substr(pos2 + 1, pos1 - pos2 - 1));
    pos2 = line.find(",", pos1 + 1);
    task.Correlation = std::stoul(line.substr(pos1 + 1, pos2 - pos1 - 1));
    pos1 = line.find("\"", pos2 + 1);
    pos2 = line.find("\"", pos1 + 1);
    task.Note = line.substr(pos1 + 1, pos2 - pos1 - 1);
    return task;
}

void Graph::construct_dependency_graph(const char* filename) {
    std::vector<Task> tasks;
    std::ifstream fin(filename);
    std::string line;
    std::getline(fin, line);
    while (std::getline(fin, line)) {
        auto task = csv_parser(line);
        tasks.emplace_back(task);
    }
    std::sort(tasks.begin(), tasks.end());
    std::unordered_map<unsigned int, Task> thread_map;
    std::unordered_map<unsigned int, Task> stream_map;
    std::unordered_map<unsigned int, Task> correlation_map;
    for (auto task : tasks) {
        if (task.Type == "DRIVER" || task.Type == "RUNTIME") {
            // CPU sequential task
            if (thread_map.find(task.Thread) == thread_map.end()) {
                thread_map[task.Thread] = task;
                dependency_graph[task] = std::vector<Task>();
            } else {
                dependency_graph[thread_map[task.Thread]].emplace_back(task);
            }
            // CPU correlation task
            if (correlation_map.find(task.Correlation) !=
                correlation_map.end()) {
                auto gpu_task = correlation_map[task.Correlation];
                dependency_graph[task].emplace_back(gpu_task);
                if (gpu_task.Type == "SYNC") {
                    // CPU task don't depend on MEMSET, MEMCPY, KERNEL tasks
                    dependency_graph[gpu_task].emplace_back(task);
                }
            } else {
                correlation_map[task.Correlation] = task;
            }
        } else {
            // GPU sequential task
            if (stream_map.find(task.Stream) == stream_map.end()) {
                stream_map[task.Stream] = task;
                dependency_graph[task] = std::vector<Task>();
            } else {
                dependency_graph[stream_map[task.Stream]].emplace_back(task);
            }
            // GPU correlation task
            if (correlation_map.find(task.Correlation) !=
                correlation_map.end()) {
                auto cpu_task = correlation_map[task.Correlation];
                dependency_graph[cpu_task].emplace_back(task);
                if (task.Type == "SYNC") {
                    // CPU task don't depend on MEMSET, MEMCPY, KERNEL tasks
                    dependency_graph[task].emplace_back(cpu_task);
                }
            } else {
                correlation_map[task.Correlation] = task;
            }
        }
    }
    fin.close();
}

void Graph::replay_log() {
    std::ofstream fout("replay.log", std::ios::out | std::ios::trunc);
    for (auto it = dependency_graph.begin(); it != dependency_graph.end();
         ++it) {
        fout << "[(" << it->first.Type << "," << it->first.Start << ","
                  << it->first.End << ")"
                  << " ";
        for (auto task : it->second) {
            fout << "-> (" << task.Type << "," << task.Start << ","
                      << task.End << ")";
        }
        fout << "]" << std::endl;
    }
}