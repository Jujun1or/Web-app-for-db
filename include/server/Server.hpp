#pragma once
#include <restinio/all.hpp>
#include "../db/Database.hpp"

class Server {
public:
    explicit Server(Database& db);
    void start();
    
private:
    Database& db_;
    
    restinio::request_handling_status_t handler(restinio::request_handle_t req);
    restinio::request_handling_status_t handleViewBooks(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handleAddUser(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handleExternalTopUp(const restinio::request_handle_t& req);

    restinio::request_handling_status_t handleUserIssueBook(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handleUserExtendBook(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handleUserReturnBook(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handleUserLostBook(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handleSearch(const restinio::request_handle_t& req);

    restinio::request_handling_status_t handleGetOverdue(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handleGenerateLetter(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handlePayFine(const restinio::request_handle_t& req);

    restinio::request_handling_status_t handlePopularBooks(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handleReadersStats(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handleFinancialReport(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handleDeactivateUsers(const restinio::request_handle_t& req);

    restinio::request_handling_status_t handleUsersList(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handleCurrentBooks(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handleGetUserId(const restinio::request_handle_t& req);
};