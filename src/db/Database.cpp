#include "db/Database.hpp"
#include <stdexcept>
#include <chrono>
#include <sstream>
#include <iomanip>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>

// Парсинг даты из строки формата "YYYY-MM-DD"
std::chrono::year_month_day parseDate(const std::string& dateStr) {
    std::istringstream iss(dateStr);
    int year, month, day;
    char delim1, delim2;
    
    if (!(iss >> year >> delim1 >> month >> delim2 >> day) || 
        delim1 != '-' || delim2 != '-') 
    {
        throw std::runtime_error("Invalid date format");
    }
    
    // Создаем объект даты с проверкой валидности
    std::chrono::year y{year};
    std::chrono::month m{static_cast<unsigned>(month)};
    std::chrono::day d{static_cast<unsigned>(day)};
    auto ymd = y/m/d;
    
    if (!ymd.ok()) {
        throw std::runtime_error("Invalid date value");
    }
    
    return ymd;
}

// Расчет дней между двумя датами
int daysBetweenDates(const std::string& startDate, const std::string& endDate) {
    try {
        auto start = std::chrono::sys_days(parseDate(startDate));
        auto end = std::chrono::sys_days(parseDate(endDate));
        return (end - start).count();
    }
    catch (...) {
        throw std::runtime_error("Error in days calculation");
    }
}

// Добавление дней к дате
std::string dateSumm(const std::string& startDate, int days) {
    try {
        auto start = std::chrono::sys_days(parseDate(startDate));
        auto end = start + std::chrono::days{days};
        
        // Преобразуем обратно в дату
        auto end_date = std::chrono::year_month_day{end};
        
        // Обработка последнего дня месяца
        if (!end_date.ok()) {
            end_date = end_date.year()/end_date.month()/std::chrono::last;
        }
        
        // Форматирование в строку
        std::ostringstream oss;
        oss << std::setfill('0')
            << std::setw(4) << static_cast<int>(end_date.year()) << "-"
            << std::setw(2) << static_cast<unsigned>(end_date.month()) << "-"
            << std::setw(2) << static_cast<unsigned>(end_date.day());
            
        return oss.str();
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Date calculation error: " + std::string(e.what()));
    }
}

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

nlohmann::json Database::bookIssue(int user_id, int book_id, const std::string& date) {
    nlohmann::json result;
    pqxx::work txn(conn_);
    
    try {
        // 1. Проверка активных книг пользователя
        pqxx::result active_books = txn.exec_params(
            "SELECT COUNT(*) FROM BookUser "
            "WHERE user_id = $1 AND return_date IS NULL", 
            user_id);

        if (active_books[0][0].as<int>() >= 3) {
            result["status"] = "error";
            result["message"] = "У пользователя уже 3 книги на руках";
            txn.abort();
            return result;
        }

        // 2. Проверка просрочек по активным книгам
        pqxx::result overdue_check = txn.exec_params(
            "SELECT issue_date, duration FROM BookUser "
            "WHERE user_id = $1 AND return_date IS NULL",
            user_id);

        for (const auto& row : overdue_check) {
            // Рассчитываем due_date для каждой активной книги
            std::string due_date = dateSumm(
                row["issue_date"].as<std::string>(),
                row["duration"].as<int>());
            
            int days_overdue = daysBetweenDates(due_date, date);
            
            if (days_overdue > 0) {
                result["status"] = "error";
                result["message"] = "Имеется просрочка по текущим книгам";
                txn.abort();
                return result;
            }
        }

        // 3. Проверка наличия книги
        pqxx::result book_available = txn.exec_params(
            "SELECT curr_amount FROM Books WHERE book_id = $1", 
            book_id);

        if (book_available[0]["curr_amount"].as<int>() < 1) {
            result["status"] = "error";
            result["message"] = "Книга отсутствует в наличии";
            txn.abort();
            return result;
        }

        // 4. Выдача книги с NULL в return_date
        txn.exec_params(
            "INSERT INTO BookUser "
            "(book_id, user_id, issue_date, duration, return_date, extension_count) "
            "VALUES ($1, $2, $3, $4, NULL, 0)",
            book_id, 
            user_id, 
            date, 
            30);  // duration = 30 дней

        // 5. Обновление количества книг
        txn.exec_params(
            "UPDATE Books SET curr_amount = curr_amount - 1 "
            "WHERE book_id = $1", 
            book_id);

        txn.commit();
        result["status"] = "success";
        result["message"] = "Книга успешно выдана";
    }
    catch (const std::exception& e) {
        txn.abort();
        result["status"] = "error";
        result["message"] = std::string("Ошибка: ") + e.what();
    }
    return result;
}

nlohmann::json Database::bookExtension(int user_id, int book_id, int extensionTime) {
    nlohmann::json result;
    pqxx::work txn(conn_);
    
    try {
        // 1. Получение информации о выдаче
        pqxx::row ext_info = txn.exec_params1(
            "SELECT bu_id, issue_date, duration, extension_count "
            "FROM BookUser "
            "WHERE user_id = $1 AND book_id = $2 AND return_date IS NULL "
            "LIMIT 1",
            user_id, book_id);

        int extension_count = ext_info["extension_count"].as<int>();
        if (extension_count >= 3) {
            result["status"] = "error";
            result["message"] = "Лимит продлений исчерпан";
            txn.abort();
            return result;
        }

        // 2. Обновление duration
        int new_duration = ext_info["duration"].as<int>() + extensionTime;
        
        // 3. Обновление записи
        txn.exec_params(
            "UPDATE BookUser SET "
            "duration = $1, extension_count = extension_count + 1 "
            "WHERE bu_id = $2",
            new_duration,
            ext_info["bu_id"].as<int>());

        // 4. Расчет новой даты возврата для ответа
        std::string new_due_date = dateSumm(
            ext_info["issue_date"].as<std::string>(),
            new_duration);

        txn.commit();
        result["status"] = "success";
        result["new_due_date"] = new_due_date;
    }
    catch (const pqxx::unexpected_rows&) {
        txn.abort();
        result["status"] = "error";
        result["message"] = "Активная запись о книге не найдена";
    }
    catch (const std::exception& e) {
        txn.abort();
        result["status"] = "error";
        result["message"] = std::string("Ошибка: ") + e.what();
    }
    return result;
}

nlohmann::json Database::bookReturn(int user_id, int book_id, const std::string& date) {
    nlohmann::json result;
    pqxx::work txn(conn_);
    
    try {
        // 1. Поиск активной записи
        pqxx::result active_records = txn.exec_params(
            "SELECT bu_id FROM BookUser "
            "WHERE user_id = $1 AND book_id = $2 AND return_date IS NULL",
            user_id, book_id);

        if (active_records.empty()) {
            result["status"] = "error";
            result["message"] = "Активная запись не найдена";
            txn.abort();
            return result;
        }

        // 2. Обновление записи о возврате
        txn.exec_params(
            "UPDATE BookUser SET return_date = $1 "
            "WHERE bu_id = $2",
            date, 
            active_records[0]["bu_id"].as<int>());

        // 3. Возврат книги в фонд
        txn.exec_params(
            "UPDATE Books SET curr_amount = curr_amount + 1 "
            "WHERE book_id = $1", 
            book_id);

        txn.commit();
        result["status"] = "success";
        result["message"] = "Книга успешно возвращена";
    }
    catch (const std::exception& e) {
        txn.abort();
        result["status"] = "error";
        result["message"] = std::string("Ошибка: ") + e.what();
    }
    return result;
}

nlohmann::json Database::bookLost(int user_id, int book_id, const std::string& date) {
    nlohmann::json result;
    pqxx::work txn(conn_);
    
    try {
        // 1. Получение информации о выдаче
        pqxx::row issue_info = txn.exec_params1(
            "SELECT bu_id, issue_date, duration FROM BookUser "
            "WHERE user_id = $1 AND book_id = $2 AND return_date IS NULL "
            "LIMIT 1",
            user_id, book_id);

        // 2. Расчет даты возврата и просрочки
        std::string due_date = dateSumm(
            issue_info["issue_date"].as<std::string>(),
            issue_info["duration"].as<int>());
        
        int days_overdue = daysBetweenDates(due_date, date);

        // 3. Проверка максимальной просрочки
        if (days_overdue > 365) {
            result["status"] = "error";
            result["message"] = "Нельзя возместить утерю при просрочке >1 года";
            txn.abort();
            return result;
        }

        // 4. Получение информации о книге
        pqxx::row book_info = txn.exec_params1(
            "SELECT price, total_amount FROM Books WHERE book_id = $1",
            book_id);
        
        int price = book_info["price"].as<int>();
        int total_amount = book_info["total_amount"].as<int>();

        // 5. Обновление общего количества книг
        txn.exec_params(
            "UPDATE Books SET total_amount = $1 "
            "WHERE book_id = $2",
            total_amount - 1,
            book_id);

        // 6. Запись возмещения в Fond
        txn.exec_params(
            "INSERT INTO Fond (operation_type, book_id, summ, date) "
            "VALUES ($1, $2, $3, $4)",
            static_cast<int>(Operation_type::LOST),
            book_id,
            price,  // Фиксированная сумма возмещения
            date);

        // 7. Пометить как возвращенную
        txn.exec_params(
            "UPDATE BookUser SET return_date = $1 "
            "WHERE bu_id = $2",
            date, 
            issue_info["bu_id"].as<int>());

        txn.commit();
        result["status"] = "success";
        result["message"] = "Утеря оформлена. Стоимость возмещена: " + std::to_string(price) + " руб.";
    }
    catch (const pqxx::unexpected_rows&) {
        txn.abort();
        result["status"] = "error";
        result["message"] = "Книга не найдена у пользователя";
    }
    catch (const std::exception& e) {
        txn.abort();
        result["status"] = "error";
        result["message"] = std::string("Ошибка: ") + e.what();
    }
    return result;
}