#include "App.h"

#include <stdlib.h>

enum MainStates
{
	MAIN_CREATION,
	MAIN_AWAKE,
	MAIN_START,
	MAIN_UPDATE,
	MAIN_CLEAN,
	MAIN_FAIL,
	MAIN_EXIT
};

int main(int argc, char* argv[])
{
	LOG(LogType::LOG_INFO, "Starting Engine");

	MainStates state = MAIN_CREATION;
	int result = EXIT_FAILURE;
	App* app = NULL;

	while (state != MAIN_EXIT)
	{
		switch (state)
		{
		case MAIN_CREATION:
			LOG(LogType::LOG_INFO, "-------------- Application Creation --------------");

			app = new App(argc, argv);

			if (app != NULL)
				state = MAIN_AWAKE;
			else
				state = MAIN_FAIL;

			break;

		case MAIN_AWAKE:
			LOG(LogType::LOG_INFO, "-------------- Application Awake --------------");

			if (app->Awake() == true)
				state = MAIN_START;
			else
				state = MAIN_FAIL;

			break;

		case MAIN_START:
			LOG(LogType::LOG_INFO, "-------------- Application Start --------------");

			if (app->Start() == true)
			{
				LOG(LogType::LOG_INFO, "-------------- Application Update --------------");
				state = MAIN_UPDATE;
			}
			else
				state = MAIN_FAIL;

			break;

		case MAIN_UPDATE:
			if (app->Update() == false || app->exit)
				state = MAIN_CLEAN;

			break;

		case MAIN_CLEAN:
			LOG(LogType::LOG_INFO, "-------------- Application CleanUp --------------");

			if (app->CleanUp() == true)
			{
				delete app;
				result = EXIT_SUCCESS;
				state = MAIN_EXIT;
			}
			else
				state = MAIN_FAIL;

			break;

		case MAIN_FAIL:
			LOG(LogType::LOG_ERROR, "-------------- Application Failed --------------");

			result = EXIT_FAILURE;
			state = MAIN_EXIT;
			break;

		default:
			break;
		}
	}
	return result;
}