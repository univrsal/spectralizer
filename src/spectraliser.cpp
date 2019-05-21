/**
 * This file is part of spectraliser
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectraliser
 */

#include <obs-module.h>
#include "source/visualiser_source.hpp"

#define blog(log_level, format, ...)                            \
	blog(log_level, "[spectraliser: '%s'] " format,             \
         obs_source_get_name(context->source), ##__VA_ARGS__)

#define debug(format, ...)                      \
	blog(LOG_DEBUG, format, ##__VA_ARGS__)
#define info(format, ...)                       \
	blog(LOG_INFO, format, ##__VA_ARGS__)
#define warn(format, ...)                       \
	blog(LOG_WARNING, format, ##__VA_ARGS__)

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("spectraliser", "en-US")

bool obs_module_load()
{
    source::register_visualiser();

    return true;
}

void obs_module_unload()
{

}
