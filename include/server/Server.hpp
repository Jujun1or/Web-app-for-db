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

    restinio::request_handling_status_t handleIssueBook(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handleExtendBook(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handleReturnBook(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handleLostBook(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handleSearch(const restinio::request_handle_t& req);

    restinio::request_handling_status_t handleGetOverdue(const restinio::request_handle_t& req);
    restinio::request_handling_status_t handleGenerateLetter(const restinio::request_handle_t& req);
};