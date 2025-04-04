#include "cseries.h"

#include "byte_swapping.h"
#include "update_dat.h"
#include "update_server_packets.h"

static void byte_swap_us_update_information_packet(char * buffer, short length,	boolean outgoing);
static short build_empty_header(char * buffer, short type);
static short append_data_to_packet(char * buffer, void * data, short data_length);

short build_us_login_packet(
	char * buffer)
{
	return build_empty_header(buffer, _us_login_packet);
}

short build_us_login_successful_packet(
	char * buffer)
{
	return build_empty_header(buffer, _us_login_successful_packet);
}

short build_us_login_failure_packet(
	char * buffer)
{
	return build_empty_header(buffer, _us_login_failure_packet);
}

short start_building_us_update_information_packet(
	char * buffer,
	long host,
	short port)
{
	short l = 0;

	build_empty_header(buffer, _us_update_information_packet);
	append_data_to_packet(buffer, (void *)&l, sizeof(l));
	append_data_to_packet(buffer, (void *)&l, sizeof(l));
	append_data_to_packet(buffer, (void *)&l, sizeof(l));
	append_data_to_packet(buffer, (void *)&l, sizeof(l));
	append_data_to_packet(buffer, (void *)&host, sizeof(host));
	append_data_to_packet(buffer, (void *)&port, sizeof(port));
	return append_data_to_packet(buffer, (void *)&l, sizeof(l));
}

short add_application_to_us_update_information_packet(
	char * buffer, 
	struct update_app_entry * app)
{
	char * p;
	struct us_update_information_header * header;

	p = buffer;
	p += sizeof(struct update_server_packet_header);

	header = (struct us_update_information_header *)p;
	header->number_of_applications++;

	append_data_to_packet(buffer, (void *)&app->version_number, sizeof(app->version_number));
	append_data_to_packet(buffer, (void *)&app->platform_type, sizeof(app->platform_type));

	return append_data_to_packet(buffer, (void *)&app->size, sizeof(app->size));
}

short add_bnupdate_to_us_update_information_packet(
	char * buffer,
	struct update_app_entry * app)
{
	char * p;
	struct us_update_information_header * header;

	p = buffer;
	p += sizeof(struct update_server_packet_header);

	header = (struct us_update_information_header *)p;
	header->number_of_bnupdates++;

	append_data_to_packet(buffer, (void *)&app->version_number, sizeof(app->version_number));
	append_data_to_packet(buffer, (void *)&app->platform_type, sizeof(app->platform_type));
	return append_data_to_packet(buffer, (void *)&app->size, sizeof(app->size));
}

short add_patch_to_us_update_information_packet(
	char * buffer,
	struct update_patch_entry * patch)
{
	char * p;
	struct us_update_information_header * header;
	long length;

	p = buffer;
	p += sizeof(struct update_server_packet_header);

	header = (struct us_update_information_header *)p;
	header->number_of_patches++;

	append_data_to_packet(buffer, (void *)&patch->patch_number, sizeof(patch->patch_number));
	append_data_to_packet(buffer, (void *)&patch->size, sizeof(patch->size));
	length = strlen(patch->description) + 1;
	append_data_to_packet(buffer, (void *)&length, sizeof(length));
	return append_data_to_packet(buffer, (void *)&patch->description[0], length);
}

short build_update_server_login_packet(
	char * buffer,
	short platform_type,
	short country_code)
{
	build_empty_header(buffer, _fetch_app_packet);
	append_data_to_packet(buffer, (void *)&platform_type, sizeof(platform_type));
	return append_data_to_packet(buffer, (void *)&country_code, sizeof(country_code));
}

short build_fetch_app_packet(
	char * buffer,
	short platform_type,
	short country_code)
{
	build_empty_header(buffer, _fetch_app_packet);
	append_data_to_packet(buffer, (void *)&platform_type, sizeof(platform_type));
	return append_data_to_packet(buffer, (void *)&country_code, sizeof(country_code));
}

short build_fetch_app_rf_packet(
	char * buffer,
	short platform_type,
	short country_code)
{
	build_empty_header(buffer, _fetch_app_rf_packet);
	append_data_to_packet(buffer, (void *)&platform_type, sizeof(platform_type));
	return append_data_to_packet(buffer, (void *)&country_code, sizeof(country_code));
}

short build_fetch_update_app_packet(
	char * buffer,
	short platform_type,
	short country_code)
{
	build_empty_header(buffer, _fetch_update_app_packet);
	append_data_to_packet(buffer, (void *)&platform_type, sizeof(platform_type));
	return append_data_to_packet(buffer, (void *)&country_code, sizeof(country_code));
}

short build_fetch_update_app_rf_packet(
	char * buffer,
	short platform_type,
	short country_code)
{
	build_empty_header(buffer, _fetch_update_app_rf_packet);
	append_data_to_packet(buffer, (void *)&platform_type, sizeof(platform_type));
	return append_data_to_packet(buffer, (void *)&country_code, sizeof(country_code));
}

short build_fetch_patch_packet(
	char * buffer,
	long patch_number,
	short country_code)
{
	build_empty_header(buffer, _fetch_patch_packet);
	append_data_to_packet(buffer, (void *)&patch_number, sizeof(patch_number));
	append_data_to_packet(buffer, (void *)&country_code, sizeof(country_code));
	return append_data_to_packet(buffer, (void *)&country_code, sizeof(country_code));
}

short build_fetch_number_of_present_updates_packet(
	char * buffer)
{
	return build_empty_header(buffer, _fetch_number_of_present_updates_packet);
}

short build_present_update_info_packet(
	char * buffer,
	long app_version,
	long bnupdate_version,
	long patch_number)	
{
	build_empty_header(buffer, _present_update_info_packet);
	append_data_to_packet(buffer, (void *)&app_version, sizeof(app_version));
	append_data_to_packet(buffer, (void *)&bnupdate_version, sizeof(bnupdate_version));
	return append_data_to_packet(buffer, (void *)&patch_number, sizeof(patch_number));	
}

short build_file_description_packet(
	char * buffer,
	long patch_number,
	long size,
	char * description)
{
	build_empty_header(buffer, _file_description_packet);
	append_data_to_packet(buffer, (void *)&patch_number, sizeof(patch_number));
	append_data_to_packet(buffer, (void *)&size, sizeof(size));
	return append_data_to_packet(buffer, (void *)description, strlen(description) + 1);
}

short build_file_transfer_complete_packet(
	char * buffer,
	long update_id)
{
	build_empty_header(buffer, _file_transfer_complete_packet);
	return append_data_to_packet(buffer, (void *)&update_id, sizeof(update_id));
}

short build_number_of_present_updates_packet(
	char * buffer,
	long number_of_present_updates)
{
	build_empty_header(buffer, _number_of_present_updates_packet);
	return append_data_to_packet(buffer, (void *)&number_of_present_updates, sizeof(long));
}

static short build_empty_header(
	char * buffer,
	short type)
{
	struct update_server_packet_header * header = 
		(struct update_server_packet_header *)buffer;

	/* Fill in the header */
	header->type = type;
	header->length = sizeof(struct update_server_packet_header);

	return header->length;
}

static short append_data_to_packet(
	char * buffer,
	void * data,
	short data_length)
{
	struct update_server_packet_header * header = (struct update_server_packet_header *) buffer;
	char * dest = (buffer + header->length);

	/* Fill in the header */	
	memcpy(dest, data, data_length);
	header->length += data_length;

	return header->length;
}

boolean send_update_server_packet(int socket, char * buffer, short length)
{
	int last_sent;
	int left_to_send;
	char * index = buffer;

	byte_swap_update_server_packet(buffer, TRUE);
	left_to_send = length;
	while (left_to_send)
	{
		last_sent = send(socket, index, length, 0);
		if (last_sent == -1)
			return FALSE;
		left_to_send -= last_sent;
		index += last_sent;
	}

	byte_swap_update_server_packet(buffer, FALSE);
}

static byte_swap_code _bs_us_packet_header[] =
{
	_begin_bs_array, 1, 
		_2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_us_update_information_header[] =
{
	_begin_bs_array, 1,
		_2byte, _2byte, _2byte,
		_2byte, _4byte, _2byte,
		_2byte,
	_end_bs_array
};

static byte_swap_code _bs_us_update_information_app_segment[] = 
{
	_begin_bs_array, 1,
		_4byte, _4byte, _4byte,
	_end_bs_array
};

static byte_swap_code _bs_us_update_information_patch_segment[] =
{
	_begin_bs_array, 1,
		_4byte, _4byte, _4byte,
	_end_bs_array
};

static byte_swap_code _bs_us_update_server_login_packet[] = 
{
	_begin_bs_array, 1,
		_2byte, _2byte,
		_2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_us_fetch_app_packet[] = 
{
	_begin_bs_array, 1,
		_2byte, _2byte,
		_2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_us_fetch_app_rf_packet[] = 
{
	_begin_bs_array, 1,
		_2byte, _2byte,
		_2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_us_fetch_update_app_packet[] = 
{
	_begin_bs_array, 1,
		_2byte, _2byte,
		_2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_us_fetch_update_app_rf_packet[] = 
{
	_begin_bs_array, 1,
		_2byte, _2byte,
		_2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_us_fetch_patch_packet[] =
{
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte,
		_2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_us_fetch_number_of_present_updates_packet[] =
{
	_begin_bs_array, 1,
		_2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_us_file_transfer_complete_ack_packet[] =
{
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte,
	_end_bs_array
};

static byte_swap_code _bs_us_present_update_info_packet[] =
{
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte, _4byte, _4byte,
	_end_bs_array
};

static byte_swap_code _bs_us_file_description_packet[] =
{
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte, _4byte,
	_end_bs_array
};

static byte_swap_code _bs_us_file_transfer_complete_packet[] =
{
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte,
	_end_bs_array
};

static byte_swap_code _bs_us_number_of_present_updates_packet[] =
{
	_begin_bs_array, 1,
		_2byte, _2byte, 
		_4byte,
	_end_bs_array
};

boolean byte_swap_update_server_packet(
	char * buffer, 
	boolean outgoing)
{
	short type, length;
	struct update_server_packet_header * header;
	boolean success;

	header = (struct update_server_packet_header *)buffer;
	type = header->type;
	length = header->length;

	if (!outgoing)
	{
		type = SWAP2(type);
		length = SWAP2(length);
	}

	switch (type)
	{
	// sent from update server to userd
	case _us_login_packet:
		byte_swap_data("update server packet", buffer, 
			sizeof(struct update_server_packet_header), 1,
			_bs_us_packet_header);
		success = TRUE;
		break;

	case _us_update_information_packet:
		byte_swap_us_update_information_packet(buffer, length, outgoing);
		success = TRUE;
		break;

	// sent from userd to update_server
	case _us_login_successful_packet:
	case _us_login_failure_packet:
		byte_swap_data("update server packet", buffer, 
			sizeof(struct update_server_packet_header), 1,
			_bs_us_packet_header);
		success = TRUE;
		break;

	// send from client to update server
	case _update_server_login_packet:
		byte_swap_data("update server packet", buffer,
			sizeof(struct update_server_packet_header) +
			sizeof(struct update_server_login_packet), 1,
			_bs_us_update_server_login_packet);
		success = TRUE;
		break;

	case _fetch_app_packet:
		byte_swap_data("update server packet", buffer,
			sizeof(struct update_server_packet_header) +
			sizeof(struct fetch_app_packet), 1, 
			_bs_us_fetch_app_packet);
		success = TRUE;
		break;

	case _fetch_app_rf_packet:
		byte_swap_data("update_server_packet", buffer, 
			sizeof(struct update_server_packet_header) + 
			sizeof(struct fetch_app_rf_packet), 1,
			_bs_us_fetch_app_rf_packet);
		success = TRUE;
		break;

	case _fetch_update_app_packet:
		byte_swap_data("update server packet", buffer,
			sizeof(struct update_server_packet_header) +
			sizeof(struct fetch_app_packet), 1, 
			_bs_us_fetch_update_app_packet);
		success = TRUE;
		break;

	case _fetch_update_app_rf_packet:
		byte_swap_data("update_server_packet", buffer, 
			sizeof(struct update_server_packet_header) + 
			sizeof(struct fetch_app_rf_packet), 1,
			_bs_us_fetch_update_app_rf_packet);
		success = TRUE;
		break;

	case _fetch_patch_packet:
		byte_swap_data("update server packet", buffer,
			sizeof(struct update_server_packet_header) +
			sizeof(struct fetch_patch_packet), 1,
			_bs_us_fetch_patch_packet);
		success = TRUE;
		break;

	case _fetch_number_of_present_updates_packet:
		byte_swap_data("update server packet", buffer,
			sizeof(struct update_server_packet_header), 1,
			_bs_us_fetch_number_of_present_updates_packet);
		success = TRUE;
		break;

	// sent from update server to client
	case _present_update_info_packet:
		byte_swap_data("update server packet", buffer,
			sizeof(struct update_server_packet_header) +
			sizeof(struct present_update_info_packet), 1,
			_bs_us_present_update_info_packet);
		success = TRUE;
		break;

	case _file_description_packet:
		byte_swap_data("update server packet", buffer,
			sizeof(struct update_server_packet_header) + 
			sizeof(struct file_description_packet), 1, 
			_bs_us_file_description_packet);
		success = TRUE;
		break;

	case _file_transfer_complete_packet:
		byte_swap_data("update server packet", buffer,
			sizeof(struct update_server_packet_header) +
			sizeof(struct file_transfer_complete_packet), 1,
			_bs_us_file_transfer_complete_packet);
		success = TRUE;
		break;

	case _number_of_present_updates_packet:
		byte_swap_data("update server packet", buffer,
			sizeof(struct update_server_packet_header) + 
			sizeof(struct number_of_present_updates_packet), 1,
			_bs_us_number_of_present_updates_packet);
		success = TRUE;
		break;

	default:
		success = FALSE;
		break;
	}

	return success;
}

static void byte_swap_us_update_information_packet(
	char * buffer, 
	short length,
	boolean outgoing)
{
	char * byte_swap_me;
	int app_count;
	int bnupdate_count;
	int patch_count;
	int index;
	int size;				// used for patch segment descriptions

	struct us_update_information_header * header;
	struct us_update_information_patch_segment * patch;

	byte_swap_me = buffer;
	// start out with the packet header

	byte_swap_data("update information packet", byte_swap_me, 
		sizeof(struct update_server_packet_header), 1, _bs_us_packet_header);

	byte_swap_me += sizeof(struct update_server_packet_header);
	
	header = (struct us_update_information_header *)byte_swap_me;
	if (outgoing)
	{
		app_count = header->number_of_applications;
		bnupdate_count = header->number_of_bnupdates;
		patch_count = header->number_of_patches;
	}

	// now lets get the update_information_header
	byte_swap_data("update information packet", byte_swap_me,
		sizeof(struct us_update_information_header), 1, _bs_us_update_information_header);

	if (!outgoing)
	{
		app_count = header->number_of_applications;
		bnupdate_count = header->number_of_bnupdates;
		patch_count = header->number_of_patches;
	}

	byte_swap_me += sizeof(struct us_update_information_header);

	// get all of the app segments
	for (index = 0; index < app_count; index++)
	{
		byte_swap_data("update information packet", byte_swap_me,
			sizeof(struct us_update_information_app_segment), 1,
			_bs_us_update_information_app_segment);
		byte_swap_me += sizeof(struct us_update_information_app_segment);
	}

	for (index = 0; index < bnupdate_count; index++)
	{
		byte_swap_data("update information packet", byte_swap_me,
			sizeof(struct us_update_information_app_segment), 1,
			_bs_us_update_information_app_segment);
		byte_swap_me += sizeof(struct us_update_information_app_segment);
	}

	// get all of the patch segments
	for (index = 0; index < patch_count; index++)
	{
		patch = (struct us_update_information_patch_segment *)byte_swap_me;
		if (outgoing)
			size = patch->description_length;

		byte_swap_data("update information packet", byte_swap_me,
			sizeof(struct us_update_information_patch_segment), 1,
			_bs_us_update_information_patch_segment);
		if (!outgoing)
			size = patch->description_length;
		byte_swap_me += sizeof(struct us_update_information_patch_segment) + size;
	}
}
