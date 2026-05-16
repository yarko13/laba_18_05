#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include "Record.h"

// Собственный класс исключения
class DbException : public std::runtime_error {
public:
    explicit DbException(const std::string& msg) : std::runtime_error(msg) {}
};

struct sqlite3;

class Database {
public:
    explicit Database(const std::string& path);
    ~Database();

    void createTable(const std::string& tableName,
        const std::vector<std::string>& columns);

    void insertRecord(const std::string& tableName,
        const std::vector<std::string>& values);

    std::vector<Record> getRecords(const std::string& tableName);

    void updateRecord(const std::string& tableName, int id,
        const std::string& column,
        const std::string& newValue);

    void deleteRecord(const std::string& tableName, int id);

    void dropTable(const std::string& tableName);  // НОВЫЙ МЕТОД

    std::vector<std::string> getTableNames();

    std::vector<std::string> getColumns(const std::string& tableName);

private:
    sqlite3* db_ = nullptr;
    void exec(const std::string& sql);
};