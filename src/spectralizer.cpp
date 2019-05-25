/**
 * This file is part of spectralizer
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectralizer
 */

#include <obs-module.h>
#include "source/visualizer_source.hpp"

OBS_DECLARE_MODULE()

OBS_MODULE_USE_DEFAULT_LOCALE("spectralizer", "en-US")

bool obs_module_load()
{
    source::register_visualiser();
    return true;
}

void obs_module_unload()
{
    /* TODO: MPD disconnecting? */
}
