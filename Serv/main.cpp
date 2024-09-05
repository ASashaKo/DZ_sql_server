#include "TCPServer.h"
int main() {
	print_os_data();
	TSPS CS;
	if (!CS.init_ok())
		std::cout << BOLDRED << "\nFailed to run chat server\n" << RESET << std::endl;
	else 
		if (CS.process_clients());

	return 0;
}
