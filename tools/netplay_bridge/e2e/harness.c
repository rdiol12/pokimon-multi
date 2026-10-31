// Runs several mGBA instances of the ROM side by side, headless, each with the
// netplay bridge and a test driver script loaded, for end-to-end tests.
//
// Usage: [NETPLAY_PORT=n] harness <rom> <bridge.lua> <frames> <instance-dir>...
// Each instance dir holds driver.lua; the save file (game.sav) is created there.
// The scripts see INSTANCE (1-based), INSTANCE_DIR and NETPLAY_PORT as globals,
// and the run ends early once every driver sets HARNESS_DONE = true.
// Run it through run_e2e.py, which also starts the server.

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <mgba/core/core.h>
#include <mgba/core/log.h>
#include <mgba/core/scripting.h>
#include <mgba/script/context.h>
#include <mgba/internal/script/lua.h>
#include <mgba-util/vfs.h>

#define MAX_INSTANCES 4
#define GBA_WIDTH 240
#define GBA_HEIGHT 160

struct Instance {
	struct mCore* core;
	struct mScriptContext context;
	struct mScriptEngineContext* lua;
	color_t* video;
};

static void logStdout(struct mLogger* logger, int category, enum mLogLevel level, const char* format, va_list args) {
	(void) logger;
	(void) category;
	if (level & (mLOG_FATAL | mLOG_ERROR | mLOG_WARN | mLOG_INFO)) {
		vprintf(format, args);
		putchar('\n');
		fflush(stdout);
	}
}

static struct mLogger sLogger = { .log = logStdout };

static bool runLuaVF(struct mScriptEngineContext* lua, const char* name, struct VFile* vf) {
	if (!vf || !lua->load(lua, name, vf) || !lua->run(lua)) {
		fprintf(stderr, "%s: %s\n", name, lua->getError(lua));
		return false;
	}
	return true;
}

static bool runLua(struct mScriptEngineContext* lua, const char* name, const char* code) {
	return runLuaVF(lua, name, VFileFromConstMemory(code, strlen(code)));
}

static bool isDone(struct mScriptEngineContext* lua) {
	struct mScriptValue* done = lua->getGlobal(lua, "HARNESS_DONE");
	bool result = done && done->type == mSCRIPT_TYPE_MS_BOOL && done->value.u32;
	if (done)
		mScriptValueDeref(done);
	return result;
}

int main(int argc, char** argv) {
	if (argc < 5) {
		fprintf(stderr, "usage: %s <rom> <bridge.lua> <frames> <instance-dir>...\n", argv[0]);
		return 2;
	}
	const char* rom = argv[1];
	const char* bridge = argv[2];
	long frames = strtol(argv[3], NULL, 10);
	int count = argc - 4;
	if (count > MAX_INSTANCES)
		count = MAX_INSTANCES;

	setvbuf(stdout, NULL, _IOLBF, 0);
	mLogSetDefaultLogger(&sLogger);

	struct Instance instances[MAX_INSTANCES] = {0};
	for (int i = 0; i < count; i++) {
		struct Instance* inst = &instances[i];
		const char* dir = argv[4 + i];
		char path[1024];

		inst->core = mCoreFind(rom);
		if (!inst->core || !inst->core->init(inst->core)) {
			fprintf(stderr, "cannot create core for %s\n", rom);
			return 1;
		}
		mCoreInitConfig(inst->core, NULL);
		inst->video = calloc(GBA_WIDTH * GBA_HEIGHT, sizeof(color_t));
		inst->core->setVideoBuffer(inst->core, inst->video, GBA_WIDTH);
		if (!mCoreLoadFile(inst->core, rom)) {
			fprintf(stderr, "cannot load %s\n", rom);
			return 1;
		}
		snprintf(path, sizeof(path), "%s/game.sav", dir);
		inst->core->loadSave(inst->core, VFileOpen(path, O_CREAT | O_RDWR));
		inst->core->reset(inst->core);

		mScriptContextInit(&inst->context);
		mScriptContextAttachStdlib(&inst->context);
		mScriptContextAttachSocket(&inst->context);
		inst->lua = mScriptContextRegisterEngine(&inst->context, mSCRIPT_ENGINE_LUA);
		mScriptContextAttachCore(&inst->context, inst->core);
		mScriptContextAttachLogger(&inst->context, &sLogger);

		char setup[512];
		const char* port = getenv("NETPLAY_PORT");
		snprintf(setup, sizeof(setup), "INSTANCE = %d\nINSTANCE_DIR = \"%s\"\nNETPLAY_PORT = %s\n",
		    i + 1, dir, port ? port : "nil");
		if (!runLua(inst->lua, "setup.lua", setup)
		 || !runLuaVF(inst->lua, bridge, VFileOpen(bridge, O_RDONLY)))
			return 1;
		snprintf(path, sizeof(path), "%s/driver.lua", dir);
		if (!runLuaVF(inst->lua, path, VFileOpen(path, O_RDONLY)))
			return 1;
	}

	// NETPLAY_FPS: runs no faster than that, for scenarios timed by the
	// server's clock (hide and seek). Otherwise as fast as it can.
	const char* fpsText = getenv("NETPLAY_FPS");
	long frameNs = fpsText ? 1000000000L / atol(fpsText) : 0;
	struct timespec next;
	clock_gettime(CLOCK_MONOTONIC, &next);

	long frame;
	for (frame = 0; frame < frames; frame++) {
		if (frameNs > 0) {
			next.tv_nsec += frameNs;
			while (next.tv_nsec >= 1000000000L) {
				next.tv_nsec -= 1000000000L;
				next.tv_sec++;
			}
			clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
		}
		bool allDone = true;
		for (int i = 0; i < count; i++) {
			instances[i].core->runFrame(instances[i].core);
			mScriptContextTriggerCallback(&instances[i].context, "frame");
			if (!isDone(instances[i].lua))
				allDone = false;
		}
		if (allDone)
			break;
	}
	printf("harness: ran %ld frames\n", frame);

	// Flush saves; tearing down the script contexts is left to process exit.
	for (int i = 0; i < count; i++)
		instances[i].core->deinit(instances[i].core);
	_exit(0);
}
