#include <string.h>
#include <assert.h>
extern "C" {
#include "SPConfig.h"
#include "SPLogger.h"
#include "SPConfigUnitTest.h"
}

#define DEFAULT_CONFIG_FILE	"spcbir.config"

/*int main(int argc, char** argv) {
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	SPConfig config;
	const char* configFilename;
	char* givenImagePath;
	SP_LOGGER_MSG loggerMsg;

	if (argc == 1) {
		configFilename = DEFAULT_CONFIG_FILE;
		config = spConfigCreate(configFilename, &msg);
		if (msg == SP_CONFIG_CANNOT_OPEN_FILE)
			printf("The default configuration file spcbir.config couldn’t be open\n");
	}
	else if (argc == 3 && !strcmp(argv[1], "-c")) {
		configFilename = argv[2];
		config = spConfigCreate(configFilename, &msg);
		if (msg == SP_CONFIG_CANNOT_OPEN_FILE)
			printf("The configuration file %s couldn’t be open\n", configFilename);
	}
	else {
		printf("Invalid command line : use -c <config_filename>\n");
		//exit - maybe return something...
	}

	switch (config->spLoggerLevel) {
	case 1:
		loggerMsg = spLoggerCreate(config->spLoggerFilename, SP_LOGGER_ERROR_LEVEL);
		break;
	case 2:
		loggerMsg = spLoggerCreate(config->spLoggerFilename, SP_LOGGER_WARNING_ERROR_LEVEL);
		break;
	case 3:
		loggerMsg = spLoggerCreate(config->spLoggerFilename, SP_LOGGER_INFO_WARNING_ERROR_LEVEL);
		break;
	case 4:
		loggerMsg = spLoggerCreate(config->spLoggerFilename, SP_LOGGER_DEBUG_INFO_WARNING_ERROR_LEVEL);
		break;
	}
	assert(loggerMsg == SP_LOGGER_SUCCESS);

	// call extract/non extract function according to Extraction Mode field in the config struct

	printf("Please enter an image path:\n");
	gets(givenImagePath);

	// call "Search Similar Images" on givenImagePath

	return 0;
}*/

int main() {
	testGivenConfFile();
	return 0;
}

