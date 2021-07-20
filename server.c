#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include "account.c"
#include "customer.c"
#include "transaction.c"

bool check_user(int id,char username[],char password[])
{
	int customer_fd = open("customer_db", O_RDONLY);
	off_t offset = lseek(customer_fd, (id-1)*sizeof(struct customer), SEEK_SET);

	struct flock lock = {F_RDLCK, SEEK_SET, (id-1)*sizeof(struct customer), sizeof(struct customer), getpid()};
	int lockingStatus = fcntl(customer_fd, F_SETLKW, &lock);
	ssize_t readBytes, writeBytes;
	struct customer cust;
	readBytes = read(customer_fd, &cust, sizeof(struct customer));
	//printf("%d",cust.status);

	if(cust.status==false)
		return false;
	if(cust.type==0)
	{
		if((strcmp(username,cust.username1)==0)&&(strcmp(password,cust.password1)==0))
			return true;
	}
	else
	{
		if((strcmp(username,cust.username1)==0)&&(strcmp(password,cust.password1)==0))
			return true;
		if((strcmp(username,cust.username2)==0)&&(strcmp(password,cust.password2)==0))
			return true;
	}

	lock.l_type = F_UNLCK;
    lockingStatus = fcntl(customer_fd, F_SETLK, &lock);
    close(customer_fd);

	return false;
}

int add_user(int conn_fd)
{
	ssize_t readBytes, writeBytes;
    char data_from_client[1000];

    struct account new_account, last_account;
    struct customer cust;
    int account_fd = open("ACCOUNT_DB.txt", O_RDONLY);
    if ( account_fd == -1 && errno == ENOENT)
    {
        new_account.account_no = 1;
        cust.account_no = 1;
    }
    else
    {
        int offset = lseek(account_fd, -sizeof(struct account), SEEK_END);
        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct account), getpid()};
        int fcntlStatus = fcntl(account_fd, F_SETLKW, &lock);
        readBytes = read(account_fd, &last_account, sizeof(struct account));
        lock.l_type = F_UNLCK;
        fcntlStatus = fcntl(account_fd, F_SETLKW, &lock);
        close(account_fd);
        new_account.account_no = last_account.account_no + 1;
        cust.account_no = last_account.account_no + 1;
    }
    writeBytes=write(conn_fd,"RW",sizeof("RW"));
    writeBytes=write(conn_fd,"Enter account type, J for joint and S for single \n",sizeof("Enter account type, J for joint and S for single \n"));
    readBytes=read(conn_fd,data_from_client,sizeof(data_from_client));
    if(strcmp(data_from_client,"J")==0)
    {
    	new_account.type=1;
    	cust.type=1;
    	new_account.status=true;
    	cust.status=true;
    	strcpy(data_from_client,"");
    	writeBytes=write(conn_fd,"RW",sizeof("RW"));
    	writeBytes=write(conn_fd,"Enter username 1 \n",sizeof("Enter username 1 \n"));
    	readBytes=read(conn_fd,data_from_client,sizeof(data_from_client));
    	strcpy(cust.username1,data_from_client);
    	strcpy(data_from_client,"");

    	writeBytes=write(conn_fd,"RW",sizeof("RW"));
    	writeBytes=write(conn_fd,"Enter password for username 1 \n",sizeof("Enter password for username 1 \n"));
    	readBytes=read(conn_fd,data_from_client,sizeof(data_from_client));
    	strcpy(cust.password1,data_from_client);
    	strcpy(data_from_client,"");

    	writeBytes=write(conn_fd,"RW",sizeof("RW"));
    	writeBytes=write(conn_fd,"Enter username 2 \n",sizeof("Enter username 2 \n"));
    	readBytes=read(conn_fd,data_from_client,sizeof(data_from_client));
    	strcpy(cust.username2,data_from_client);
    	strcpy(data_from_client,"");

    	writeBytes=write(conn_fd,"RW",sizeof("RW"));
    	writeBytes=write(conn_fd,"Enter password for username 2 \n",sizeof("Enter password for username 2 \n"));
    	readBytes=read(conn_fd,data_from_client,sizeof(data_from_client));
    	strcpy(cust.password2,data_from_client);
    	strcpy(data_from_client,"");

    	new_account.balance=0;
    }

    if(strcmp(data_from_client,"S")==0)
    {
    	new_account.type=0;
    	cust.type=0;
    	new_account.status=true;
    	cust.status=true;
    	strcpy(data_from_client,"");
    	writeBytes=write(conn_fd,"RW",sizeof("RW"));
    	writeBytes=write(conn_fd,"Enter username 1 \n",sizeof("Enter username 1 \n"));
    	readBytes=read(conn_fd,data_from_client,sizeof(data_from_client));
    	strcpy(cust.username1,data_from_client);
    	strcpy(data_from_client,"");

    	writeBytes=write(conn_fd,"RW",sizeof("RW"));
    	writeBytes=write(conn_fd,"Enter password for username 1 \n",sizeof("Enter password for username 1 \n"));
    	readBytes=read(conn_fd,data_from_client,sizeof(data_from_client));
    	strcpy(cust.password1,data_from_client);
    	strcpy(data_from_client,"");

    	//writeBytes=(conn_fd,"RW",sizeof("RW"));
    	//writeBytes=(conn_fd,"Enter username 2 \n",sizeof("Enter username 2 \n"));
    	//readBytes=(conn_fd,data_from_client,sizeof(data_from_client));
    	strcpy(cust.username2,"fake");
    	strcpy(data_from_client,"");

    	//writeBytes=(conn_fd,"RW",sizeof("RW"));
    	//writeBytes=(conn_fd,"Enter password for username 2 \n",sizeof("Enter password for username 2 \n"));
    	//readBytes=(conn_fd,data_from_client,sizeof(data_from_client));
    	strcpy(cust.password2,"news");
    	strcpy(data_from_client,"");

    	new_account.balance=0;
    }

    account_fd = open("ACCOUNT_DB.txt", O_CREAT | O_APPEND | O_WRONLY, S_IRWXU);
    writeBytes = write(account_fd, &new_account, sizeof(struct account));
    close(account_fd);

    // Creating the customer/account file as account_no.txt
    char fname[50];
    sprintf(fname, "%d",new_account.account_no);
    int trans_fd= open(fname, O_CREAT | O_WRONLY, S_IRWXU);
    struct transaction temp;
    time_t systime;
    time(&systime);
    strcpy(temp.date,ctime(&systime));
    temp.activity='c'; // c means created d means deposit w means withdraw
    temp.account_no=new_account.account_no;
    temp.amount=0.0;
    temp.balance=0.0;

    writeBytes = write(trans_fd, &temp, sizeof(temp));
    close(trans_fd);

    // Adding User in customer database
    //struct customer cust;
    int cust_fd = open("customer_db", O_WRONLY|O_APPEND);
    writeBytes = write(cust_fd,&cust,sizeof(cust));
    close(cust_fd);

    //writeBytes = write(conn_fd, "RD",sizeof("RD"));
	// readBytes = read(conn_fd,data_from_client,sizeof(data_from_client));
	// strcpy(data_from_client,"");
	// write(conn_fd,cust.account_no,sizeof(int));
    // Printing account is created with neccessary credentials
	return 1;

}

bool del_user(int id)
{
	ssize_t readBytes,writeBytes;
	int customer_fd = open("customer_db", O_RDWR);
	off_t offset = lseek(customer_fd, (id-1)*sizeof(struct customer), SEEK_SET);

	struct flock lock = {F_RDLCK, SEEK_SET, (id-1)*sizeof(struct customer), sizeof(struct customer), getpid()};
	int lockingStatus = fcntl(customer_fd, F_SETLKW, &lock);

	struct customer cust;
	readBytes = read(customer_fd, &cust, sizeof(struct customer));

	cust.status = false;
	offset = lseek(customer_fd, (id-1)*sizeof(struct customer), SEEK_SET);
	writeBytes = write(customer_fd, &cust,sizeof(struct customer));

	//write back the struture???????

	lock.l_type = F_UNLCK;
    lockingStatus = fcntl(customer_fd, F_SETLK, &lock);
	close(customer_fd);

	int account_fd = open("ACCOUNT_DB.txt", O_RDWR);
	off_t offset2 = lseek(account_fd, (id-1)*sizeof(struct account), SEEK_SET);

	struct flock lock2 = {F_RDLCK, SEEK_SET, (id-1)*sizeof(struct account), sizeof(struct account), getpid()};
	int lockingStatus2 = fcntl(account_fd, F_SETLKW, &lock2);

	struct account acc;
	readBytes = read(account_fd, &acc, sizeof(struct account));

	acc.status = false;
	offset2 = lseek(account_fd, (id-1)*sizeof(struct account), SEEK_SET);
	writeBytes = write(account_fd, &acc,sizeof(struct account));
	//write back the structure????
	lock2.l_type = F_UNLCK;
    lockingStatus2 = fcntl(account_fd, F_SETLK, &lock);
	close(account_fd);
	return true;
}

bool modify_account(int conn_fd)
{
	ssize_t wb,rb;
	char data_from_client[1000];
	wb = write(conn_fd, "RW",sizeof("RW"));
	strcpy(data_from_client,"");
	wb = write(conn_fd, "Enter account no to be modified\n", sizeof("Enter account no to be modified\n"));
	rb = read(conn_fd,data_from_client,sizeof(data_from_client));
	int ac_id= atoi(data_from_client);
	int account_fd = open("ACCOUNT_DB.txt", O_RDWR);
	off_t offset = lseek(account_fd, (ac_id-1)*sizeof(struct account), SEEK_SET);
	struct flock lock = {F_RDLCK, SEEK_SET, (ac_id-1)*sizeof(struct account), sizeof(struct account), getpid()};
	int lockingStatus = fcntl(account_fd, F_SETLKW, &lock);
	struct account acc;
	rb = read(account_fd, &acc, sizeof(struct account));

	int customer_fd = open("customer_db", O_RDWR);
	off_t offset2 = lseek(customer_fd, (ac_id-1)*sizeof(struct customer), SEEK_SET);

	struct flock lock2 = {F_RDLCK, SEEK_SET, (ac_id-1)*sizeof(struct customer), sizeof(struct customer), getpid()};
	int lockingStatus2 = fcntl(customer_fd, F_SETLKW, &lock);

	struct customer cust;
	rb = read(customer_fd, &cust, sizeof(struct customer));

	if(acc.type==1)
	{
		wb = write(conn_fd, "RW",sizeof("RW"));
		strcpy(data_from_client,"");
		wb = write(conn_fd, "You have a joined account, press 1 to delete 2nd user\n 2 to delte first user and anything else to cancel modification\n", sizeof("You have a joined account, press 1 to delete 2nd user\n 2 to delte first user and anything else to cancel modification\n"));
		rb = read(conn_fd,data_from_client,sizeof(data_from_client));
		int ans= atoi(data_from_client);
		if(ans==1)
		{
			acc.type=0;
			cust.type=0;
		}
		else if(ans==2)
		{
			strcpy(cust.username1,cust.username2);
			strcpy(cust.password1,cust.password2);
			acc.type=0;
			cust.type=0;
		}
		else
			return true;

		offset = lseek(account_fd, (ac_id-1)*sizeof(struct account), SEEK_SET);
		wb = write(account_fd, &acc,sizeof(struct account));
		lock.l_type = F_UNLCK;
    	lockingStatus = fcntl(account_fd, F_SETLK, &lock);
		close(account_fd);

		offset2 = lseek(customer_fd, (ac_id-1)*sizeof(struct customer), SEEK_SET);
		wb = write(customer_fd, &cust,sizeof(struct customer));
		lock2.l_type = F_UNLCK;
    	lockingStatus2 = fcntl(customer_fd, F_SETLK, &lock);
		close(customer_fd);

	}
	else
	{
		wb = write(conn_fd, "RW",sizeof("RW"));
		strcpy(data_from_client,"");
		wb = write(conn_fd, "You have a single account, press 1 to add 2nd user\n anything else to cancel modification\n", sizeof("You have a single account, press 1 to add 2nd user\n anything else to cancel modification\n"));
		rb = read(conn_fd,data_from_client,sizeof(data_from_client));
		int ans= atoi(data_from_client);
		if(ans!=1)
			return true;
		else
		{
			wb=write(conn_fd,"RW",sizeof("RW"));
    		wb=write(conn_fd,"Enter username 2 \n",sizeof("Enter username 2 \n"));
    		rb=read(conn_fd,data_from_client,sizeof(data_from_client));
    		strcpy(cust.username2,data_from_client);
    		strcpy(data_from_client,"");

    		wb=write(conn_fd,"RW",sizeof("RW"));
    		wb=write(conn_fd,"Enter password for username 2 \n",sizeof("Enter password for username 2 \n"));
    		rb=read(conn_fd,data_from_client,sizeof(data_from_client));
    		strcpy(cust.password2,data_from_client);
    		strcpy(data_from_client,"");
    		cust.type=1;
    		acc.type=1;
		}
		offset = lseek(account_fd, (ac_id-1)*sizeof(struct account), SEEK_SET);
		wb = write(account_fd, &acc,sizeof(struct account));
		lock.l_type = F_UNLCK;
    	lockingStatus = fcntl(account_fd, F_SETLK, &lock);
		close(account_fd);

		offset2 = lseek(customer_fd, (ac_id-1)*sizeof(struct customer), SEEK_SET);
		wb = write(customer_fd, &cust,sizeof(struct customer));
		lock2.l_type = F_UNLCK;
    	lockingStatus2 = fcntl(customer_fd, F_SETLK, &lock);
		close(customer_fd);
	}
	return true;
}

bool search_account(int conn_fd)
{
	ssize_t wb,rb;
	char data_from_client[1000];
	wb = write(conn_fd, "RW",sizeof("RW"));
	strcpy(data_from_client,"");
	wb = write(conn_fd, "Enter account no to be searched\n", sizeof("Enter account no to be searched\n"));
	rb = read(conn_fd,data_from_client,sizeof(data_from_client));
	int ac_id= atoi(data_from_client);
	int account_fd = open("ACCOUNT_DB.txt", O_RDONLY);
	off_t offset = lseek(account_fd, (ac_id-1)*sizeof(struct account), SEEK_SET);
	struct flock lock = {F_RDLCK, SEEK_SET, (ac_id-1)*sizeof(struct account), sizeof(struct account), getpid()};
	int lockingStatus = fcntl(account_fd, F_SETLKW, &lock);
	struct account acc;
	rb = read(account_fd, &acc, sizeof(struct account));

	// wb = write(conn_fd,"RD",sizeof("RD"));
	// rb = read(conn_fd,data_from_client,sizeof(data_from_client));
	// strcpy(data_from_client,"");
	// wb = (conn_fd,"Account No:",sizeof("Account No:"));
	wb = write(conn_fd, "RD",sizeof("RD"));
	rb = read(conn_fd,data_from_client,sizeof(data_from_client));
	strcpy(data_from_client,"");
	char ac_no[1000];
	sprintf(ac_no, "%d", ac_id);
	wb = write(conn_fd,ac_no,sizeof(ac_no));

	// wb = write(conn_fd, "RD",sizeof("RD"));
	// rb = read(conn_fd,data_from_client,sizeof(data_from_client));
	// strcpy(data_from_client,"");
	// wb = (conn_fd,"Balance",sizeof("Balance"));
	wb = write(conn_fd, "RD",sizeof("RD"));
	rb = read(conn_fd,data_from_client,sizeof(data_from_client));
	strcpy(data_from_client,"");
	strcpy(ac_no,"");
	sprintf(ac_no,"%f",acc.balance);
	write(conn_fd,ac_no,sizeof(ac_no));
	strcpy(ac_no,"");
	lock.l_type = F_UNLCK;
    lockingStatus = fcntl(account_fd, F_SETLK, &lock);
	close(account_fd);

	int customer_fd = open("customer_db", O_RDONLY);
	off_t offset2 = lseek(customer_fd, (ac_id-1)*sizeof(struct customer), SEEK_SET);

	struct flock lock2 = {F_RDLCK, SEEK_SET, (ac_id-1)*sizeof(struct customer), sizeof(struct customer), getpid()};
	int lockingStatus2 = fcntl(customer_fd, F_SETLKW, &lock);
	struct customer cust;
	rb = read(customer_fd, &cust, sizeof(struct customer));
	sprintf(ac_no,"%d",cust.type);
	// wb = write(conn_fd, "RD",sizeof("RD"));
	// rb = read(conn_fd,data_from_client,sizeof(data_from_client));
	// strcpy(data_from_client,"");
	// wb = (conn_fd,"Type",sizeof("Type"));
	wb = write(conn_fd, "RD",sizeof("RD"));
	rb = read(conn_fd,data_from_client,sizeof(data_from_client));
	strcpy(data_from_client,"");
	write(conn_fd,ac_no,sizeof(ac_no));
	strcpy(ac_no,"");


	// wb = write(conn_fd, "RD",sizeof("RD"));
	// rb = read(conn_fd,data_from_client,sizeof(data_from_client));
	// strcpy(data_from_client,"");
	// wb = (conn_fd,"Status",sizeof("Status"));
	sprintf(ac_no,"%d",cust.status);
	wb = write(conn_fd, "RD",sizeof("RD"));
	rb = read(conn_fd,data_from_client,sizeof(data_from_client));
	strcpy(data_from_client,"");
	write(conn_fd,ac_no,sizeof(ac_no));
	strcpy(ac_no,"");

	// wb = write(conn_fd, "RD",sizeof("RD"));
	// rb = read(conn_fd,data_from_client,sizeof(data_from_client));
	// strcpy(data_from_client,"");
	// wb = (conn_fd,"Username",sizeof("Username"));
	strcpy(ac_no,cust.username1);
	wb = write(conn_fd, "RD",sizeof("RD"));
	rb = read(conn_fd,data_from_client,sizeof(data_from_client));
	strcpy(data_from_client,"");
	write(conn_fd,ac_no,sizeof(ac_no));
	strcpy(ac_no,"");

	if(cust.type==1)
	{
		strcpy(ac_no,cust.username2);
		// wb = write(conn_fd, "RD",sizeof("RD"));
		// rb = read(conn_fd,data_from_client,sizeof(data_from_client));
		// strcpy(data_from_client,"");
		// wb = (conn_fd,"Username2",sizeof("Username2"));
		wb = write(conn_fd, "RD",sizeof("RD"));
		rb = read(conn_fd,data_from_client,sizeof(data_from_client));
		strcpy(data_from_client,"");
		write(conn_fd,ac_no,sizeof(ac_no));
		strcpy(ac_no,"");
	}

	return true;
}

bool deposit(int conn_fd,int id)
{
	char file_name[50];
	ssize_t rb, wb;
	sprintf(file_name,"%d",id);
	int amount;
	char data_from_client[1000];
	wb = write(conn_fd, "RW",sizeof("RW"));
	strcpy(data_from_client,"");
	wb = write(conn_fd, "Enter amount to be deposited\n", sizeof("Enter amount to be deposited\n"));
	rb = read(conn_fd,data_from_client,sizeof(data_from_client));
	amount = atoi(data_from_client);
	if(amount<0)
		return false;
	int account_fd = open("ACCOUNT_DB.txt", O_RDWR);
	off_t offset = lseek(account_fd, (id-1)*sizeof(struct account), SEEK_SET);

	struct flock lock = {F_WRLCK, SEEK_SET, (id-1)*sizeof(struct account), sizeof(struct account), getpid()};
	int lockingStatus = fcntl(account_fd, F_SETLKW, &lock);
	struct account acc;
	rb = read(account_fd,&acc,sizeof(struct account));
	acc.balance = acc.balance+amount;
	offset = lseek(account_fd, (id-1)*sizeof(struct account), SEEK_SET);
	wb = write(account_fd, &acc,sizeof(struct account));
	//write back the structure????
	lock.l_type = F_UNLCK;
    lockingStatus = fcntl(account_fd, F_SETLK, &lock);
	close(account_fd);

	struct transaction txn;
	time_t systime;
    time(&systime);
    strcpy(txn.date,ctime(&systime));
    txn.activity="d";
    txn.account_no=id;
    txn.amount=amount;
    txn.balance=acc.balance;
    char buf[50];
    snprintf(buf, 50, "Updated balance: %f", txn.balance);

    int trans_fd = open(file_name,O_APPEND|O_WRONLY);
    wb=write(trans_fd,&txn,sizeof(struct transaction));
    printf("%ld",wb);
    close(trans_fd);
    wb = write(conn_fd, "RD",sizeof("RD"));
	rb = read(conn_fd,data_from_client,sizeof(data_from_client));
	strcpy(data_from_client,"");
	wb = write(conn_fd,buf,sizeof(buf));
	return true;
}

bool withdraw(int conn_fd,int id)
{
	char file_name[50];
	ssize_t rb, wb;
	sprintf(file_name,"%d",id);
	int amount;
	char data_from_client[1000];
	wb = write(conn_fd, "RW",sizeof("RW"));
	strcpy(data_from_client,"");
	wb = write(conn_fd, "Enter amount to be withdrawn\n", sizeof("Enter amount to be withdrawn\n"));
	rb = read(conn_fd,data_from_client,sizeof(data_from_client));
	amount = atoi(data_from_client);
	if(amount<0)
		return false;
	int account_fd = open("ACCOUNT_DB.txt", O_RDWR);
	off_t offset = lseek(account_fd, (id-1)*sizeof(struct account), SEEK_SET);

	struct flock lock = {F_WRLCK, SEEK_SET, (id-1)*sizeof(struct account), sizeof(struct account), getpid()};
	int lockingStatus = fcntl(account_fd, F_SETLKW, &lock);
	struct account acc;
	rb = read(account_fd,&acc,sizeof(struct account));
	//printf("%f",acc.balance);
	acc.balance = acc.balance-amount;
	if(acc.balance<0)
		return false;

	offset = lseek(account_fd, (id-1)*sizeof(struct account), SEEK_SET);
	wb = write(account_fd, &acc,sizeof(struct account));
	//write back the structure????
	lock.l_type = F_UNLCK;
    lockingStatus = fcntl(account_fd, F_SETLK, &lock);
	close(account_fd);

	struct transaction txn;
	time_t systime;
    time(&systime);
    strcpy(txn.date,ctime(&systime));
    txn.activity="w";
    txn.account_no=id;
    txn.amount=amount;
    txn.balance=acc.balance;
    char buf[50];
    snprintf(buf, 50, "Updated balance: %f", txn.balance);

    int trans_fd = open(file_name,O_APPEND|O_WRONLY);
    wb=write(trans_fd,&txn,sizeof(struct transaction));
    printf("%ld",wb);
    close(trans_fd);
    wb = write(conn_fd, "RD",sizeof("RD"));
	rb = read(conn_fd,data_from_client,sizeof(data_from_client));
	strcpy(data_from_client,"");
	wb = write(conn_fd,buf,sizeof(buf));
	return true;
}

bool change_password(int conn_fd,int id)
{
	int customer_fd = open("customer_db", O_RDWR);
	off_t offset = lseek(customer_fd, (id-1)*sizeof(struct customer), SEEK_SET);

	struct flock lock = {F_RDLCK, SEEK_SET, (id-1)*sizeof(struct customer), sizeof(struct customer), getpid()};
	int lockingStatus = fcntl(customer_fd, F_SETLKW, &lock);
	ssize_t readBytes, writeBytes;
	struct customer cust;
	readBytes = read(customer_fd, &cust, sizeof(struct customer));
	char data_from_client[1000],uname[1000],pass[1000];
	writeBytes=write(conn_fd,"RW",sizeof("RW"));
    writeBytes=write(conn_fd,"Enter username\n",sizeof("Enter username\n"));
    readBytes=read(conn_fd,data_from_client,sizeof(data_from_client));
    strcpy(uname,data_from_client);
    strcpy(data_from_client,"");

    writeBytes=write(conn_fd,"RW",sizeof("RW"));
    writeBytes=write(conn_fd,"Enter password\n",sizeof("Enter password\n"));
    readBytes=read(conn_fd,data_from_client,sizeof(data_from_client));
    strcpy(pass,data_from_client);
    strcpy(data_from_client,"");
    int dummy=0;

    if(cust.type==0)
	{
		if((strcmp(uname,cust.username1)==0)&&(strcmp(pass,cust.password1)==0))
			dummy=1;
	}
	else
	{
		if((strcmp(uname,cust.username1)==0)&&(strcmp(pass,cust.password1)==0))
			dummy=1;
		if((strcmp(uname,cust.username2)==0)&&(strcmp(pass,cust.password2)==0))
			dummy=2;
	}
	if(dummy==0)
		return false;

	strcpy(pass,"");
	writeBytes=write(conn_fd,"RW",sizeof("RW"));
    writeBytes=write(conn_fd,"Enter NEW password\n",sizeof("Enter NEW password\n"));
    readBytes=read(conn_fd,data_from_client,sizeof(data_from_client));
    strcpy(pass,data_from_client);
    strcpy(data_from_client,"");

    if(dummy==1)
    	strcpy(cust.password1,pass);
    else
    	strcpy(cust.password2,pass);

    offset = lseek(customer_fd, (id-1)*sizeof(struct customer), SEEK_SET);
    writeBytes = write(customer_fd,&cust,sizeof(struct customer));

    lock.l_type = F_UNLCK;
    lockingStatus = fcntl(customer_fd, F_SETLK, &lock);
    close(customer_fd);


    writeBytes = write(conn_fd, "RD",sizeof("RD"));
	readBytes = read(conn_fd,data_from_client,sizeof(data_from_client));
	strcpy(data_from_client,"");
	writeBytes = write(conn_fd,"Password Updated\n",sizeof("Password Updated\n"));
	return true;
}

bool mini_statement(int conn_fd,int id)
{
	char file_name[50],buff[2000],data_from_client[1000];
	ssize_t rb, wb;
	sprintf(file_name,"%d",id);
	int trans_fd = open(file_name,O_RDONLY);


	//rb=1;
	struct transaction txn;
	rb = read(trans_fd,&txn,sizeof(struct transaction));
	while(rb!=0)
	{
		//rb = read(trans_fd,&txn,sizeof(struct transaction));
		/*strcpy(buff,txn.date);
		wb = write(conn_fd, "RD",sizeof("RD"));
		rb = read(conn_fd,data_from_client,sizeof(data_from_client));
		strcpy(data_from_client,"");
		wb = write(conn_fd,buff,sizeof(buff));
		printf("%s",buff);
		strcpy(buff,"");
		

		strcpy(buff,txn.activity);
		wb = write(conn_fd, "RD",sizeof("RD"));
		rb = read(conn_fd,data_from_client,sizeof(data_from_client));
		strcpy(data_from_client,"");
		wb = write(conn_fd,buff,sizeof(buff));
		printf("%s",buff);
		strcpy(buff,"");
		//print

		sprintf(buff,"%d",txn.account_no);
		wb = write(conn_fd, "RD",sizeof("RD"));
		rb = read(conn_fd,data_from_client,sizeof(data_from_client));
		strcpy(data_from_client,"");
		wb = write(conn_fd,buff,sizeof(buff));
		printf("%s",buff);
		strcpy(buff,"");

		sprintf(buff,"%f",txn.amount);
		wb = write(conn_fd, "RD",sizeof("RD"));
		rb = read(conn_fd,data_from_client,sizeof(data_from_client));
		strcpy(data_from_client,"");
		wb = write(conn_fd,buff,sizeof(buff));
		printf("%s",buff);
		strcpy(buff,"");

		sprintf(buff,"%f",txn.balance);
		wb = write(conn_fd, "RD",sizeof("RD"));
		rb = read(conn_fd,data_from_client,sizeof(data_from_client));
		strcpy(data_from_client,"");
		wb = write(conn_fd,buff,sizeof(buff));
		printf("%s",buff);
		strcpy(buff,"");*/
		sprintf(buff,"Date: %s Activity: %c Account NO: %d Amount: %f Balance: %f",txn.date,txn.activity,txn.account_no,txn.amount,txn.balance);
		wb = write(conn_fd, "RD",sizeof("RD"));
		rb = read(conn_fd,data_from_client,sizeof(data_from_client));
		strcpy(data_from_client,"");
		wb = write(conn_fd,buff,sizeof(buff));
		//printf("%s",buff);
		strcpy(buff,"");

		rb = read(trans_fd,&txn,sizeof(struct transaction));
		printf("%d",rb);

	}
	close(trans_fd);


	return true;
}

int main()
{
	int sock_fd, conn_fd;
	int bind_status;
	int listen_status;
	int client_size;
	struct sockaddr_in address, client;
	struct account acc;
	ssize_t rb,wb;
	char data_from_client[1000];
	char username[1000],password[1000];
	int id;
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	printf("Server side socket created\n");

	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_family = AF_INET;
	address.sin_port = htons(5676);

	bind_status = bind(sock_fd, (struct sockaddr *)&address, sizeof(address));

	printf("Binding to socket Successful\n");
	listen_status = listen(sock_fd, 2);
	printf("Listening for connections on this Socket\n");

	while (1)
	{
		client_size = (int)sizeof(client);
		conn_fd = accept(sock_fd, (struct sockaddr *)&client, &client_size);
		if (conn_fd == -1)
			perror("Error");
		else
		{
			if (fork() == 0)
			{
				int flag=0;
				char ch='y';
				wb = write(conn_fd, "Please Enter your id,username and password", sizeof("Please Enter your id,username and password"));
				rb = read(conn_fd, data_from_client,sizeof(data_from_client));
				id= atoi(data_from_client);
				strcpy(data_from_client,"");
				rb = read(conn_fd, data_from_client,sizeof(data_from_client));
				strcpy(username,data_from_client);
				strcpy(data_from_client,"");
				rb = read(conn_fd, data_from_client,sizeof(data_from_client));
				strcpy(password,data_from_client);
				strcpy(data_from_client,"");
				if((strcmp(username,"root")==0)&&(strcmp(password,"toor")==0))
				{
					wb=write(conn_fd,"Login Successful, Welcome Admin",sizeof("Login Successful, Welcome Admin"));
					flag=1;
					printf("flag%d\n",flag);
                            //continue;
				}
				else if(check_user(id,username,password))
				{
					wb=write(conn_fd,"Login Successful, Welcome User",sizeof("Login Successful, Welcome User"));
					flag=2;
					printf("flag%d\n",flag);

                            //continue;
				}
				else
				{
					wb=write(conn_fd,"Login Failed",sizeof("Login Successful"));
					break; 
				}
				while(ch=='y')
				{

					if(flag==1)
					{
						wb = write(conn_fd, "RW",sizeof("RW"));
						//rb = read(conn_fd,data_from_client,sizeof(data_from_client));
						strcpy(data_from_client,"");
						wb = write(conn_fd, "Enter 1 to create account\n Enter 2 to delete account \n Enter 3 to modify account \n Enter 4 to search account\n anything else to quit ", sizeof("Enter 1 to create account\n Enter 2 to delete account \n Enter 3 to delete account \n Enter 4 to search account\n anything else to quit "));
						rb = read(conn_fd,data_from_client,sizeof(data_from_client));
						char c=data_from_client[0];
						strcpy(data_from_client,"");
						int statess;
						switch(c)
						{
							case '1':
							
							statess = add_user(conn_fd);
							if(statess!=1)
							{
								wb = write(conn_fd, "RD",sizeof("RD"));
								rb = read(conn_fd,data_from_client,sizeof(data_from_client));
								strcpy(data_from_client,"");
								write(conn_fd,"Failed Creating Account\n",sizeof("Failed Creating Account\n"));
							}
							else
							{
								wb=write(conn_fd,"RD",sizeof("RD"));
    					    	rb = read(conn_fd,data_from_client,sizeof(data_from_client));
								strcpy(data_from_client,"");
								wb = write(conn_fd,"Account is created",sizeof("Account is created"));
							}
							break;
							

							case '2':
							wb = write(conn_fd, "RW",sizeof("RW"));
							strcpy(data_from_client,"");
							wb = write(conn_fd, "Enter account no to be deleted\n", sizeof("Enter account no to be deleted\n"));
							rb = read(conn_fd,data_from_client,sizeof(data_from_client));
							int ac_id= atoi(data_from_client);
							bool st = del_user(ac_id);

							wb = write(conn_fd, "RD",sizeof("RD"));
							rb = read(conn_fd,data_from_client,sizeof(data_from_client));
							strcpy(data_from_client,"");
							write(conn_fd,"Account deleted\n",sizeof("Account deleted\n"));
							
							break;

							case '3':
							statess = modify_account(conn_fd);

							if(statess!=1)
							{
								wb = write(conn_fd, "RD",sizeof("RD"));
								rb = read(conn_fd,data_from_client,sizeof(data_from_client));
								strcpy(data_from_client,"");
								write(conn_fd,"Failed Modifying Account\n",sizeof("Failed Modifying Account\n"));
							}
							else
							{
								wb=write(conn_fd,"RD",sizeof("RD"));
    					    	rb = read(conn_fd,data_from_client,sizeof(data_from_client));
								strcpy(data_from_client,"");
								wb = write(conn_fd,"Account modified\n",sizeof("Account modified\n"));
							}
							break;

							case '4':
							statess = search_account(conn_fd);

							if(statess!=1)
							{
								wb = write(conn_fd, "RD",sizeof("RD"));
								rb = read(conn_fd,data_from_client,sizeof(data_from_client));
								strcpy(data_from_client,"");
								write(conn_fd,"Failed Searching Account\n",sizeof("Failed Searching Account\n"));
							}
							else
							{
								wb=write(conn_fd,"RD",sizeof("RD"));
    					    	rb = read(conn_fd,data_from_client,sizeof(data_from_client));
								strcpy(data_from_client,"");
								wb = write(conn_fd,"Account is searched\n",sizeof("Account is searched\n"));
							}

							default:
							wb = write(conn_fd, "RD",sizeof("RD"));
							rb = read(conn_fd,data_from_client,sizeof(data_from_client));
							strcpy(data_from_client,"");
							write(conn_fd,"Connection closed",sizeof("Connection closed"));
							break;
						}

					}
					if(flag==2)
					{
						wb = write(conn_fd, "RW",sizeof("RW"));
						//rb = read(conn_fd,data_from_client,sizeof(data_from_client));
						strcpy(data_from_client,"");
						wb = write(conn_fd, "Enter 1 to view statement\n Enter 2 to withdraw \n Enter 3 to deposit\n Enter 4 to change password\n anything else to quit ", sizeof("Enter 1 to view statement\n Enter 2 to withdraw \n Enter 3 to deposit\n Enter 4 to change password\n anything else to quit "));
						rb = read(conn_fd,data_from_client,sizeof(data_from_client));
						char c=data_from_client[0];
						strcpy(data_from_client,"");
						int statess;
						switch(c)
						{
							case '1':
							statess = mini_statement(conn_fd,id);
							if(statess!=1)
							{
								wb = write(conn_fd, "RD",sizeof("RD"));
								rb = read(conn_fd,data_from_client,sizeof(data_from_client));
								strcpy(data_from_client,"");
								write(conn_fd,"Failed Printing Mini statement\n",sizeof("Failed Printing Mini statement\n"));
							}
							break;

							case '2':
							statess = withdraw(conn_fd,id);
							if(statess!=1)
							{
								wb = write(conn_fd, "RD",sizeof("RD"));
								rb = read(conn_fd,data_from_client,sizeof(data_from_client));
								strcpy(data_from_client,"");
								write(conn_fd,"Failed Withdrawing Amount\n",sizeof("Failed Withdrawing Amount\n"));
							}
							// wb = write(conn_fd, "RD",sizeof("RD"));
							// rb = read(conn_fd,data_from_client,sizeof(data_from_client));
							// strcpy(data_from_client,"");
							// write(conn_fd,"I delete account\n",sizeof("I delete account\n"));
							break;

							case '3':
							statess = deposit(conn_fd,id);
							if(statess!=1)
							{
								wb = write(conn_fd, "RD",sizeof("RD"));
								rb = read(conn_fd,data_from_client,sizeof(data_from_client));
								strcpy(data_from_client,"");
								write(conn_fd,"Failed Depositing Amount\n",sizeof("Failed Depositing Amount\n"));
							}
							// wb = write(conn_fd, "RD",sizeof("RD"));
							// rb = read(conn_fd,data_from_client,sizeof(data_from_client));
							// strcpy(data_from_client,"");
							// write(conn_fd,"I modify account\n",sizeof("I modify account\n"));
							break;

							case '4':
							statess = change_password(conn_fd,id);
							if(statess!=1)
							{
								wb = write(conn_fd, "RD",sizeof("RD"));
								rb = read(conn_fd,data_from_client,sizeof(data_from_client));
								strcpy(data_from_client,"");
								write(conn_fd,"Failed Changing Password\n",sizeof("Failed Changing Password\n"));
							}
							// wb = write(conn_fd, "RD",sizeof("RD"));
							// rb = read(conn_fd,data_from_client,sizeof(data_from_client));
							// strcpy(data_from_client,"");
							// write(conn_fd,"I search account\n",sizeof("I search account\n"));
							break;

							default:
							wb = write(conn_fd, "RD",sizeof("RD"));
							rb = read(conn_fd,data_from_client,sizeof(data_from_client));
							strcpy(data_from_client,"");
							write(conn_fd,"Connection closed",sizeof("Connection closed"));
							break;
						}

					}
					wb = write(conn_fd, "RW",sizeof("RW"));
					//rb = read(conn_fd,data_from_client,sizeof(data_from_client));
					strcpy(data_from_client,"");
					wb = write(conn_fd, "Press y to start again, n to exit\n",sizeof("Press y to start again, n to exit\n"));
					rb = read(conn_fd, data_from_client,sizeof(data_from_client));
					ch=data_from_client[0];
					printf("ch %c\n",ch);
					if(ch!='y')
					{
						wb = write(conn_fd, "RD",sizeof("RD"));
						rb = read(conn_fd,data_from_client,sizeof(data_from_client));
						strcpy(data_from_client,"");
						write(conn_fd,"Connection closed",sizeof("Connection closed"));
							//break;
					//printf("%s\n",ch );
					strcpy(data_from_client,"");
					}
				}
				wb = write(conn_fd, "RD",sizeof("RD"));
				rb = read(conn_fd,data_from_client,sizeof(data_from_client));
				strcpy(data_from_client,"");
				wb = write(conn_fd, "Connection Ended\n",sizeof("Connection Ended\n"));
				close(conn_fd);
			}
			else
				close(conn_fd);
		}
	}
	close(sock_fd);
	return 0;
}

//(strcmp(username,"user")==0)&&(strcmp(password,"resu")==0)