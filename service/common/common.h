#pragma once
#include <stdint.h>

// �������id��ȡ���ݿ�id ���������id�ĸ�10λ�����ݿ�id
#define _GET_PLAYER_DB_ID(id) (uint32_t)(id>>54)

#define _UC_SERVICE_DELTA		80000
#define _GAME_DB_SERVICE_DELTA	70000