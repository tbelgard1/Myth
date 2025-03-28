#ifndef __UPDATE_SERVER_PACKETS_H
#define __UPDATE_SERVER_PACKETS_H

#define DEFAULT_UPDATE_SERVER_PORT		7982
#define FIRST_USERD_PACKET				100
#define FIRST_CLIENT_UPDATE_PACKET		200
#define FIRST_SERVER_UPDATE_PACKET		300

enum
{
	// from update server to userd
	_us_login_packet,
	_us_update_information_packet,

	// from userd to update_server
	_us_login_successful_packet = FIRST_USERD_PACKET,
	_us_login_failure_packet,

	// from clients to update_server
	_update_server_login_packet = FIRST_CLIENT_UPDATE_PACKET,
	_fetch_app_packet,
	_fetch_app_rf_packet,
	_fetch_update_app_packet,
	_fetch_update_app_rf_packet,
	_fetch_patch_packet,
	_fetch_number_of_present_updates_packet,

	// from update server to clients
	_present_update_info_packet = FIRST_SERVER_UPDATE_PACKET,
	_file_description_packet,
	_file_transfer_complete_packet,
	_number_of_present_updates_packet
};

struct update_server_packet_header
{
	short type;
	short length;
};

// from update server to userd
struct us_update_information_header
{
	short number_of_applications;		// related to number of platforms
	short number_of_bnupdates;			// ditto; should be identical to above
	short number_of_patches;
	short unused_0;
	long host;
	short port;
	short unused_1;
};

struct us_update_information_app_segment
{
	long version_number;
	long platform_type;
	long size;
};

struct us_update_information_patch_segment
{
	long patch_number;
	long size;
	long description_length;
	// char * description ...
};

// from clients to update server
struct update_server_login_packet
{
	short platform_type;
	short country_code;
};

struct fetch_app_packet
{
	short platform_type;
	short country_code;
};

struct fetch_app_rf_packet
{
	short platform_type;
	short country_code;
};

struct fetch_update_app_packet
{
	short platform_type;
	short country_code;
};

struct fetch_update_app_rf_packet
{
	short platform_type;
	short country_code;
};

struct fetch_patch_packet
{
	long patch_number;
	short country_code;
	short unused_short;
};

struct file_transfer_complete_ack_packet
{
	long update_id;
};

// from update server to clients
struct present_update_info_packet
{
	long app_version;
	long bnupdate_version;
	long patch_number;
};

struct file_description_packet
{
	long patch_number;
	long size;				// in bytes
	// char * description ...
};

struct file_transfer_complete_packet
{
	long update_id;	
};

struct number_of_present_updates_packet
{
	long number_of_present_updates;
};

// sent from update server to userd
short build_us_login_packet(char * buffer);
short start_building_us_update_information_packet(char * buffer, long host, short port);															      
short add_applicaton_to_us_update_information_packet(char * buffer, struct update_app_entry * app);
short add_bnupdate_to_us_update_information_packet(char * buffer, struct update_app_entry * app);
short add_patch_to_update_information_packet(char * buffer, struct update_patch_entry * patch);

// sent from userd to update server
short build_us_login_success_packet(char * buffer);
short build_us_login_failure_packet(char * buffer);

// sent from clients to update server
short build_update_server_login_packet(char * buffer, short platform_type, short country_code);
short build_fetch_app_packet(char * buffer, short platform_type, short country_code);
short build_fetch_app_rf_packet(char * buffer, short platform_type, short country_code);
short build_fetch_update_app_packet(char * buffer, short platform_type, short country_code);
short build_fetch_update_app_rf_packet(char * buffer, short platform_type, short country_code);
short build_fetch_patch_packet(char * buffer, long patch_number, short country_code);
short build_fetch_number_of_present_updates_packet(char * buffer);

// sent from update server to clients
short build_present_update_info_packet(char * buffer, long app_version, long bnupdate_version,
	long patch_number);
short build_file_description_packet(char * buffer, long patch_number, long size, char * description);
short build_file_transfer_complete_packet(char * buffer, long update_id);
short build_number_of_present_updates_packet(char * buffer, long number_of_present_updates);

// send function
boolean send_update_server_packet(int socket, char * buffer, short length);

// byte swapping function
boolean byte_swap_update_server_packet(char * buffer, boolean outgoing);

#endif
