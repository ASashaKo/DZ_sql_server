#pragma once
#include <ctime>
#include "User.h"


class Message {
public:
    Message(const char* msg, const std::string& snd, const std::string& rcv) :body(msg), reciever(rcv), sender(snd) {};
    explicit Message(const char*);
    ~Message() = default;
    Message(const Message&) = delete;
    Message& operator=(const Message&) = delete;
    friend std::ostream& operator<<(std::ostream& out, const Message&);
	template<class S> void set_body(S& b) { body = b; }
	auto get_recv() -> std::string const { return reciever; };
	auto get_sender() -> std::string const { return sender; };
	auto get_ts() -> std::string const { return timestamp; };
	auto get_body() -> std::string const { return body; };
	auto serialize_msg() -> const std::string;
	void set_ts(const char* t) { timestamp = t; };
	void set_message_id(const char* id) { message_id = id; };

private:
	std::string timestamp;
	std::string message_id;
	std::string sender;
	std::string reciever;
	std::string body;

};
