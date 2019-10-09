#include <Client.h>

int main()
{
	Client user;
	user.Init();
	user.TryConnect("127.0.0.1", 76475);
	user.Run();
	return EXIT_SUCCESS;
}