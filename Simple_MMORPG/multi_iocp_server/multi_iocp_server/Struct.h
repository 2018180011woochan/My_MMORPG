#pragma once
#ifndef __STRUCT_H__
#define __STRUCT_H__

#include "Enum.h"
#include "protocol.h"

struct OBJ_STAT
{
	int		_id;
	int		_db_id;
	short	x, y;
	char	_name[NAME_SIZE];
	//RACE	race;
	int		race;
	short	level;
	int		exp, maxexp;
	int		hp, hpmax;
	SECTOR  sector;
	bool	isDead;
};

struct BLOCK {
	int     blockID;
	short	x, y;
	SECTOR  sector;
};


#endif // !__STRUCT_H__
