#include <Client.h>

int main()
{
	Client user;
	user.Init();
	user.TryConnect();
	user.Run();
	return EXIT_SUCCESS;
}