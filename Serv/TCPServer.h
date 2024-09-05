#pragma once
#include "../netcommon.h"
#include "DBAccess.h"

#include "../utils.h"

class TCPServer{
public:
    TCPServer();
    ~TCPServer();
	auto process_clients()-> bool;
	auto init_ok()->bool const { return !_err_cnt;};

private:
	int _socket{-1};	
	std::string _name{ "localhost" };
	std::string _user,_pwd_hash;
	size_t _err_cnt{};
	uint64_t _usr_db_id{};
	std::shared_ptr<DBCTX>_hDB;
    std::unordered_map<size_t, std::shared_ptr<User>> _um_users;
    std::map<std::string, std::shared_ptr<Message>> _msg_pool;
	auto _send_to_client(int,IOMSG&)->bool;
	auto _process_client_msg(int,IOMSG&, bool&) -> bool;
	auto _login_used(const std::string&) -> bool;
	auto _is_valid_user_pwd(const std::string& pwd) -> bool;
	auto _db_init() -> bool;
	auto _hash_func(const std::string&) -> size_t;
};

using TSPS = TCPServer;


