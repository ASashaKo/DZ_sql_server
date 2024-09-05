#pragma once
#include <iostream>
#include <string>
#define uint64_t __int64_t

class User {
public:
    User() = delete;
    explicit User(const char*, const char*);
    User(const User&);
    ~User() = default;
    User& operator=(const User&);
	template<class S> void set_name(const S& nm) { _name = nm; };
	template<class S> void set_pwd(const S& p) { _pwd_hash = p; };
	std::string get_name() const { return _name; };
	auto get_pwd_hash() -> std::string const { return _pwd_hash; };
    auto get_id() -> uint64_t const { return _id; };

	auto set_new_one(bool p) { _new_one = p; };
	auto set_id(uint64_t p) { _id = p; };
    bool operator==(const User& rhs);
    bool operator!=(const User& rhs);
    friend std::ostream& operator<<(std::ostream& out, const User&);
private:
	uint64_t _id;
	std::string _name;
	std::string _pwd_hash;
	bool _new_one{ false };
};
