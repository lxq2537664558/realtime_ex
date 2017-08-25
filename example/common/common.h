#pragma once
#include <stdint.h>

// 根据玩家id获取数据库id 规则是玩家id的高10位是数据库id
#define _GET_PLAYER_DB_ID(id) (uint32_t)(id>>54)

#define _UC_SERVICE_DELTA		80000
#define _GAME_DB_SERVICE_DELTA	70000