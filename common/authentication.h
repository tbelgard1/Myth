/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#ifdef linux
#define SERVER
#endif

enum {
	TIME_IN_SECONDS= 60,
	TIME_IN_MINUTES= TIME_IN_SECONDS*60,
	TIME_IN_HOURS= TIME_IN_MINUTES*60,
	TIME_IN_DAYS= 24*TIME_IN_HOURS,
	MAXIMUM_SALT_SIZE= 16,
	MAXIMUM_ENCRYPTED_PASSWORD_SIZE= MAXIMUM_SALT_SIZE,
	AUTHENTICATION_EXPIRATION_TIME= 2*TIME_IN_DAYS
};

enum {
	MAXIMUM_HANDOFF_TOKEN_SIZE= 32
};


enum {
	_plaintext_encryption,
	_simple_encryption,
	NUMBER_OF_ENCRYPTION_TYPES
};

#define	GUEST_ACCOUNT_NAME			""

void encrypt_password(char *password, unsigned char *salt, char *result, short type);

typedef unsigned char authentication_token[MAXIMUM_HANDOFF_TOKEN_SIZE];

long get_current_time(void);

#ifdef SERVER
boolean guest_token(authentication_token *token);
void get_guest_user_token(authentication_token token);
void generate_authentication_token(long host_address, long user_id,
	long expiration_time, authentication_token token);
boolean authenticate_token(long client_address, long current_time, long *user_id, authentication_token token);

void get_random_salt(byte *salt);

void test_authentication_tokens(void);
#endif
