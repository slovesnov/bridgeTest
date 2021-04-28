/*
 * BridgeConstants.h
 *
 *       Created on: 29.11.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 *
 *
 * solver folder is using in both projects bridge(gtk) & bridgeConsole, this make dependences between projects
 * if put "solver" folder in bridge project then it'll be compiled every time if one need changes in bridge or bridgeConsole project
 * bridgeConsole use internal compiling which is really long, so it's better to put "solver" folder into bridgeConsole project
 * which compiling really fast
 */

#ifndef SOLVER_BRIDGECOMMON_H_

#ifndef BRIDGECONSTANTS_H_
#define BRIDGECONSTANTS_H_

const int NT = 4;
const int MIZER = NT + 1;
const int UNKNOWN_ESTIMATE = 100;
const int MAX_TABLE_CARDS[] = { 4, 3 };

enum ESTIMATE {
	ESTIMATE_NONE,
	ESTIMATE_BEST_LOCAL,
	ESTIMATE_BEST_TOTAL,
	ESTIMATE_ALL_LOCAL,
	ESTIMATE_ALL_TOTAL
};

enum CARD_INDEX {
	CARD_INDEX_INVALID = -1,
	CARD_INDEX_ABSENT,
	CARD_INDEX_NORTH,
	CARD_INDEX_EAST,
	CARD_INDEX_SOUTH,
	CARD_INDEX_WEST,
	CARD_INDEX_NORTH_INNER,
	CARD_INDEX_EAST_INNER,
	CARD_INDEX_SOUTH_INNER,
	CARD_INDEX_WEST_INNER
};

const char RANK[] = "akqjt98765432";
const char SUITS_CHAR[] = "shdcn";
typedef void (*SET_ESTIMATION_FUNCTION)(int index, int value);

const char BRIDGE_SIGNAL_FILE_NAME[] = "HwaQbVSv7FG4kxwP.hHT";
/* Note if use negative exit status then g_spawn_command_line_sync
 * interprets it as error and don't get status
 */
const int BRIDGE_CONSOLE_STATUS_OK = 0;
const int BRIDGE_CONSOLE_STATUS_USER_BREAK = 1;

#endif /* BRIDGECONSTANTS_H_ */
#endif
