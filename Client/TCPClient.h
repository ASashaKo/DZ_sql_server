#pragma once
#include "../netcommon.h"
#include <map>
#include "Message.h"
#include "sha1.h"

#include "../utils.h"


class TCPClient{
public:
	TCPClient();
	TCPClient(const char*);
	~TCPClient();
	void start();
	void set_peer(const char* p) { _srv_name = p; };
	auto create_pwd_hash(const std::string&) -> std::string;
	auto set_db_id(const uint64_t& id) { _usr_db_id = id; };
private:
	int _socket{-1};	
	
	int _last_err_code{ 0 };
	char _buffer[sizeof(IOMSG)]{'\0'};
	size_t _buf_len{ sizeof(IOMSG) };

	std::string _srv_name{"localhost"};
	auto _init() -> void;
	auto _unpack_available_users(const std::string&, std::map<size_t, std::string>&) -> void;
	auto _send_to_server(IOMSG&) -> bool;
	uint64_t _usr_db_id{};

};

using TCPC = TCPClient;

