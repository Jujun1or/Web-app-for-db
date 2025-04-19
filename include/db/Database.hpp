#pragma once
#include <pqxx/pqxx>
#include <nlohmann/json.hpp>

class Database {
public:
    Database(const std::string& conn_str);
    nlohmann::json getAllBooks();
    nlohmann::json addNewReader(const std::string& name, const std::string& date);
private:
    pqxx::connection conn_;
};