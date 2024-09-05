#include "TCPClient.h"

TCPClient::TCPClient() {
	_init();
}

TCPClient::TCPClient(const char* peer) :_srv_name(peer) {
	_init();
}

TCPClient::~TCPClient(){
	close(_socket);
}

auto TCPClient::_init() -> void {
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket < 0) {
		printf("failed to create socket with error: %s\n", strerror(errno));
		_last_err_code = errno;
		return;
	}
	struct sockaddr_in svr_adress {};
	int conn{ -1 };
	memset(&svr_adress, 0, sizeof(svr_adress));
	svr_adress.sin_family = AF_INET;
	svr_adress.sin_addr.s_addr = inet_addr(_srv_name.c_str());
	svr_adress.sin_port = htons(PORT);

	conn = connect(_socket, (struct sockaddr*)&svr_adress, sizeof(svr_adress));
	if (conn < 0) {
		close(_socket);
		printf("failed to connect to server with error: %s\n", strerror(errno));
		_last_err_code = errno;
		return;
	}
}

void TCPClient::start()
{
	if (!_last_err_code) {
		size_t bytes_in{}, bytes_out{}, iRet{};
		bool exit_loop{ false }, server_down{false};
		std::string input;
		IOMSG _msg;
		uint8_t _msg_id{};

		if (!_send_to_server(_msg)) {
			return;
		}

		while (!exit_loop)
		{
			bytes_in = read(_socket, _buffer, _buf_len);
			if (bytes_in > 0) {
				memcpy(&_msg, _buffer, _buf_len);
				_msg_id = static_cast<uint8_t>(_msg.mtype);
				switch (_msg_id) {
				case eWelcome:
					std::cout << _msg.body;
					std::getline(std::cin, input);
					if (input.size() == 1 && input.at(0) == 'x') {
						_msg.mtype = eQuit;
						strcpy(_msg.body, "Good buy\n");
					}else {
						clear_message(_msg);
						if (input.size() == 1 && input.at(0) == 'y')
							_msg.mtype = eNewUser;
						else
							_msg.mtype = eExistingUser;
					}
					break;
				case eChooseLogin:
					std::cout << _msg.body;
					std::getline(std::cin, input);
					if (input.size() == 1 && input.at(0) == 'x') {
						_msg.mtype = eQuit;
						strcpy(_msg.body, "Good buy\n");
					}else{
						clear_message(_msg);
						_msg.mtype = eLogin;
						strcpy(_msg.body, input.c_str());
						strcpy(_msg.user, "n"); //new user
					}
					break;
				case eLogin:
					std::cout << _msg.body;
					std::getline(std::cin, input);
					if (input.size() == 1 && input.at(0) == 'x') {
						_msg.mtype = eQuit;
						strcpy(_msg.body, "Good buy\n");
					}
					else {  
						clear_message(_msg);
						_msg.mtype = eLogin;
						strcpy(_msg.body, input.c_str());
						strcpy(_msg.user, "e"); //existing user
					}
					break;

				case eChoosePassword:
					std::cout << _msg.body;
					std::getline(std::cin, input);
					if (input.size() == 1 && input.at(0) == 'x') {
						_msg.mtype = eQuit;
						strcpy(_msg.body, "Good buy\n");
					}
					else {
						clear_message(_msg);
						_msg.mtype = ePassword;
						strcpy(_msg.body, create_pwd_hash(input).c_str());
					}

					break;
				case ePassword:
					std::cout << _msg.body;
					std::getline(std::cin, input);
					if (input.size() == 1 && input.at(0) == 'x') {
						_msg.mtype = eQuit;
						strcpy(_msg.body, "Good buy\n");
					}
					else {
						clear_message(_msg); 
						_msg.mtype = ePassword;
						strcpy(_msg.body, create_pwd_hash(input).c_str());
						strcpy(_msg.user, "e"); //existing user
					}
					break;

				case eWrongLogin:
					std::cout << _msg.body;
					std::getline(std::cin, input);
					if (input.size() == 1 && input.at(0) == 'x') {
						_msg.mtype = eQuit;
						strcpy(_msg.body, "Good buy\n");
					}
					else {						
						if (*_msg.user == 'n')
							_msg.mtype = eChooseLogin;
						else {
							_msg.mtype = eLogin;
							strcpy(_msg.user, "e"); //existing user
						}
						strcpy(_msg.body, input.c_str());						
					}
					break;
				case eWrongPassword:
					std::cout << _msg.body;
					std::getline(std::cin, input);
					if (input.size() == 1 && input.at(0) == 'x') {
						_msg.mtype = eQuit;
						strcpy(_msg.body, "Good buy\n");
					}
					else {
						clear_message(_msg);
						_msg.mtype = ePassword;
						strcpy(_msg.body, create_pwd_hash(input).c_str());
						strcpy(_msg.user, "e"); //existing user
					}

					break;
				case eAuthOK:
					std::cout << _msg.body;
					if (_msg.user) {
						try {
							_usr_db_id = std::stol(_msg.user);
						}catch (const std::invalid_argument& ex) {
							std::cout << "Bad user id sent by server - " << _msg.user << ": " << ex.what() << std::endl;
						}
					}
					clear_message(_msg);
					_msg.mtype = eGetUserMsg;
					break;
				case eNoMsg:
					if(_msg.body) std::cout << _msg.body;
					clear_message(_msg);
					_msg.mtype = eGetMainMenu;
					break;
				case eMsgNext:
				{
					if (_msg.user) std::cout << _msg.user;
					std::string message_id;
                    Message::show_unpacked(_msg.body, message_id);
					clear_message(_msg);
					_msg.mtype = eGetNextMsg;
					strcpy(_msg.body, message_id.c_str()); //this message must be marked as viewed by server
					break;
				}
				case eMsgMainMenu:
					std::cout << _msg.body << std::endl;
					clear_message(_msg);
					std::getline(std::cin, input);
					if (input.size() == 1 && input.at(0) == 'x') {
						_msg.mtype = eQuit;
						strcpy(_msg.body, "Good buy\n");
					}else {
						if (input.at(0) == 'l') {
							_msg.mtype = eLogOut;
						}else if (input.at(0) == 'u') { //write to user
							clear_message(_msg);
							_msg.mtype = eSendToUser;
							strcpy(_msg.user,std::to_string(_usr_db_id).c_str());
						}else if (input.at(0) == 'a') { //write to all
							std::cout << "Type your message (x - exit):\n";
							std::getline(std::cin, input);
							if (input.size() == 1 && input.at(0) == 'x') {
								_msg.mtype = eQuit;
								strcpy(_msg.body, "Good buy\n");
							}else{
								clear_message(_msg);
								_msg.mtype = eSendToAll;
								strcpy(_msg.body, input.c_str());
								strcpy(_msg.user, std::to_string(_usr_db_id).c_str());
							}
						}else {
							_msg.mtype = eNone;
							strcpy(_msg.body, "Undefined choice\n");
						}
					}
					break;
				case eMsgDelivered:
					std::cout << BOLDCYAN << _msg.body << RESET << std::endl;
					clear_message(_msg);
					_msg.mtype = eGetMainMenu;

					break;
				case eErrMsgNotDelivered:
					if (_msg.body)
						std::cout << BOLDCYAN << _msg.body << RESET << std::endl;
					clear_message(_msg);
					_msg.mtype = eGetMainMenu;
					
					break;
				case eAvailableUsers:
				{
					std::map<size_t, std::string> usrs;
					std::cout << "Available users:\n";
					_unpack_available_users(_msg.body, usrs);
				USER_ID_INPUT:
					std::cout << "Type user id (in brackets) (x - exit):\n";
					std::getline(std::cin, input);
					while (!input.size()) {
						std::cout << BOLDRED << "You`ve typed nothing." << RESET << std::endl;
						std::cout << "Type user id (in brackets) (x - exit):\n";
						std::getline(std::cin, input);
					}

					if (input.at(0) == 'x') {
						_msg.mtype = eQuit;
						strcpy(_msg.body, "Good buy\n");
					}
					else {
						size_t uid{};
						try {
							uid = std::stol(input);
						}
						catch (std::invalid_argument const& ex) {
							std::cout << BOLDRED << "You`ve mistyped user id:" << ex.what() << RESET << std::endl;
							goto USER_ID_INPUT;
						}

						auto it = usrs.find(uid);
						if (it != usrs.end())
							std::cout << "Type your message to user " << it->second << " (x - exit):" << std::endl;
						else {
							std::cout << "You`ve mistyped user id - No such id\n";
							goto USER_ID_INPUT;
						}
						std::getline(std::cin, input);
						while (!input.size()) {
							std::cout << BOLDRED << "You`ve typed nothing." << RESET << std::endl;
							std::cout << "Type your message to user " << it->second << " (x - exit):" << std::endl;
							std::getline(std::cin, input);
						}
						if (input.at(0) == 'x') {
							_msg.mtype = eQuit;
							strcpy(_msg.body, "Good buy\n");
						}
						else {
							clear_message(_msg);
							_msg.mtype = eSendToUserMsgReady;
							strcpy(_msg.body, input.c_str());
							strcpy(_msg.user, std::to_string(uid).c_str());
						}
					}
					break;
				}
				case eQuit:
					if(_msg.body)
						std::cout << _msg.body  <<"Connection closed by server" << std::endl;

					exit_loop = true;
					server_down = true;
					break;
				}

				if (!_send_to_server(_msg))
					break;

			}else if (bytes_in == 0) {
				printf("Connection closing...\n");
				close(_socket);
				return;
			}else {
				printf("recv failed: %s\n", strerror(errno));			
				close(_socket); 
				return;
            }
            sleep(1);
		}
	}else
		std::cout << "\nClient init failure\n";
	
}

auto TCPClient::create_pwd_hash(const std::string& pwd) -> std::string {
	if (!pwd.size())
		return "";
	uint* digest = sha1((char*)pwd.c_str(), pwd.size());

	if (digest)
		return hash_to_string(digest);
	else
		return "";
}

auto TCPClient::_unpack_available_users(const std::string& msg_body, std::map<size_t, std::string>& usrs) -> void {
	std::string packed_usrs = msg_body;
	auto pos_b = packed_usrs.find("|");
	while (pos_b != std::string::npos) {
		auto pos_cr = packed_usrs.find("#");
		if (pos_cr != std::string::npos) {
			size_t uid = std::stol(packed_usrs.substr(0, pos_b));
			usrs.insert(std::make_pair(uid, packed_usrs.substr(pos_b + 1, pos_cr - (pos_b + 1))));
		}
		packed_usrs = packed_usrs.substr(pos_cr + 1);
		pos_b = packed_usrs.find("|");
	}
	for (const auto& it : usrs)
		std::cout << "[" << it.first << "] " << it.second << std::endl;

}

auto TCPClient::_send_to_server(IOMSG& msg) -> bool {
	size_t bytes_out{};
	bytes_out = write(_socket, reinterpret_cast<void*>(&msg), sizeof(IOMSG));
	if (!bytes_out) {
		close(_socket);
		printf("Failed to send authorization request. Exiting..\n");
		exit(1);
	}
	return true;
}

