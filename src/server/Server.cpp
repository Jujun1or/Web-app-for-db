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

restinio::request_handling_status_t Server::handleUsersList(const restinio::request_handle_t& req) {
    try {
        auto users = db_.getAllUsers();
        return req->create_response()
            .append_header(restinio::http_field::content_type, "application/json")
            .set_body(users.dump())
            .done();
    }
    catch(...) {
        return req->create_response(restinio::status_internal_server_error()).done();
    }
}

restinio::request_handling_status_t Server::handleCurrentBooks(const restinio::request_handle_t& req) {
    try {
        auto params = restinio::parse_query(req->header().query());
        int user_id = std::stoi(std::string{params["user_id"]});
        
        auto books = db_.getCurrentBooks(user_id);
        return req->create_response()
            .append_header(restinio::http_field::content_type, "application/json")
            .set_body(books.dump())
            .done();
    }
    catch(...) {
        return req->create_response(restinio::status_bad_request()).done();
    }
}

restinio::request_handling_status_t Server::handleUserIssueBook(const restinio::request_handle_t& req) {
    try {
        auto body = nlohmann::json::parse(req->body());
        auto result = db_.bookIssueByUserId(
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

restinio::request_handling_status_t Server::handleUserExtendBook(const restinio::request_handle_t& req) {
    try {
        auto body = nlohmann::json::parse(req->body());
        auto result = db_.bookExtensionByUserId(
            body["user_id"].get<int>(),
            body["book_id"].get<int>(),
            body["extension_days"].get<int>()
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

restinio::request_handling_status_t Server::handleUserReturnBook(const restinio::request_handle_t& req) {
    try {
        auto body = nlohmann::json::parse(req->body());
        auto result = db_.bookReturnByUserId(
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

restinio::request_handling_status_t Server::handleUserLostBook(const restinio::request_handle_t& req) {
    try {
        auto body = nlohmann::json::parse(req->body());
        auto result = db_.bookLostByUserId(
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

restinio::request_handling_status_t Server::handleGetOverdue(const restinio::request_handle_t& req) {
    try {
        nlohmann::json response;
        response["data"] = db_.getOverdueLoans();
        
        return req->create_response()
            .append_header(restinio::http_field::content_type, "application/json")
            .append_header("Access-Control-Allow-Origin", "*")
            .set_body(response.dump())
            .done();
    }
    catch(const std::exception& e) {
        nlohmann::json error;
        error["status"] = "error";
        error["message"] = e.what();
        
        return req->create_response(restinio::status_internal_server_error())
            .append_header("Content-Type", "application/json")
            .set_body(error.dump())
            .done();
    }
}

restinio::request_handling_status_t Server::handleGenerateLetter(const restinio::request_handle_t& req) {
    try {
        auto body = nlohmann::json::parse(req->body());
        auto bu_id = body["bu_id"].get<int>();
        
        auto letter = db_.generateComplaintLetter(bu_id);
        
        return req->create_response()
            .append_header("Content-Type", "application/json")
            .set_body(letter.dump())
            .done();
    }
    catch(const std::exception& e) {
        return req->create_response(restinio::status_bad_request())
            .set_body(nlohmann::json{{"error", e.what()}}.dump())
            .done();
    }
}

restinio::request_handling_status_t Server::handlePayFine(const restinio::request_handle_t& req) {
    try {
        auto body = nlohmann::json::parse(req->body());
        int bu_id = body["bu_id"].get<int>();
        
        db_.processFinePayment(bu_id);
        
        return req->create_response()
            .append_header("Content-Type", "application/json")
            .set_body(nlohmann::json{{"status", "success"}}.dump())
            .done();
    }
    catch(const std::exception& e) {
        return req->create_response(restinio::status_bad_request())
            .set_body(nlohmann::json{{"error", e.what()}}.dump())
            .done();
    }
}

restinio::request_handling_status_t Server::handlePopularBooks(const restinio::request_handle_t& req) {
    try {
        auto params = restinio::parse_query(req->header().query());
        
        // Используем метод find и operator[] для доступа к параметрам
        std::string start_date;
        if (params.has("start_date")) {
            start_date = params["start_date"];
        }
        
        std::string end_date;
        if (params.has("end_date")) {
            end_date = params["end_date"];
        }

        auto result = db_.getPopularBooks(start_date, end_date);
        
        return req->create_response()
            .append_header(restinio::http_field::content_type, "application/json")
            .set_body(result.dump())
            .done();
    }
    catch(const std::exception& e) {
        return req->create_response(restinio::status_bad_request())
            .set_body(nlohmann::json{{"error", e.what()}}.dump())
            .done();
    }
}

restinio::request_handling_status_t Server::handleReadersStats(const restinio::request_handle_t& req) {
    try {
        auto params = restinio::parse_query(req->header().query());
        
        int year = 2025;
        if (params.has("year")) {
            std::string year_str{params["year"]};
            year = std::stoi(year_str);
        }

        auto result = db_.getReadersStats(year);
        
        return req->create_response()
            .append_header(restinio::http_field::content_type, "application/json")
            .set_body(result.dump())
            .done();
    }
    catch(const std::exception& e) {
        return req->create_response(restinio::status_bad_request())
            .set_body(nlohmann::json{{"error", e.what()}}.dump())
            .done();
    }
}

restinio::request_handling_status_t Server::handleFinancialReport(const restinio::request_handle_t& req) {
    try {
        auto params = restinio::parse_query(req->header().query());
        int year = 2024;
        
        if(params.has("year")) {
            std::string year_str{params["year"]};
            year = std::stoi(year_str);
        }

        auto result = db_.getFinancialReport(year);
        
        return req->create_response()
            .append_header(restinio::http_field::content_type, "application/json")
            .set_body(result.dump())
            .done();
    }
    catch(const std::exception& e) {
        return req->create_response(restinio::status_bad_request())
            .set_body(nlohmann::json{{"error", e.what()}}.dump())
            .done();
    }
}

restinio::request_handling_status_t Server::handleDeactivateUsers(const restinio::request_handle_t& req) {
    try {
        auto result = db_.deactivateInactiveUsers();
        return req->create_response()
            .append_header(restinio::http_field::content_type, "application/json")
            .set_body(result.dump())
            .done();
    }
    catch(const std::exception& e) {
        return req->create_response(restinio::status_bad_request())
            .set_body(nlohmann::json{{"error", e.what()}}.dump())
            .done();
    }
}

restinio::request_handling_status_t Server::handleGetUserId(const restinio::request_handle_t& req) {
    try {
        auto params = restinio::parse_query(req->header().query());
        std::string name{params["name"]};
        
        auto result = db_.getUserIdByName(name);
        return req->create_response()
            .append_header(restinio::http_field::content_type, "application/json")
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



restinio::request_handling_status_t Server::handler(restinio::request_handle_t req) {
    const auto target = req->header().request_target();
    
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
        else if(target == "/overdue") {
            return handleGetOverdue(req);
        }
        else if(target.starts_with("/reports/popular")) {
            return handlePopularBooks(req);
        }
        else if(target.starts_with("/reports/readers")) {
            return handleReadersStats(req);
        }
        else if(target.starts_with("/reports/financial")) {
            return handleFinancialReport(req);
        }
        else if(target == "/users-list") {
            return handleUsersList(req);
        }
        else if(target.starts_with("/current-books")) {
            return handleCurrentBooks(req);
        }
        else if(target.starts_with("/user-id")) {
            return handleGetUserId(req);
        }
    }
    else if(req->header().method() == restinio::http_method_post()) {
        if (target == "/users"){
            return handleAddUser(req);
        }
        else if (target == "/fond_top_up"){
            return handleExternalTopUp(req);
        }
        else if(target == "/user-operations/issue-book") {
            return handleUserIssueBook(req);
        }
        else if(target == "/user-operations/extend-book") {
            return handleUserExtendBook(req);
        }
        else if(target == "/user-operations/return-book") {
            return handleUserReturnBook(req);
        }
        else if(target == "/user-operations/lost-book") {
            return handleUserLostBook(req);
        }
        else if(target == "/search") {
            return handleSearch(req);
        }
        else if(target == "/generate-letter") {
            return handleGenerateLetter(req);
        }
        else if(target == "/pay-fine") {
            return handlePayFine(req);
        }
        else if(target == "/deactivate-users") {
            return handleDeactivateUsers(req);
        }
        else if(target.starts_with("/user-id")) {
            return handleGetUserId(req);
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

