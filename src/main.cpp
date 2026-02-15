/*
TEST_QMM - QMM plugin for testing QMM features in development
Copyright 2025-2026-2026
https://github.com/thecybermind/test_qmm/
3-clause BSD license: https://opensource.org/license/bsd-3-clause

Created By:
    Kevin Masterson < k.m.masterson@gmail.com >

*/

// #define TEST_CFG
// #define TEST_RETURN_TEST
// #define TEST_BROADCAST
// #define TEST_COMMAND
// #define TEST_SOF2SP_GENTITY
#define TEST_QVM_FUNC

#define _CRT_SECURE_NO_WARNINGS 1

#include <qmmapi.h>

#include "version.h"
#include "game.h"
#include <stdlib.h>
#include <string.h>

pluginres_t* g_result = nullptr;
plugininfo_t g_plugininfo = {
	QMM_PIFV_MAJOR,								// plugin interface version major
	QMM_PIFV_MINOR,								// plugin interface version minor
	"TEST_QMM",									// name of plugin
	TEST_QMM_VERSION,							// version of plugin
	"Test plugin",								// description of plugin
	TEST_QMM_BUILDER,							// author of plugin
	"https://github.com/thecybermind/test_qmm",	// website of plugin
	"TEST",										// logtag of plugin
};
eng_syscall_t g_syscall = nullptr;
mod_vmMain_t g_vmMain = nullptr;
pluginfuncs_t* g_pluginfuncs = nullptr;
pluginvars_t* g_pluginvars = nullptr;

// store the game's entity and client info
gentity_t* g_gents = nullptr;
intptr_t g_numgents = 0;
intptr_t g_gentsize = 1;
gclient_t* g_clients = nullptr;
intptr_t g_clientsize = 1;


C_DLLEXPORT void QMM_Query(plugininfo_t** pinfo) {
	// give QMM our plugin info struct
	QMM_GIVE_PINFO();
}


C_DLLEXPORT int QMM_Attach(eng_syscall_t engfunc, mod_vmMain_t modfunc, pluginres_t* presult, pluginfuncs_t* pluginfuncs, pluginvars_t* pluginvars) {
	QMM_SAVE_VARS();

	// make sure this DLL is loaded only in the right engine
	if (strcmp(QMM_GETGAMEENGINE(PLID), GAME_STR) != 0)
		return 0;

	return 1;
}


C_DLLEXPORT void QMM_Detach() {
}


#ifdef TEST_QVM_FUNC
// function pointer for old ent->die function
void (*old_die)(gentity_t*, gentity_t*, gentity_t*, int, int) = nullptr;

// new hook function
void new_die_real(gentity_t* self, gentity_t* inflictor, gentity_t* attacker, int damage, int mod) {
	QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "Oh no someone died! die(%p, %p, %p, %d, %d)\n", self, inflictor, attacker, damage, mod), QMMLOG_INFO);
	
	if (QMM_ISQVM(PLID)) {
		int args[5] = { SETPTR(self, int), SETPTR(inflictor, int), SETPTR(attacker, int), damage, mod };
		QMM_QVM_EXEC_FUNC(PLID, (int)old_die, 5, args);
	}
	else
		old_die(self, inflictor, attacker, damage, mod);
	QMM_WRITEQMMLOG(PLID, "After QVM call!\n", QMMLOG_INFO);
}

// function pointer for new ent->die function (default to new_die_real)
void (*new_die)(gentity_t*, gentity_t*, gentity_t*, int, int) = new_die_real;
#endif // TEST_QVM_FUNC


C_DLLEXPORT intptr_t QMM_vmMain(intptr_t cmd, intptr_t* args) {
	if (cmd == GAME_INIT) {
		QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "Test_QMM loaded! Game engine: %s\n", QMM_GETGAMEENGINE(PLID)), QMMLOG_INFO);
	}

#ifdef TEST_QVM_FUNC
	if (cmd == GAME_INIT) {
		// if QVM, register QVM func ID for new_die. don't overwrite pointer if not QVM
		if (QMM_ISQVM(PLID))
			new_die = (decltype(new_die))QMM_QVM_REGISTER_FUNC(PLID);
	}
	else if (cmd == GAME_CLIENT_THINK) {
		gentity_t* ent = ENT_FROM_NUM(args[0]);
		if (!ent)
			QMM_RET_IGNORED(0);
		gclient_t* client = GETPTR(ent->client, gclient_t*);
		// player just spawned, replace die function
		if (client && (client->ps.pm_flags & PMF_RESPAWNED) == PMF_RESPAWNED) {
			if (!old_die)
				old_die = ent->die;
			ent->die = new_die;
		}
	}
#endif // TEST_QVM_FUNC

#ifdef TEST_SOF2SP_GENTITY
	if (cmd == GAME_CONSOLE_COMMAND) {
		char arg0[20], arg1[20], arg2[20], arg3[20];
		QMM_ARGV(PLID, 0, arg0, sizeof(arg0));
		//health= client+208?
		//armor= client+212?
		if (!strcmp(arg0, "setclient")) {
			if (g_syscall(G_ARGC) != 3) {
				QMM_WRITEQMMLOG(PLID, "setclient <offset> <value>\n", QMMLOG_INFO);
				QMM_RET_SUPERCEDE(1);
			}
			QMM_ARGV(PLID, 1, arg1, sizeof(arg1));
			QMM_ARGV(PLID, 2, arg2, sizeof(arg2));
			int* x = (int*)((char*)g_clients + atoi(arg1));
			*x = atoi(arg2);
		}
		else if (!strcmp(arg0, "setent")) {
			if (g_syscall(G_ARGC) != 4) {
				QMM_WRITEQMMLOG(PLID, "setent <entnum> <offset> <value>\n", QMMLOG_INFO);
				QMM_RET_SUPERCEDE(1);
			}
			QMM_ARGV(PLID, 1, arg1, sizeof(arg1));
			QMM_ARGV(PLID, 2, arg2, sizeof(arg2));
			QMM_ARGV(PLID, 3, arg3, sizeof(arg3));
			int* x = (int*)((char*)g_gents + (atoi(arg1) * g_gentsize) + atoi(arg2));
			*x = atoi(arg3);
		}
		else if (!strcmp(arg0, "clientdump")) {
			//QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "client0=%p clientsize=%d\n", g_clients, g_clientsize), QMMLOG_INFO);
			for (int clientnum = 0; clientnum <= 0; clientnum++) {
				gclient_t* client = CLIENT_FROM_NUM(clientnum);
				for (int offset = 0; offset < g_clientsize; offset += 4) {
					int* x = (int*)((char*)client + offset);
					QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "client%d=%p client%d->%d=%X\n", clientnum, client, clientnum, offset, *x), QMMLOG_INFO);
				}
			}
		}
		else if (!strcmp(arg0, "entdump")) {
			for (int entnum = 0; entnum <= 10; entnum++) {
				gentity_t* ent = ENT_FROM_NUM(entnum);
				for (int offset = 0; offset < g_gentsize; offset += 4) {
					int* x = (int*)((char*)ent + offset);
					QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "ent%d=%p ent%d->%d=%X\n", entnum, ent, entnum, offset, *x), QMMLOG_INFO);
				}
			}
		}
		QMM_RET_SUPERCEDE(1);
	}
#endif // TEST_SOF2SP_GENTITY

#ifdef TEST_CFG
	if (cmd == GAME_INIT) {
		/*
		"test": {
			"int": 123,
			"bool" : true,
			"str" : "test string",
			"intarr" : [1, 2, 3] ,
			"strarr" : ["one", "two", "three"] ,
			"intarr2" : [] ,
			"strarr2" : [] ,
		},
		*/

		// scalars
		QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "test/int: %d\n", QMM_CFG_GETINT(PLID, "test/int")), QMMLOG_INFO);
		QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "test/int2: %d\n", QMM_CFG_GETINT(PLID, "test/int2")), QMMLOG_INFO);
		QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "test2/int: %d\n", QMM_CFG_GETINT(PLID, "test2/int")), QMMLOG_INFO);
		QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "test/bool: %d\n", QMM_CFG_GETBOOL(PLID, "test/bool")), QMMLOG_INFO);
		QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "test/bool2: %d\n", QMM_CFG_GETBOOL(PLID, "test/bool2")), QMMLOG_INFO);
		QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "test/str: %s\n", QMM_CFG_GETSTR(PLID, "test/str")), QMMLOG_INFO);
		QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "test/str2: %s\n", QMM_CFG_GETSTR(PLID, "test/str2")), QMMLOG_INFO);

		// int arr
		int* iarr = QMM_CFG_GETARRAYINT(PLID, "test/intarr");
		QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "test/intarr: %d %d %d %d %d\n", iarr[0], iarr[1], iarr[2], iarr[3], iarr[4]), QMMLOG_INFO);
		int* iarr2 = QMM_CFG_GETARRAYINT(PLID, "test/intarr2");
		QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "test/intarr2: %d %d\n", iarr2[0], iarr2[1]), QMMLOG_INFO);
		int* iarr3 = QMM_CFG_GETARRAYINT(PLID, "test/intarr3");
		QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "test/intarr3: %d %d\n", iarr3[0], iarr3[1]), QMMLOG_INFO);

		// str arr
		const char** sarr = QMM_CFG_GETARRAYSTR(PLID, "test/strarr");
		QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "test/strarr: %s %s %s %s\n", sarr[0], sarr[1], sarr[2], sarr[3]), QMMLOG_INFO);
		const char** sarr2 = QMM_CFG_GETARRAYSTR(PLID, "test/strarr2");
		QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "test/strarr2: %s\n", sarr2[0]), QMMLOG_INFO);
		const char** sarr3 = QMM_CFG_GETARRAYSTR(PLID, "test/strarr3");
		QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "test/strarr3: %s\n", sarr3[0]), QMMLOG_INFO);
	}
#endif // TEST_CFG

#ifdef TEST_RETURN_TEST
	if (cmd == GAME_CLIENT_CONNECT) {
		QMM_RET_SUPERCEDE((intptr_t)"Kicked, pre");
	}
#endif // TEST_RETURN_TEST

#ifdef TEST_COMMAND
	if (cmd == GAME_CLIENT_COMMAND) {
		char buf[16] = "";
		intptr_t clientnum = args[0];

		// some engines use this arg/buf/buflen syntax for G_ARGV while others return
		// the char*, so we use QMM_ARGV to handle both methods automatically
		QMM_ARGV(PLID, 0, buf, sizeof(buf));

		// example showing how to use infostrings
		if (!strcmp(buf, "myinfo")) {
			char userinfo[MAX_INFO_STRING];
			g_syscall(G_GET_USERINFO, clientnum, userinfo, sizeof(userinfo));
			const char* name = QMM_INFOVALUEFORKEY(PLID, userinfo, "name");
#if defined(GAME_Q2_ENGINE)
			g_syscall(G_CLIENT_PRINT, clientnum, PRINT_HIGH, QMM_VARARGS(PLID, "[STUB_QMM] Your name is: '%s'\n", name));
#else
			g_syscall(G_SEND_SERVER_COMMAND, clientnum, QMM_VARARGS(PLID, "print \"[STUB_QMM] Your name is: '%s'\"\n", name));
#endif
			QMM_RET_SUPERCEDE(1);
		}
#if !defined(GAME_Q2_ENGINE)
		// purely an example to show entity/client access and how it might be different per-game
		else if (!strcmp(buf, "myweapon")) {
			gclient_t* client = CLIENT_FROM_NUM(clientnum);
#if defined(GAME_STEF2)
			int left = client->ps.activeItems[ITEM_NAME_WEAPON_LEFT];
			int right = client->ps.activeItems[ITEM_NAME_WEAPON_RIGHT];
			g_syscall(G_SEND_SERVER_COMMAND, clientnum, QMM_VARARGS(PLID, "print \"[STUB_QMM] Your weapons are: %d %d\"\n", left, right));
#else
#if defined(GAME_MOHAA) || defined(GAME_MOHSH) || defined(GAME_MOHBT)
			int item = client->ps.activeItems[ITEM_WEAPON];
#else
			int item = client->ps.weapon;
#endif // MOHAA, MOHSH, MOHBT
			g_syscall(G_SEND_SERVER_COMMAND, clientnum, QMM_VARARGS(PLID, "print \"[STUB_QMM] Your weapon is: %d\"\n", item));
#endif // GAME_STEF2
			QMM_RET_SUPERCEDE(1);
		}
#endif // !GAME_Q2R && !GAME_QUAKE2
	}
#endif // TEST_COMMAND

	QMM_RET_IGNORED(0);
}


C_DLLEXPORT intptr_t QMM_syscall(intptr_t cmd, intptr_t* args) {
	if (cmd == G_LOCATE_GAME_DATA) {
		g_gents = (gentity_t*)(args[0]);
		g_numgents = args[1];
		g_gentsize = args[2];
		g_clients = (gclient_t*)(args[3]);
		g_clientsize = args[4];
		//QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "LocateGameData(%p, %d, %d, %p, %d)\n", g_gents, g_numgents, g_gentsize, g_clients, g_clientsize), QMMLOG_INFO);
	}

	QMM_RET_IGNORED(0);
}


C_DLLEXPORT intptr_t QMM_vmMain_Post(intptr_t cmd, intptr_t* args) {
#ifdef TEST_RETURN_TEST
	if (cmd == GAME_CLIENT_CONNECT) {
		QMM_RET_SUPERCEDE((intptr_t)"Kicked, post");
	}
#endif // TEST_RETURN_TEST

#ifdef TEST_BROADCAST
	// example of broadcasting a message to other plugins
	if (cmd == GAME_SHUTDOWN)
		QMM_PLUGIN_BROADCAST(PLID, "BYE", nullptr, 0);
#endif // TEST_BROADCAST

	QMM_RET_IGNORED(0);
}


C_DLLEXPORT intptr_t QMM_syscall_Post(intptr_t cmd, intptr_t* args) {
#ifdef TEST_RETURN_TEST
	if (cmd == G_ARGC) {
		QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "G_ARGC return value: %d\n", QMM_VAR_RETURN(intptr_t)), QMMLOG_INFO);
	}
#endif // TEST_RETURN_TEST

	QMM_RET_IGNORED(0);
}


C_DLLEXPORT void QMM_PluginMessage(plid_t from_plid, const char* message, void* buf, intptr_t buflen, int is_broadcast) {
#ifdef TEST_BROADCAST
	QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "Received plugin message \"%s\" with a %d-byte buffer", message, buflen), QMMLOG_INFO);
#endif // TEST_BROADCAST
}


C_DLLEXPORT int QMM_QVMHandler(int func, int* args) {
#ifdef TEST_QVM_FUNC
	QMM_WRITEQMMLOG(PLID, QMM_VARARGS(PLID, "QMM_QVMHandler(%d) called\n", func), QMMLOG_DEBUG);
	if (func == (int)new_die) {
		new_die_real(GETPTR(args[0], gentity_t*), GETPTR(args[1], gentity_t*), GETPTR(args[2], gentity_t*), args[3], args[4]);
	}
	#endif // TEST_QVM_FUNC

	return 0;
}
