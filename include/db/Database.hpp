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
    nlohmann::json bookIssueByUserId(int user_id, int book_id, const std::string& date);
    nlohmann::json bookExtensionByUserId(int user_id, int book_id, int extension_days);
    nlohmann::json bookLostByUserId(int user_id, int book_id, const std::string& date);
    nlohmann::json bookReturnByUserId(int user_id, int book_id, const std::string& date);
    nlohmann::json searchBooks(const std::string& query, bool search_by_author);
    nlohmann::json generateComplaintLetter(int bu_id);

    void processFinePayment(int bu_id);
    nlohmann::json getPopularBooks(const std::string& start_date, const std::string& end_date);
    nlohmann::json getReadersStats(int year);
    nlohmann::json getFinancialReport(int year);
    nlohmann::json deactivateInactiveUsers();

    nlohmann::json getAllUsers();
    nlohmann::json getCurrentBooks(int user_id);
    nlohmann::json getUserIdByName(const std::string& name);
private:
    pqxx::connection conn_;
};