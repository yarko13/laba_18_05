#include "Database.h"
#include "sqlite3.h"
#include <sstream>

Database::Database(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK)
        throw DbException("Cannot open database: " + std::string(sqlite3_errmsg(db_)));
}

Database::~Database() {
    if (db_) sqlite3_close(db_);
}

void Database::exec(const std::string& sql) {
    char* errMsg = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string err = errMsg ? errMsg : "Unknown error";
        sqlite3_free(errMsg);
        throw DbException("SQL error: " + err);
    }
}

void Database::createTable(const std::string& name,
    const std::vector<std::string>& cols) {
    std::ostringstream ss;
    ss << "CREATE TABLE IF NOT EXISTS " << name
        << " (id INTEGER PRIMARY KEY AUTOINCREMENT";
    for (auto& c : cols) ss << ", " << c << " TEXT";
    ss << ");";
    try { exec(ss.str()); }
    catch (const DbException& e) { throw; }
}

std::vector<std::string> Database::getColumns(const std::string& table) {
    std::vector<std::string> cols;
    auto cb = [](void* d, int, char** argv, char**) -> int {
        if (argv[1] && std::string(argv[1]) != "id")
            static_cast<std::vector<std::string>*>(d)->push_back(argv[1]);
        return 0;
        };
    char* err = nullptr;
    int rc = sqlite3_exec(db_,
        ("PRAGMA table_info(" + table + ");").c_str(),
        cb, &cols, &err);
    if (rc != SQLITE_OK) {
        std::string msg = err ? err : "unknown";
        sqlite3_free(err);
        throw DbException("getColumns failed: " + msg);
    }
    return cols;
}

void Database::insertRecord(const std::string& table,
    const std::vector<std::string>& values) {
    auto cols = getColumns(table);
    if (cols.size() != values.size())
        throw DbException("Number of values does not match table columns");

    std::ostringstream cs, vs;
    for (size_t i = 0; i < cols.size(); ++i) {
        if (i) { cs << ","; vs << ","; }
        cs << cols[i];
        vs << "'" << values[i] << "'";
    }
    exec("INSERT INTO " + table + " (" + cs.str() +
        ") VALUES (" + vs.str() + ");");
}

std::vector<Record> Database::getRecords(const std::string& table) {
    std::vector<Record> result;
    auto cb = [](void* data, int argc, char** argv, char**) -> int {
        auto* res = static_cast<std::vector<Record>*>(data);
        Record r;
        r.id = argv[0] ? std::stoi(argv[0]) : 0;
        for (int i = 1; i < argc; ++i)
            r.fields.push_back(argv[i] ? argv[i] : "");
        res->push_back(r);
        return 0;
        };
    char* err = nullptr;
    int rc = sqlite3_exec(db_, ("SELECT * FROM " + table + ";").c_str(),
        cb, &result, &err);
    if (rc != SQLITE_OK) {
        std::string msg = err ? err : "unknown";
        sqlite3_free(err);
        throw DbException("getRecords failed: " + msg);
    }
    return result;
}

void Database::updateRecord(const std::string& table, int id,
    const std::string& col,
    const std::string& val) {
    exec("UPDATE " + table + " SET " + col + "='" + val +
        "' WHERE id=" + std::to_string(id) + ";");
}

void Database::deleteRecord(const std::string& table, int id) {
    exec("DELETE FROM " + table +
        " WHERE id=" + std::to_string(id) + ";");
}

void Database::dropTable(const std::string& tableName) {
    // Проверяем, существует ли таблица
    std::string checkSql = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + tableName + "';";
    bool exists = false;

    auto cb = [](void* data, int argc, char** argv, char**) -> int {
        if (argc > 0 && argv[0]) {
            *static_cast<bool*>(data) = true;
        }
        return 0;
        };

    char* err = nullptr;
    sqlite3_exec(db_, checkSql.c_str(), cb, &exists, &err);
    if (err) {
        sqlite3_free(err);
        throw DbException("Failed to check table existence");
    }

    if (!exists) {
        throw DbException("Table '" + tableName + "' does not exist");
    }

    // Удаляем таблицу
    std::string sql = "DROP TABLE IF EXISTS " + tableName + ";";
    exec(sql);
}

std::vector<std::string> Database::getTableNames() {
    std::vector<std::string> names;
    auto cb = [](void* d, int, char** argv, char**) -> int {
        static_cast<std::vector<std::string>*>(d)->push_back(argv[0]);
        return 0;
        };
    char* err = nullptr;
    // Исключаем системные таблицы SQLite
    int rc = sqlite3_exec(db_,
        "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%';",
        cb, &names, &err);
    if (rc != SQLITE_OK) {
        std::string msg = err ? err : "unknown";
        sqlite3_free(err);
        throw DbException("getTableNames failed: " + msg);
    }
    return names;
}