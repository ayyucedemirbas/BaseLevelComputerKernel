#ifndef STRING_H
#define STRING_H

#include "types.hpp"

struct string {
public:
    typedef char*             iterator;
    typedef const char*       const_iterator;

private:
    char* _data;
    size_t _size;
    size_t _capacity;

public:
    string(const char* s);
    explicit string(size_t capacity);


    string(const string& rhs);
    string& operator=(const string& rhs);


    string(string&& rhs);
    string& operator=(string&& rhs);


    ~string();

    size_t size() const;

    const char* c_str() const;

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;
};

#endif
