#include "db/Database.hpp"
#include <stdexcept>

#define EXTERNAL_PAY 99999 //id книги для внешного пополнения (грязь)

enum Operation_type{
    EXTERNAL = 1,
    LOST,
    FINE
};

Database::Database(const std::string& conn_str) : conn_(conn_str) {
    if (!conn_.is_open()) {
        throw std::runtime_error("Database connection failed");
    }
}

nlohmann::json Database::getAllBooks() {
    nlohmann::json result = nlohmann::json::array();
    pqxx::work txn(conn_);
    
    try {
        pqxx::result res = txn.exec("SELECT book_id, title, price FROM Books");
        for (const auto& row : res) {
            nlohmann::json book;
            book["id"] = row[0].as<int>();
            book["title"] = row[1].as<std::string>();
            book["price"] = row[2].as<int>();
            result.push_back(book);
        }
        txn.commit();
    }
    catch (const std::exception& e) {
        txn.abort();
        throw std::runtime_error("Database error: " + std::string(e.what()));
    }
    
    return result;
}

nlohmann::json Database::addNewReader(const std::string& name, const std::string& date) {
    nlohmann::json result;
    pqxx::work txn(conn_);
    
    try {
        std::string insert_query = 
            "INSERT INTO Users (name, last_activity, is_active, book_amount) "
            "VALUES ($1, $2, $3, $4)";
        
        txn.exec_params(
            insert_query,
            name,
            date, 
            true,  // is_active
            0      // book_amount
        );
        
        txn.commit();
        result["status"] = "success";
        result["message"] = "User created successfully";
    }
    catch (const pqxx::unique_violation& e) {
        txn.abort();
        result["status"] = "error";
        result["message"] = "User with this name already exists";
    }
    catch (const std::exception& e) {
        txn.abort();
        result["status"] = "error";
        result["message"] = std::string("Database error: ") + e.what();
    }
    
    return result;
}


nlohmann::json Database::externalTopUpFund(int summ, const std::string& date) {
    nlohmann::json result;
    pqxx::work txn(conn_);
    
    try {
        std::string insert_query = 
            "INSERT INTO Fond (operation_type, book_id, summ, date) "
            "VALUES ($1, $2, $3, $4)";
        
        txn.exec_params(
            insert_query,
            static_cast<int>(Operation_type::EXTERNAL),
            EXTERNAL_PAY, 
            summ,  
            date
        );
        
        txn.commit();
        result["status"] = "success";
        result["message"] = "Top up is successful!";
    }
    catch (const std::exception& e) {
        txn.abort();
        result["status"] = "error";
        result["message"] = std::string("Database error: ") + e.what();
    }
    
    return result;
}