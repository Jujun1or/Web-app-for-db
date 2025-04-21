#pragma once
#include <pqxx/pqxx>
#include <nlohmann/json.hpp>

class Database {
public:
    Database(const std::string& conn_str);
    nlohmann::json getAllBooks();
    nlohmann::json addNewReader(const std::string& name, const std::string& date);
    nlohmann::json externalTopUpFund(int summ, const std::string& date);
    nlohmann::json bookIssue(const int user_id, const int book_id, const std::string& date);
    nlohmann::json bookExtension(int user_id, int book_id, int extensionTime);
    nlohmann::json bookLost(int user_id, int book_id, const std::string& date);
    nlohmann::json bookReturn(int user_id, int book_id, const std::string& date);
    nlohmann::json searchBooks(const std::string& query, bool search_by_author);
private:
    pqxx::connection conn_;
};