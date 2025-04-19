#include "server/Server.hpp"
#include <nlohmann/json.hpp>
#include <restinio/helpers/http_field_parsers/bearer_auth.hpp>

Server::Server(Database& db)
    : db_(db) {}

restinio::request_handling_status_t Server::handleViewBooks(const restinio::request_handle_t& req) {
    try {
        auto books = db_.getAllBooks();
        req->create_response(restinio::status_ok())
            .append_header(restinio::http_field::content_type, "application/json")
            .set_body(books.dump())
            .done();
        return restinio::request_accepted();
    }
    catch(const std::exception& e) {
        req->create_response(restinio::status_internal_server_error())
            .append_header(restinio::http_field::content_type, "application/json")
            .set_body(nlohmann::json{{"error", e.what()}}.dump())
            .done();
        return restinio::request_accepted();
    }
}

restinio::request_handling_status_t Server::handleAddUser(const restinio::request_handle_t& req) {
    try {
        const auto body = nlohmann::json::parse(req->body());
        const std::string name = body["name"];
        const std::string date = body.value("date", "");
        
        auto result = db_.addNewReader(name, date);
        
        auto status = result["status"] == "success"
            ? restinio::status_created()
            : restinio::status_bad_request();
        
        req->create_response(status)
            .append_header(restinio::http_field::content_type, "application/json")
            .set_body(result.dump())
            .done();
               
        return restinio::request_accepted();
    }
    catch(const std::exception& e) {
        req->create_response(restinio::status_bad_request())
            .append_header(restinio::http_field::content_type, "application/json")
            .set_body(nlohmann::json{{"error", e.what()}}.dump())
            .done();
        return restinio::request_accepted();
    }
}

restinio::request_handling_status_t Server::handler(restinio::request_handle_t req) {
    const auto method = req->header().method();
    const auto target = req->header().request_target();

    if(method == restinio::http_method_get()) {
        if(target == "/books") {
            return handleViewBooks(req);
        }
    }
    else if(method == restinio::http_method_post()) {
        if(target == "/users") {
            return handleAddUser(req);
        }
    }
    
    return restinio::request_rejected();
}

void Server::start() {
    struct my_traits : public restinio::default_traits_t {
        using logger_t = restinio::shared_ostream_logger_t;
    };

    restinio::run(
        restinio::on_this_thread<my_traits>()
            .port(8080)
            .address("localhost")
            .request_handler([this](auto req) {
                return this->handler(std::move(req));
            }));
}