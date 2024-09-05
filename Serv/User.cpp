#include "User.h"
#pragma warning(disable : 4996)

User::User(const char* nm, const char* pwdh):_name(nm), _pwd_hash(pwdh){

}

User::User(const User& rhs){
	_name = rhs._name;
	_pwd_hash == rhs._pwd_hash;
}

User& User::operator=(const User& rhs){
	if(*this==rhs)
		return *this;
	_name = rhs._name;
	_pwd_hash == rhs._pwd_hash;
	return *this;
}

bool User::operator==(const User& rhs) {
	return _name == rhs.get_name() && _pwd_hash==rhs._pwd_hash;
}

bool User::operator!=(const User& rhs) {
	return _name != rhs.get_name();
}

std::ostream& operator<<(std::ostream& out, const User& usr) {
	out << "Name:" << usr._name << " Pwd hash:" << usr._pwd_hash << std::endl;
	return out;
}
