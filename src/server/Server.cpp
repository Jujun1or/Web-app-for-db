#include "server/Server.hpp"
#include <nlohmann/json.hpp>
#include <restinio/helpers/http_field_parsers/bearer_auth.hpp>
#include <fstream>
#include <sstream>

Server::Server(Database& db)
    : db_(db) {}

restinio::request_handling_status_t send_file(
    const restinio::request_handle_t& req,
    const std::string& path,
    const std::string& content_type = "text/html; charset=utf-8")
{
    try {
        std::ifstream file(path, std::ios::binary);
        if(!file.is_open()) {
            return req->create_response(restinio::status_not_found())
                .connection_close()
                .done();
        }
        
        std::ostringstream ss;
        ss << file.rdbuf();
        std::string content = ss.str();
        
        return req->create_response()
            .append_header(restinio::http_field::content_type, content_type)
            .set_body(content)
            .done();
    }
    catch(const std::exception& e) {
        std::cerr << "File error: " << e.what() << std::endl;
        return req->create_response(restinio::status_internal_server_error())
            .done();
    }
}

restinio::request_handling_status_t Server::handleViewBooks(const restinio::request_handle_t& req) {
    try {
        auto books = db_.getAllBooks();
        
        return req->create_response()
            .append_header(restinio::http_field::content_type, "application/json")
            .append_header("Access-Control-Allow-Origin", "*") // CORS
            .set_body(books.dump())
            .done();
    }
    catch(...) {
        return req->create_response(restinio::status_internal_server_error()).done();
    }
}

restinio::request_handling_status_t Server::handleAddUser(const restinio::request_handle_t& req) {
    try {
        const auto body = nlohmann::json::parse(req->body());
        
        auto result = db_.addNewReader(
            body["name"].get<std::string>(),
            body.value("date", "")
        );
        
        return req->create_response()
            .append_header(restinio::http_field::content_type, "application/json")
            .append_header("Access-Control-Allow-Origin", "*") // CORS
            .set_body(result.dump())
            .done();
    }
    catch(...) {
        return req->create_response(restinio::status_bad_request()).done();
    }
}

restinio::request_handling_status_t Server::handleExternalTopUp(const restinio::request_handle_t& req) {
    try {
        const auto body = nlohmann::json::parse(req->body());
        
        auto result = db_.externalTopUpFund(
            body["summ"].get<int>(),
            body.value("date", "")
        );
        
        return req->create_response()
            .append_header(restinio::http_field::content_type, "application/json")
            .append_header("Access-Control-Allow-Origin", "*") // CORS
            .set_body(result.dump())
            .done();
    }
    catch(...) {
        return req->create_response(restinio::status_bad_request()).done();
    }
}

restinio::request_handling_status_t Server::handleIssueBook(const restinio::request_handle_t& req) {
    try {
        auto body = nlohmann::json::parse(req->body());
        auto result = db_.bookIssue(
            body["user_id"].get<int>(),
            body["book_id"].get<int>(),
            body["date"].get<std::string>()
        );
        
        return req->create_response()
            .append_header("Access-Control-Allow-Origin", "*")
            .set_body(result.dump())
            .done();
    }
    catch(...) {
        return req->create_response(restinio::status_bad_request()).done();
    }
}

restinio::request_handling_status_t Server::handleExtendBook(const restinio::request_handle_t& req) {
    try {
        auto body = nlohmann::json::parse(req->body());
        
        // Проверка обязательных полей
        if (!body.contains("user_id") || !body.contains("book_id") || !body.contains("extensionTime")) {
            return req->create_response(restinio::status_bad_request())
                .set_body(R"({"status": "error", "message": "Missing required fields"})")
                .done();
        }

        auto result = db_.bookExtension(
            body["user_id"].get<int>(),
            body["book_id"].get<int>(),
            body["extensionTime"].get<int>()
        );
        
        return req->create_response()
            .append_header("Access-Control-Allow-Origin", "*")
            .append_header(restinio::http_field::content_type, "application/json")
            .set_body(result.dump())
            .done();
    }
    catch(const nlohmann::json::exception& e) {
        return req->create_response(restinio::status_bad_request())
            .append_header("Access-Control-Allow-Origin", "*")
            .set_body(nlohmann::json{
                {"status", "error"},
                {"message", std::string("JSON parsing error: ") + e.what()}
            }.dump())
            .done();
    }
    catch(const std::exception& e) {
        return req->create_response(restinio::status_internal_server_error())
            .append_header("Access-Control-Allow-Origin", "*")
            .set_body(nlohmann::json{
                {"status", "error"},
                {"message", std::string("Internal error: ") + e.what()}
            }.dump())
            .done();
    }
}

restinio::request_handling_status_t Server::handleReturnBook(const restinio::request_handle_t& req) {
    try {
        auto body = nlohmann::json::parse(req->body());
        auto result = db_.bookReturn(
            body["user_id"].get<int>(),
            body["book_id"].get<int>(),
            body["date"].get<std::string>()
        );
        
        return req->create_response()
            .append_header("Access-Control-Allow-Origin", "*")
            .set_body(result.dump())
            .done();
    }
    catch(...) {
        return req->create_response(restinio::status_bad_request())
            .append_header("Access-Control-Allow-Origin", "*")
            .done();
    }
}

restinio::request_handling_status_t Server::handleLostBook(const restinio::request_handle_t& req) {
    try {
        auto body = nlohmann::json::parse(req->body());
        auto result = db_.bookLost(
            body["user_id"].get<int>(),
            body["book_id"].get<int>(),
            body["date"].get<std::string>()
        );
        
        return req->create_response()
            .append_header("Access-Control-Allow-Origin", "*")
            .set_body(result.dump())
            .done();
    }
    catch(...) {
        return req->create_response(restinio::status_bad_request())
            .append_header("Access-Control-Allow-Origin", "*")
            .done();
    }
}

restinio::request_handling_status_t Server::handleSearch(const restinio::request_handle_t& req) {
    try {
        const auto body = nlohmann::json::parse(req->body());
        
        bool search_by_author = body.value("search_type", "title") == "author";
        std::string query = body["query"].get<std::string>();
        
        auto results = db_.searchBooks(query, search_by_author);
        
        return req->create_response()
            .append_header(restinio::http_field::content_type, "application/json")
            .append_header("Access-Control-Allow-Origin", "*")
            .set_body(results.dump())
            .done();
    }
    catch(const std::exception& e) {
        return req->create_response(restinio::status_bad_request())
            .set_body(nlohmann::json{{"error", e.what()}}.dump())
            .done();
    }
}

restinio::request_handling_status_t Server::handler(restinio::request_handle_t req) {
    const auto target = req->header().request_target();
    
    // Обработка статических файлов
    if(req->header().method() == restinio::http_method_get()) {
        if(target == "/") {
            return send_file(req, "frontend/static/index.html");
        }
        else if(target == "/style.css") {
            return send_file(req, "frontend/static/style.css", "text/css; charset=utf-8");
        }
        else if(target == "/script.js") {
            return send_file(req, "frontend/static/script.js", "text/javascript");
        }
        else if(target == "/books") {
            return handleViewBooks(req);
        }
    }
    else if(req->header().method() == restinio::http_method_post()) {
        if (target == "/users"){
            return handleAddUser(req);
        }
        else if (target == "/fond_top_up"){
            return handleExternalTopUp(req);
        }
        else if(target == "/issue-book") {
            return handleIssueBook(req);
        }
        else if(target == "/extend-book") {
            return handleExtendBook(req);
        }
        else if(target == "/return-book") {
            return handleReturnBook(req);
        }
        else if(target == "/lost-book") {
            return handleLostBook(req);
        }
        else if(target == "/search") {
            return handleSearch(req);
        }
    }
    
    return req->create_response(restinio::status_not_found())
        .append_header(restinio::http_field::content_type, "text/plain")
        .set_body("404 Not Found")
        .done();
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