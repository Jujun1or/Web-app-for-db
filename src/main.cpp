#include "db/Database.hpp"
#include "server/Server.hpp"
#include <iostream>

int main() {
    try {
        Database db("postgresql://postgres:roman2392@localhost/library_db");

        // Запуск бенчмарков
        // std::vector<int> record_counts = {1000, 10000, 100000};
        // db.benchmarkOperations(record_counts);

        Server server(db);
        server.start();
    }
    catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}