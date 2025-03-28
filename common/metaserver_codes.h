/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

enum {
	_syntax_error_msg,
	_login_failed_games_not_allowed,
	_login_failed_invalid_version,
	_login_failed_bad_user_or_password,
	_user_not_logged_in_msg,
	_bad_metaserver_version_msg,
	_user_already_logged_in_msg,
	_unknown_game_type_msg,
	_login_successful_msg,
	_logout_successful_msg,
	_player_not_in_room_msg,
	_game_already_exists_msg,
	_account_already_logged_in,
	_room_full_msg,
	_metaserver_account_locked_msg,
	_metaserver_not_supported,
	NUMBER_OF_MESSAGE_TYPES
};

#define METASERVER_MAJOR_VERSION (1)

#ifdef INCLUDE_DATA
char *messages[]= {
  "Syntax error (unrecognized command).",
  "Login failed (Games not allowed at this time)." ,
  "Login failed (Invalid Game Version number)." ,
  "Login failed (Bad user or Password)." ,
  "User not logged in." ,
  "Bad metaserver version." ,
  "User already logged in!",
  "Unknown game type!",
  "User logged in.",
  "User logged out.",
  "Player not in a room!",
  "You already created a game!",
  "This account is already logged in!",
  "The desired room is full!",
  "Your account has been locked",
  "The game server for your product has been shutdown"
};
#define NUMBER_OF_MESSAGES (sizeof(messages)/sizeof(messages[0]))
#else
extern char *messages[];
#endif
