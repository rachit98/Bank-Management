#include <stdbool.h>
struct customer
{
	int account_no;
	char username1[1000];
	char password1[1000];
	char username2[1000];
	char password2[1000];
	bool type;
	bool status;
};