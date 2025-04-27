#include "db/Database.hpp"
#include <stdexcept>
#include <chrono>
#include <sstream>
#include <iomanip>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>

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

std::string getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
    return ss.str();
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
        pqxx::result res = txn.exec("SELECT book_id, title, price FROM Books WHERE book_id != 99999");
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

nlohmann::json Database::getAllUsers() {
    nlohmann::json result = nlohmann::json::array();
    pqxx::work txn(conn_);
    
    try {
        pqxx::result res = txn.exec("SELECT user_id, name FROM Users WHERE is_active = true");
        for (const auto& row : res) {
            nlohmann::json user;
            user["id"] = row["user_id"].as<int>();
            user["name"] = row["name"].as<std::string>();
            result.push_back(user);
        }
        txn.commit();
    }
    catch(...) {
        txn.abort();
        throw;
    }
    return result;
}

nlohmann::json Database::getCurrentBooks(int user_id) {
    nlohmann::json result = nlohmann::json::array();
    pqxx::work txn(conn_);
    
    try {
        pqxx::result res = txn.exec_params(
            "SELECT bu.bu_id, b.title, bu.issue_date, bu.duration "
            "FROM BookUser bu "
            "JOIN Books b ON bu.book_id = b.book_id "
            "WHERE bu.user_id = $1 AND bu.return_date IS NULL",
            user_id
        );
        
        for (const auto& row : res) {
            nlohmann::json book;
            book["id"] = row["bu_id"].as<int>();
            book["title"] = row["title"].as<std::string>();
            book["issue_date"] = row["issue_date"].as<std::string>();
            book["duration"] = row["duration"].as<int>();
            result.push_back(book);
        }
        txn.commit();
    }
    catch(...) {
        txn.abort();
        throw;
    }
    return result;
}

nlohmann::json Database::getUserIdByName(const std::string& name) {
    nlohmann::json result;
    pqxx::work txn(conn_);
    
    try {
        pqxx::row row = txn.exec_params1(
            "SELECT user_id FROM Users WHERE name = $1 AND is_active = true",
            name);
            
        result["user_id"] = row["user_id"].as<int>();
        result["status"] = "success";
        txn.commit();
    }
    catch(const pqxx::unexpected_rows&) {
        result["status"] = "error";
        result["message"] = "Пользователь не найден";
    }
    catch(const std::exception& e) {
        txn.abort();
        result["status"] = "error";
        result["message"] = std::string("Ошибка базы данных: ") + e.what();
    }
    return result;
}

nlohmann::json Database::bookIssueByUserId(int user_id, int book_id, const std::string& date) {
    nlohmann::json result;
    pqxx::work txn(conn_);
    
    try {
        // Проверка количества активных книг
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

        // Проверка просроченных книг
        pqxx::result overdue_check = txn.exec_params(
            "SELECT issue_date, duration FROM BookUser "
            "WHERE user_id = $1 AND return_date IS NULL",
            user_id);

        for (const auto& row : overdue_check) {
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

        // Проверка доступности книги
        pqxx::result book_available = txn.exec_params(
            "SELECT curr_amount FROM Books WHERE book_id = $1", 
            book_id);

        if (book_available[0]["curr_amount"].as<int>() < 1) {
            result["status"] = "error";
            result["message"] = "Книга отсутствует в наличии";
            txn.abort();
            return result;
        }

        // Выдача книги
        txn.exec_params(
            "INSERT INTO BookUser "
            "(book_id, user_id, issue_date, duration, return_date, extension_count) "
            "VALUES ($1, $2, $3, $4, NULL, 0)",
            book_id, 
            user_id, 
            date, 
            30);

        // Обновление количества книг
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

nlohmann::json Database::bookExtensionByUserId(int user_id, int book_id, int extension_days) {
    nlohmann::json result;
    pqxx::work txn(conn_);
    
    try {
        // Получение информации о выдаче
        pqxx::row ext_info = txn.exec_params1(
            "SELECT bu_id, issue_date, duration, extension_count "
            "FROM BookUser "
            "WHERE user_id = $1 AND book_id = $2 AND return_date IS NULL "
            "LIMIT 1",
            user_id, book_id);

        // Проверка лимита продлений
        int extension_count = ext_info["extension_count"].as<int>();
        if (extension_count >= 3) {
            result["status"] = "error";
            result["message"] = "Лимит продлений исчерпан";
            txn.abort();
            return result;
        }

        // Обновление срока
        int new_duration = ext_info["duration"].as<int>() + extension_days;
        txn.exec_params(
            "UPDATE BookUser SET "
            "duration = $1, extension_count = extension_count + 1 "
            "WHERE bu_id = $2",
            new_duration,
            ext_info["bu_id"].as<int>());

        // Расчет новой даты возврата
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

nlohmann::json Database::bookReturnByUserId(int user_id, int book_id, const std::string& date) {
    nlohmann::json result;
    pqxx::work txn(conn_);
    
    try {
        // Поиск активной записи
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

        // Обновление записи
        txn.exec_params(
            "UPDATE BookUser SET return_date = $1 "
            "WHERE bu_id = $2",
            date, 
            active_records[0]["bu_id"].as<int>());

        // Возврат книги в фонд
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

nlohmann::json Database::bookLostByUserId(int user_id, int book_id, const std::string& date) {
    nlohmann::json result;
    pqxx::work txn(conn_);
    
    try {
        // Получение информации о выдаче
        pqxx::row issue_info = txn.exec_params1(
            "SELECT bu_id, issue_date, duration FROM BookUser "
            "WHERE user_id = $1 AND book_id = $2 AND return_date IS NULL "
            "LIMIT 1",
            user_id, book_id);

        // Проверка просрочки
        std::string due_date = dateSumm(
            issue_info["issue_date"].as<std::string>(),
            issue_info["duration"].as<int>());
        
        int days_overdue = daysBetweenDates(due_date, date);
        if (days_overdue > 365) {
            result["status"] = "error";
            result["message"] = "Нельзя возместить утерю при просрочке >1 года";
            txn.abort();
            return result;
        }

        // Получение информации о книге
        pqxx::row book_info = txn.exec_params1(
            "SELECT price, total_amount FROM Books WHERE book_id = $1",
            book_id);
        
        int price = book_info["price"].as<int>();
        int total_amount = book_info["total_amount"].as<int>();

        // Обновление данных
        txn.exec_params(
            "UPDATE Books SET total_amount = $1 "
            "WHERE book_id = $2",
            total_amount - 1,
            book_id);

        txn.exec_params(
            "INSERT INTO Fond (operation_type, book_id, summ, date) "
            "VALUES ($1, $2, $3, $4)",
            static_cast<int>(Operation_type::LOST),
            book_id,
            price,
            date);

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

nlohmann::json Database::searchBooks(const std::string& query, bool search_by_author) {
    nlohmann::json result = nlohmann::json::array();
    pqxx::work txn(conn_);
    
    try {
        std::string sql = 
            "SELECT b.book_id, b.title, array_agg(a.name) as authors "
            "FROM Books b "
            "JOIN BookAuthors ba ON b.book_id = ba.book_id "
            "JOIN Authors a ON ba.author_id = a.author_id "
            "WHERE " + std::string(search_by_author ? 
                "a.name ILIKE '%' || $1 || '%'" : 
                "b.title ILIKE '%' || $1 || '%'") + " "
            "GROUP BY b.book_id, b.title "
            "ORDER BY b.title";

        pqxx::result res = txn.exec_params(sql, query);
        
        for (const auto& row : res) {
            nlohmann::json book;
            book["id"] = row["book_id"].as<int>();
            book["title"] = row["title"].as<std::string>();
            
            std::string authors_str = row["authors"].as<std::string>();
            std::vector<std::string> authors_list;

            // Удаляем фигурные скобки в начале и конце строки
            if (!authors_str.empty() && authors_str.front() == '{' && authors_str.back() == '}') {
                authors_str = authors_str.substr(1, authors_str.size() - 2);
                
                // Разделяем авторов по запятым
                size_t pos = 0;
                while ((pos = authors_str.find(',')) != std::string::npos) {
                    std::string author = authors_str.substr(0, pos);
                    authors_list.push_back(author);
                    authors_str.erase(0, pos + 1);
                }
                authors_list.push_back(authors_str); // Добавляем последнего автора
            }

            book["authors"] = authors_list;
            result.push_back(book);
        }
        txn.commit();
    }
    catch (const std::exception& e) {
        txn.abort();
        throw std::runtime_error("Search error: " + std::string(e.what()));
    }
    
    return result;
}

nlohmann::json Database::getOverdueLoans() {
    nlohmann::json result = nlohmann::json::array();
    pqxx::work txn(conn_);
    
    try {
        std::string sql = 
            "SELECT u.name, b.title, b.price, bu.bu_id, b.book_id, "
            "(CURRENT_DATE - (bu.issue_date + bu.duration)) AS days_overdue "
            "FROM BookUser bu "
            "JOIN Users u ON bu.user_id = u.user_id "
            "JOIN Books b ON bu.book_id = b.book_id "
            "WHERE bu.return_date IS NULL "
            "AND (CURRENT_DATE - (bu.issue_date + bu.duration)) > 30";
        
        pqxx::result res = txn.exec(sql);
        
        for (const auto& row : res) {
            nlohmann::json record;
            record["bu_id"] = row["bu_id"].as<int>();
            record["book_id"] = row["book_id"].as<int>();
            record["user_name"] = row["name"].as<std::string>();
            record["book_title"] = row["title"].as<std::string>();
            record["days_overdue"] = row["days_overdue"].as<int>();
            record["price"] = row["price"].as<int>();
            record["overdue_type"] = (record["days_overdue"] > 365) ? "fine" : "warning";
            
            result.push_back(record);
        }
        txn.commit();
    }
    catch (const std::exception& e) {
        txn.abort();
        throw;
    }
    return result;
}

void Database::processFinePayment(int bu_id) {
    pqxx::work txn(conn_);
    try {
        // Получаем данные о штрафе
        pqxx::row row = txn.exec_params1(
            "SELECT b.book_id, b.price "
            "FROM BookUser bu "
            "JOIN Books b ON bu.book_id = b.book_id "
            "WHERE bu.bu_id = $1", 
            bu_id);

        int book_id = row["book_id"].as<int>();
        int price = row["price"].as<int>();

        // 1. Добавляем штраф в Fond
        txn.exec_params(
            "INSERT INTO Fond (operation_type, book_id, summ, date) "
            "VALUES ($1, $2, $3, CURRENT_DATE)",
            static_cast<int>(Operation_type::FINE),
            book_id,
            price * 3);

        // 2. Помечаем книгу как возвращенную с датой-заглушкой
        txn.exec_params(
            "UPDATE BookUser SET return_date = '1111-11-11' "
            "WHERE bu_id = $1",
            bu_id);

        // 3. Уменьшаем общее количество книг
        txn.exec_params(
            "UPDATE Books SET total_amount = total_amount - 1 "
            "WHERE book_id = $1",
            book_id);

        txn.commit();
    }
    catch (...) {
        txn.abort();
        throw;
    }
}

nlohmann::json Database::generateComplaintLetter(int bu_id) {
    pqxx::work txn(conn_);
    nlohmann::json letter;
    
    try {
        pqxx::row row = txn.exec_params1(
            "SELECT u.name, b.title, b.price, "
            "(CURRENT_DATE - (bu.issue_date + bu.duration)) AS days_overdue "
            "FROM BookUser bu "
            "JOIN Users u ON bu.user_id = u.user_id "
            "JOIN Books b ON bu.book_id = b.book_id "
            "WHERE bu.bu_id = $1", 
            bu_id);

        std::string userName = row["name"].as<std::string>();
        std::string bookTitle = row["title"].as<std::string>();
        int daysOverdue = row["days_overdue"].as<int>();
        int price = row["price"].as<int>();

        if (daysOverdue > 365) {
            letter["type"] = "fine";
            letter["letter_text"] = 
                "Уважаемый(ая) " + userName + ",\n\n" +
                "В связи с просрочкой возврата книги \"" + bookTitle + "\" на " + 
                std::to_string(daysOverdue) + " дней, на вас наложен штраф.\n\n" +
                "Сумма штрафа: " + std::to_string(price * 3) + " руб. (300% от стоимости книги)\n" +
                "С уважением,\nАдминистрация библиотеки";
        } else {
            letter["type"] = "warning";
            letter["letter_text"] = 
                "Уважаемый(ая) " + userName + ",\n\n" +
                "Напоминаем, что книга \"" + bookTitle + "\" была должна быть возвращена " + 
                std::to_string(daysOverdue) + " дней назад.\n\n" +
                "Просим немедленно вернуть издание в библиотеку.\n\n" +
                "С уважением,\nАдминистрация библиотеки";
        }
        
        txn.commit();
    }
    catch (const std::exception& e) {
        txn.abort();
        throw std::runtime_error("Ошибка генерации письма: " + std::string(e.what()));
    }
    return letter;
}

nlohmann::json Database::getPopularBooks(const std::string& start_date, const std::string& end_date) {
    nlohmann::json result = nlohmann::json::array();
    pqxx::work txn(conn_);
    
    try {
        std::string sql = 
            "SELECT b.title, COUNT(*) AS loans_count, "
            "string_agg(DISTINCT a.name, ', ') AS authors " // Используем string_agg вместо array_agg
            "FROM BookUser bu "
            "JOIN Books b ON bu.book_id = b.book_id "
            "JOIN BookAuthors ba ON b.book_id = ba.book_id "
            "JOIN Authors a ON ba.author_id = a.author_id "
            "WHERE bu.issue_date BETWEEN $1 AND $2 "
            "GROUP BY b.title "
            "ORDER BY loans_count DESC "
            "LIMIT 10";

        pqxx::result res = txn.exec_params(sql, start_date, end_date);
        
        int rank = 1;
        for (const auto& row : res) {
            nlohmann::json book;
            book["rank"] = rank++;
            book["title"] = row["title"].as<std::string>();
            book["loans_count"] = row["loans_count"].as<int>();
            
            // Обработка авторов
            std::string authors_str = row["authors"].as<std::string>();
            std::vector<std::string> authors_list;
            
            // Удаляем фигурные скобки если есть
            if (!authors_str.empty()) {
                if (authors_str.front() == '{') authors_str.erase(0, 1);
                if (authors_str.back() == '}') authors_str.pop_back();
            }
            
            // Разделяем строку по запятым
            size_t pos = 0;
            while ((pos = authors_str.find(',')) != std::string::npos) {
                std::string author = authors_str.substr(0, pos);
                authors_list.push_back(author.erase(author.find_last_not_of(' ') + 1)); // Удаляем пробелы справа
                authors_str.erase(0, pos + 1);
            }
            if (!authors_str.empty()) {
                authors_list.push_back(authors_str.erase(authors_str.find_last_not_of(' ') + 1));
            }
            
            book["authors"] = authors_list;
            result.push_back(book);
        }
        txn.commit();
    }
    catch (const std::exception& e) {
        txn.abort();
        throw std::runtime_error("Ошибка получения популярных книг: " + std::string(e.what()));
    }
    return result;
}

nlohmann::json Database::getReadersStats(int year) {
    nlohmann::json result = nlohmann::json::array();
    pqxx::work txn(conn_);
    
    try {
        std::string sql = 
            "SELECT u.name, COUNT(*) AS books_count "
            "FROM BookUser bu "
            "JOIN Users u ON bu.user_id = u.user_id "
            "WHERE bu.return_date IS NOT NULL "
            "AND bu.return_date != '1111-11-11' "
            "AND EXTRACT(YEAR FROM bu.return_date) = $1 "
            "GROUP BY u.name "
            "ORDER BY books_count DESC";

        pqxx::result res = txn.exec_params(sql, year);
        
        int rank = 1;
        for (const auto& row : res) {
            nlohmann::json reader;
            reader["rank"] = rank++;
            reader["name"] = row["name"].as<std::string>();
            reader["books_count"] = row["books_count"].as<int>();
            result.push_back(reader);
        }
        txn.commit();
    }
    catch (const std::exception& e) {
        txn.abort();
        throw std::runtime_error("Ошибка получения статистики: " + std::string(e.what()));
    }
    return result;
}

nlohmann::json Database::getFinancialReport(int year) {
    nlohmann::json report = nlohmann::json::object();
    pqxx::work txn(conn_);
    
    try {
        // 1. Общая стоимость книг
        pqxx::row total_books = txn.exec_params1(
            "SELECT SUM(price * total_amount) FROM Books");
        int total_assets = total_books[0].as<int>();

        // 2. Сбор данных по месяцам
        nlohmann::json months = nlohmann::json::array();
        std::array<int, 4> q_expenses = {0}, q_income = {0}, q_total = {0};

        for (int month = 1; month <= 12; ++month) {
            // 2.1. Получаем расходы (стоимость книг для LOST/FINE)
            pqxx::row expenses_row = txn.exec_params1(
                "SELECT COALESCE(SUM(b.price), 0) "
                "FROM Fond f "
                "JOIN Books b ON f.book_id = b.book_id "
                "WHERE EXTRACT(YEAR FROM f.date) = $1 "
                "AND EXTRACT(MONTH FROM f.date) = $2 "
                "AND f.operation_type IN (2, 3)", // LOST и FINE
                year, 
                month);
            
            int expenses = expenses_row[0].as<int>();

            // 2.2. Получаем доходы (оригинальная логика)
            pqxx::row income_row = txn.exec_params1(
                "SELECT COALESCE(SUM(summ), 0) "
                "FROM Fond "
                "WHERE EXTRACT(YEAR FROM date) = $1 "
                "AND EXTRACT(MONTH FROM date) = $2 "
                "AND operation_type IN (1, 2, 3)", // EXTERNAL, LOST, FINE
                year,
                month);
            
            int income = income_row[0].as<int>();

            // 2.3. Рассчитываем квартал
            int quarter = (month - 1) / 3;
            
            // 2.4. Сохраняем данные месяца
            nlohmann::json month_data;
            month_data["month"] = month;
            month_data["expenses"] = expenses;
            month_data["income"] = income;
            months.push_back(month_data);

            // 2.5. Обновляем квартальные суммы
            q_expenses[quarter] += expenses;
            q_income[quarter] += income;
            q_total[quarter] += (income - expenses);
        }

        // 3. Формирование отчета
        report["months"] = months;
        report["total_assets"] = total_assets;
        
        // 4. Расчет годовых итогов
        int total_expenses = std::accumulate(q_expenses.begin(), q_expenses.end(), 0);
        int total_income = std::accumulate(q_income.begin(), q_income.end(), 0);
        int total_profit = total_assets + (total_income - total_expenses);

        report["year_total"] = {
            {"expenses", total_expenses},
            {"income", total_income},
            {"total", total_assets + total_profit}
        };

        txn.commit();
    }
    catch(const std::exception& e) {
        txn.abort();
        throw std::runtime_error("Ошибка формирования отчета: " + std::string(e.what()));
    }
    
    return report;
}

nlohmann::json Database::deactivateInactiveUsers() {
    nlohmann::json result;
    pqxx::work txn(conn_);
    
    try {
        // 1. Находим пользователей для отчисления
        std::string select_sql = 
            "SELECT user_id, name "
            "FROM Users "
            "WHERE is_active = TRUE "
            "AND last_activity < CURRENT_DATE - INTERVAL '1 year' "
            "AND user_id NOT IN ("
            "   SELECT user_id FROM BookUser "
            "   WHERE return_date IS NULL"
            ")";
        
        pqxx::result res = txn.exec(select_sql);
        
        // 2. Формируем список отчисленных
        nlohmann::json deactivated = nlohmann::json::array();
        for (const auto& row : res) {
            nlohmann::json user;
            user["user_id"] = row["user_id"].as<int>();
            user["name"] = row["name"].as<std::string>();
            deactivated.push_back(user);
        }
        
        // 3. Обновляем статус пользователей
        if (!deactivated.empty()) {
            txn.exec(
                "UPDATE Users SET is_active = FALSE "
                "WHERE user_id IN ("
                "   SELECT user_id FROM Users "
                "   WHERE is_active = TRUE "
                "   AND last_activity < CURRENT_DATE - INTERVAL '1 year' "
                "   AND user_id NOT IN ("
                "       SELECT user_id FROM BookUser "
                "       WHERE return_date IS NULL"
                "   )"
                ")"
            );
        }
        
        txn.commit();
        result["status"] = "success";
        result["deactivated_users"] = deactivated;
    }
    catch (const std::exception& e) {
        txn.abort();
        result["status"] = "error";
        result["message"] = "Ошибка отчисления: " + std::string(e.what());
    }
    
    return result;
}