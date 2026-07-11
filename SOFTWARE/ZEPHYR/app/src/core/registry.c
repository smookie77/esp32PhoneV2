#include <string.h>
#include <phone/app.h>

size_t phone_app_count(void)
{
	size_t count;

	STRUCT_SECTION_COUNT(phone_app, &count);
	return count;
}

const struct phone_app *phone_app_by_index(size_t idx)
{
	const struct phone_app *app;

	if (idx >= phone_app_count()) {
		return NULL;
	}
	STRUCT_SECTION_GET(phone_app, idx, &app);
	return app;
}

const struct phone_app *phone_app_find(const char *name)
{
	STRUCT_SECTION_FOREACH(phone_app, app) {
		if (strcmp(app->name, name) == 0) {
			return app;
		}
	}
	return NULL;
}
