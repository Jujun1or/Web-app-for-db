#pragma once
#include <pqxx/pqxx>
#include <nlohmann/json.hpp>

class Database {
public:
    Database(const std::string& conn_str);
    nlohmann::json getAllBooks();
    nlohmann::json getOverdueLoans();

    nlohmann::json addNewReader(const std::string& name, const std::string& date);
    nlohmann::json externalTopUpFund(int summ, const std::string& date);
    nlohmann::json bookIssue(const std::string& user_name, int book_id, const std::string& date);
    nlohmann::json bookExtension(const std::string& user_name, int book_id, int extensionTime);
    nlohmann::json bookLost(const std::string& user_name, int book_id, const std::string& date);
    nlohmann::json bookReturn(const std::string& user_name, int book_id, const std::string& date);
    nlohmann::json searchBooks(const std::string& query, bool search_by_author);
    nlohmann::json generateComplaintLetter(int bu_id);

private:
    pqxx::connection conn_;
};