#ifndef BUNGIE_NET_ORDER_H
#define BUNGIE_NET_ORDER_H

#include <stdint.h>
#include "bungie_net_constants.h"

/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

enum {
    MAXIMUM_ORDER_NAME_LENGTH = 31,
    MAXIMUM_ORDER_MOTTO_LENGTH = 511,
    MAXIMUM_ORDER_URL_LENGTH = 127,
    MAXIMUM_ORDER_CONTACT_EMAIL_LENGTH = 127
};

#pragma pack(push, 1)
typedef struct bungie_net_order_database_header {
    char signature[4];         // should be "RDRO"
    uint32_t version;          // 0
    uint32_t order_count;      // number of orders
} bungie_net_order_database_header;
#pragma pack(pop)

typedef struct bungie_net_order_datum {
    unsigned long order_id;
    long founding_date;
    long initial_date_below_three_members;

    char name[MAXIMUM_ORDER_NAME_LENGTH + 1];
    char motto[MAXIMUM_ORDER_MOTTO_LENGTH + 1];
    char url[MAXIMUM_ORDER_URL_LENGTH + 1];
    char contact_email[MAXIMUM_ORDER_CONTACT_EMAIL_LENGTH + 1];
    char maintenance_password[MAXIMUM_PASSWORD_LENGTH + 1];
    char member_password[MAXIMUM_PASSWORD_LENGTH + 1];

    struct bungie_net_player_score_datum unranked_score;
    struct bungie_net_player_score_datum ranked_score;
    struct bungie_net_player_score_datum ranked_scores_by_game_type[MAXIMUM_NUMBER_OF_GAME_TYPES];
} bungie_net_order_datum;

#endif // __BUNGIE_NET_ORDER_H__
