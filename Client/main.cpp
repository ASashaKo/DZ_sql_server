#include "TCPClient.h"

int main(int argc, char** argv) {
    std::string Name_or_IP;
    std::cout << "Enter server name/ip adress" << std::endl;
    std::getline(std::cin >> std::ws, Name_or_IP);

    argv[1] = Name_or_IP.data();
    argc = 2;
    if (argc < 2) {
		std::string path{ argv[0] };
		auto const pos = path.find_last_of('/');
		std::cout << "Usage: " << path.substr(pos + 1) << " [server name/ip adress]" << std::endl;
        return 0;
	}else {
		TCPC CS(argv[1]);
		CS.start();
	}

	return 0;
}
