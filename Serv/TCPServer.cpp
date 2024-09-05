#include "TCPServer.h"

TCPServer::TCPServer() {
	if(!_db_init()) _err_cnt++;
	if (!_err_cnt) {
		struct sockaddr_in svr_adress,_client_adress;
		_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (_socket < 0) {
			fprintf(stderr, "Could not create socket: %s\n", strerror(errno));
			_err_cnt++;
			exit(1);
		}

		const int enable = 1;
		socklen_t cl_length{ sizeof(_client_adress) };

		int iResult = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
		if (iResult < 0) {
			close(_socket);
			fprintf(stderr, "Could not set the socket option: %s\n", strerror(errno));
			_err_cnt++;
			return;
		}
		memset(&svr_adress, 0, sizeof(svr_adress));

		svr_adress.sin_family = AF_INET;
		svr_adress.sin_addr.s_addr = htonl(INADDR_ANY);
		svr_adress.sin_port = htons(PORT);

		if (bind(_socket, (struct sockaddr*)&svr_adress, sizeof(svr_adress)) < 0) {
			close(_socket);
			fprintf(stderr, "Could not bind the address: %s\n", strerror(errno));
			_err_cnt++;
			return;
		}
		if (listen(_socket, SOMAXCONN) < 0) {
			close(_socket);
			fprintf(stderr, "Could not set the backlog: %s\n", strerror(errno));
			_err_cnt++;
			return;
		}else {
			std::cout << "Server is listening...\n";
		}
	}
}

TCPServer::~TCPServer(){
	close(_socket);
}

auto TCPServer::_db_init() -> bool {
	try {
		_hDB = std::make_shared<DBCTX>();
	}
	catch (const std::bad_alloc& ex) {
		fprintf(stderr, "Failed to allocate memory for database context:%s\n", ex.what());
		return false;
	}
	catch (...) {
		fprintf(stderr, "Failed to init database\n");
		return false;
	}
	if (_hDB) {
		if (!_hDB->init_ok()) {
			fprintf(stderr, "DB not ready:%s\n", _hDB->get_last_error());
			return false;
		}
		_hDB->read_users(_um_users);
	}else
		return false;

	return true;

}

auto TCPServer::process_clients()->bool {
	if (_err_cnt)
		return false;

	size_t bytes_in{}, bytes_out{}, iRet{};
	size_t buf_len{ sizeof(IOMSG) };
	char rcv_buf[sizeof(IOMSG)]{'\0'};
	bool exit_loop{ false }, stop_server{false};
	IOMSG _msg;
	uint8_t _msg_id{};

	while (!stop_server)
	{
		int _cl_socket;
		struct sockaddr_in addr_c;
		char cl_ip[15]{(char)'/0'};
		exit_loop = false;
		unsigned int addrlen = sizeof(addr_c);
		_cl_socket = accept(_socket, (struct sockaddr*)&addr_c, &addrlen);
		if (_cl_socket < 0) {
			close(_socket);
			fprintf(stderr, "Could not accept the client: %s\n", strerror(errno));
			_err_cnt++;
			return false;
		}
		else {
			inet_ntop(AF_INET, &(addr_c.sin_addr), cl_ip, 15);
			std::cout << BOLDCYAN << "Incoming connection from client:"<< cl_ip << RESET << std::endl;
		}

		while (!exit_loop) {
			bytes_in = read(_cl_socket, rcv_buf, buf_len);
			if (bytes_in) {
				memcpy(&_msg, rcv_buf, buf_len);
				if(_process_client_msg(_cl_socket,_msg, exit_loop))
				_send_to_client(_cl_socket,_msg);
				if (exit_loop) {
					printf("Session terminated.\n");
					break;
				}
			}
		}
		close(_socket);
	}	
	return true;	
}

auto TCPServer::_send_to_client(int _cl_socket,IOMSG& msg)->bool {

	size_t bytes_out{};
	bytes_out = write(_cl_socket, reinterpret_cast<void*>(&msg), sizeof(IOMSG));
	if (!bytes_out) {
		close(_cl_socket);
		printf("Failed to send authorization request. Exiting..\n");
		exit(1);
	}
	return true;
}

auto TCPServer::_process_client_msg(int _cl_socket, IOMSG& in, bool& exit_loop) -> bool {

	bool existing_user{ false };
	switch (in.mtype) {
	case eAuth:
		in.mtype = eWelcome;
		memset(in.body, '\0', MSG_LENGTH);
		strcpy(in.body,"Welcome to chat session. New user?(y/n, x - exit):\n");
		break;
	case eNewUser:
		in.mtype = eChooseLogin;
		strcpy(in.body, "Choose your login(x - exit):\n");
		break;
	case eExistingUser:
		in.mtype = eLogin;
		strcpy(in.body, "Type your login(x - exit):\n");
		break;
	case eLogin:
		existing_user = *in.user == 'e';
		if (existing_user) {
			if (_login_used(in.body)) {
				_user = in.body;
				in.mtype = ePassword;
				strcpy(in.body, "Type your password(x - exit):\n");
			}else {
				in.mtype = eWrongLogin;
				strcpy(in.body, "Wrong login. Try again(x - exit):\n");
			}
		}else{
			if (_login_used(in.body)) {
				strcpy(in.body, "Login provided is already used. Choose another(x - exit):\n");
				in.mtype = eWrongLogin;
			}else {
				_user = in.body;
				in.mtype = eChoosePassword;
				strcpy(in.body, "Choose your password(x - exit):\n");
			}
		}
		break;
	case ePassword:
		existing_user = *in.user == 'e';
		if (existing_user) {
			std::string name, email, pwd_hash;
			uint64_t id;
			if (_hDB->get_user(_user.c_str(),id, name, email, pwd_hash)) {
				if (!strcmp(pwd_hash.c_str(), in.body)) {
                    auto uptr = std::make_shared<User>(_user.c_str(), in.body);
					uptr->set_id(id);
					_usr_db_id = id;
					_um_users.emplace(std::make_pair(_hash_func(_user), uptr));
					clear_message(in);
					
					in.mtype = eAuthOK;
					strcpy(in.body, "Login successful. (x - exit):\n");
					strcpy(in.user, std::to_string(_usr_db_id).c_str());
				}else {
					in.mtype = eWrongPassword;
					strcpy(in.body, "Invalid password. Type again(x - exit):\n");
				}
			}else{
				clear_message(in);
				in.mtype = eQuit;
				strcpy(in.body, "Server failed to reconfirm user existence:\n");

			}
		}else{
			if (*in.body == '\0') {
				in.mtype = eWrongPassword;
				strcpy(in.body, "Empty password not permited. Type one(x - exit):\n");
			}else {
				uint64_t id;
				if (_hDB->add_user(_user.c_str(), id)) {
					if (_hDB->set_user_pwdhash(id, in.body)){
                        auto uptr = std::make_shared<User>(_user.c_str(), in.body);
						uptr->set_id(id);
						_usr_db_id = id;
						_um_users.emplace(std::make_pair(_hash_func(_user), uptr));
						clear_message(in);
						in.mtype = eAuthOK;
						strcpy(in.body, "Registration successful.\n");
						strcpy(in.user, std::to_string(_usr_db_id).c_str());
					}else {
						clear_message(in);
						in.mtype = eQuit;
						strcpy(in.body, "Failed to save user pwd:");
						strcat(in.body,_hDB->get_last_error());
						strcat(in.body, "\n");
					}

				}else {
					in.mtype = eQuit;
					memset(in.body, '\0', MSG_LENGTH);
					strcpy(in.body, "Failed to create user record in database!\n");
				}
			}
		}
		break;
	case eGetUserMsg:
		_hDB->get_user_msgs(_usr_db_id, _user, _msg_pool);
		if (_msg_pool.empty()) {
			clear_message(in);
			in.mtype = eNoMsg;
			strcpy(in.body, "You have no new messages.\n");
		}else {
			clear_message(in);
			auto curr_msg = _msg_pool.begin();
			in.mtype = eMsgNext;
			strcpy(in.body, curr_msg->second->serialize_msg().c_str());
			strcpy(in.user, "You have new messages:\n");
			_msg_pool.erase(curr_msg);
		}
		break;
	case eGetNextMsg:
		if (in.body) {
			//here body should contain a message id to be marked as viewed
			if (_hDB->set_msg_state(in.body, "2")) {
			}
		}

		if (_msg_pool.empty()) {
			clear_message(in);
			in.mtype = eNoMsg;
		}else {
			clear_message(in);
			auto curr_msg = _msg_pool.begin();
			in.mtype = eMsgNext;
			strcpy(in.body, curr_msg->second->serialize_msg().c_str());
			_msg_pool.erase(curr_msg);
		}
		break;

	case eGetMainMenu:
		clear_message(in);
		in.mtype = eMsgMainMenu;
		strcpy(in.body, "Choose action : \n\twrite to user(u)\n\twrite to all(a)\n\tLog out(l)\n\tQuit(x) :\n");
		break;
	case eLogOut:
		_user.clear();
		_pwd_hash.clear();
		_usr_db_id = 0;
		in.mtype = eLogin;
		strcpy(in.body, "Type your login(x - exit):\n");
		break;

	case eSendToAll: 
	{
		auto msg_sent = _hDB->deliver_msg(in.body, in.user);
		clear_message(in);
		in.mtype = eMsgDelivered;
		if (msg_sent) {
			strcpy(in.body, "messages sent OK\n");
		}
		else
			strcpy(in.body, "Failed to send messages\n");
		break;
	}
	case eSendToUser:
	{
		std::string available_users;
		if (!_hDB->pack_users(in.user, available_users)) {
			clear_message(in);
			in.mtype = eQuit;
			strcpy(in.body, "Sorry, no users available.\n");
		}
		else {
			in.mtype = eAvailableUsers;
			strcpy(in.body, available_users.c_str());
		}
		break;
	}
	case eSendToUserMsgReady:
	{
		if (_hDB->deliver_msg(in.body, std::to_string(_usr_db_id).c_str(), in.user)) {
			clear_message(in);
			in.mtype = eMsgDelivered;
			strcpy(in.body, "The message sent\n");
		}else {
			clear_message(in);
			in.mtype = eErrMsgNotDelivered;
			strcpy(in.body, "Failed to send message\n");
		}
		break;
	}
	case eQuit:
		exit_loop = true;
		break;
	}

	return true;
}

auto TCPServer::_login_used(const std::string& uname) -> bool {
	if (uname.size() && _hDB)
		return _hDB->login_used(uname.c_str());
	else
		return false;
}

auto TCPServer::_is_valid_user_pwd(const std::string& pwd) -> bool {
	auto found = _um_users.find(_hash_func(_user));
	if (found != _um_users.end()) {
		if (!found->second->get_pwd_hash().size()) {
			return _hDB->user_pwdh_ok(_user.c_str(), pwd);
		}else
			return found->second->get_pwd_hash()==pwd;
	}
	return false;
}

auto TCPServer::_hash_func(const std::string& user_name)->size_t {
	return cHasher{}(user_name);
}

