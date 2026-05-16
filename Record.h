#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <ostream>

struct Record {
    int id = 0;
    std::vector<std::string> fields;

    std::string& operator[](size_t i) {
        if (i >= fields.size()) throw std::out_of_range("Field index out of range");
        return fields[i];
    }

    bool operator==(const Record& other) const {
        return id == other.id;
    }

    friend std::ostream& operator<<(std::ostream& os, const Record& r) {
        os << "ID=" << r.id;
        for (size_t i = 0; i < r.fields.size(); ++i)
            os << " | " << r.fields[i];
        return os;
    }
};
