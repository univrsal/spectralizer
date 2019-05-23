/**
 * This file is part of spectralizer
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectralizer
 */

#include "visualizer_source.hpp"
#include "../util/util.hpp"
#ifdef LINUX
#include <mpd/client.h>
#include <mpd/stats.h>

/* Keeps track of local mpd instance
 * which is needed to stop reading from the fifo,
 * while mpd is paused, since reading will block the
 * video threat otherwise*/
struct mpd_connection* local_mpd_connection = nullptr;
struct mpd_status* local_mpd_state = nullptr;

#endif
namespace source {

visualizer_source::visualizer_source(obs_source_t* source, obs_data_t* settings)
{
    m_config.settings = settings;
    m_config.source = source;

    /* Defaults are technically loaded by obs over update() */
    m_config.stereo = false;
    m_config.detail = 32;
    m_config.mode = VISUAL_BARS;
    m_config.refresh_rate = 1 / 30.f;
    m_config.refresh_counter = 0.f;
    m_config.sample_rate = 44100;
    m_config.bar_space = 5;
    m_config.bar_height = 100;
    m_config.bar_width = 10;
    m_config.cx = m_config.detail * (m_config.bar_width + m_config.bar_space) - m_config.bar_space;
    m_config.cy = m_config.bar_height;
    m_config.color = 0x440044ff;
    obs_source_update(source, settings);
}

visualizer_source::~visualizer_source()
{
    /* NO-OP (?)*/
}

void visualizer_source::update(obs_data_t* settings)
{
    auto test =  obs_data_get_int(settings, S_SAMPLE_RATE);
    UNUSED_PARAMETER(test);
    m_config.sample_rate        = (uint32_t) obs_data_get_int(settings, S_SAMPLE_RATE);
    m_config.fps                = UTIL_MAX(obs_data_get_int(settings, S_REFRESH_RATE), 1 );
    m_config.refresh_rate       = 1. / m_config.fps;
    m_config.audio_source       = obs_data_get_int(settings, S_AUDIO_SOURCE);
    m_config.mode               = static_cast<visual_mode>(obs_data_get_int(settings, S_SOURCE_MODE));
    m_config.stereo             = obs_data_get_bool(settings, S_STEREO);
    m_config.color              = obs_data_get_int(settings, S_COLOR);
    m_config.bar_width          = obs_data_get_int(settings, S_BAR_WIDTH);
    m_config.bar_space          = obs_data_get_int(settings, S_BAR_SPACE);
    m_config.detail             = obs_data_get_int(settings, S_DETAIL);
    m_config.fifo_path          = obs_data_get_string(settings, S_FIFO_PATH);
    m_config.bar_height         = obs_data_get_int(settings, S_BAR_HEIGHT);
    m_config.cx                 = UTIL_MAX(m_config.detail * (m_config.bar_width + m_config.bar_space) - m_config
            .bar_space, 10);
    m_config.cy                 = UTIL_MAX(m_config.bar_height, 10);
    m_config.mode               = static_cast<visual_mode>(obs_data_get_int(settings, S_BAR_FILTER_MODE));
    m_config.bar_filter_arg     = obs_data_get_double(settings, S_BAR_FILTER_ARG);
    m_renderer.setup(&m_config);
}

void visualizer_source::tick(float seconds)
{
    m_config.refresh_counter += seconds;
    if (m_config.refresh_counter >= m_config.refresh_rate) {
        m_config.refresh_counter = 0.f;
        /* Refresh data */
#ifdef LINUX
        local_mpd_state = mpd_run_status(local_mpd_connection);

        if (m_config.audio_source == 0 && mpd_status_get_state(local_mpd_state) == MPD_STATE_PLAY)
            m_renderer.read_fifo(&m_config);
#endif
    }
}

void visualizer_source::render(gs_effect_t* effect)
{
    gs_effect_t    *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
    gs_eparam_t    *color = gs_effect_get_param_by_name(solid, "color");
    gs_technique_t *tech  = gs_effect_get_technique(solid, "Solid");

    struct vec4 colorVal;
    vec4_from_rgba(&colorVal, m_config.color);
    gs_effect_set_vec4(color, &colorVal);

    gs_technique_begin(tech);
    gs_technique_begin_pass(tech, 0);

    if (m_config.audio_source == 0) {
        m_renderer.render_fifo(effect, &m_config);
    } else {

    }

    gs_technique_end_pass(tech);
    gs_technique_end(tech);
}

static auto fifo_filter = "Fifo file(*.fifo);;"
                          "All Files (*.*)";

obs_properties_t* get_properties_for_visualiser(void* data)
{
    UNUSED_PARAMETER(data);
    obs_properties_t* props = obs_properties_create();

    auto mode = obs_properties_add_list(props, S_SOURCE_MODE, T_SOURCE_MODE, OBS_COMBO_TYPE_LIST,
            OBS_COMBO_FORMAT_INT);
    obs_property_list_add_int(mode, T_MODE_BARS, VISUAL_BARS);
    obs_property_list_add_int(mode, T_MODE_WIRE, VISUAL_WIRE);

    auto src = obs_properties_add_list(props, S_AUDIO_SOURCE, T_AUDIO_SOURCE, OBS_COMBO_TYPE_LIST,
                                        OBS_COMBO_FORMAT_INT);
    auto filter = obs_properties_add_list(props, S_BAR_FILTER_MODE, T_BAR_FILTER_MODE, OBS_COMBO_TYPE_LIST,
                                        OBS_COMBO_FORMAT_INT);

    obs_property_list_add_int(filter, T_BAR_FILTER_MODE_NONE, BAR_FILTER_NONE);
    obs_property_list_add_int(filter, T_BAR_FILTER_MODE_MONSTERCAT, BAR_FILTER_MONSTERCAT);
    obs_property_list_add_int(filter, T_BAR_FILTER_MODE_WAVES, BAR_FILTER_WAVES);

    obs_properties_add_float_slider(props, S_BAR_FILTER_ARG, T_BAR_FILTER_ARG, 0, 15, 0.1);

    obs_properties_add_int(props, S_BAR_WIDTH, T_BAR_WIDTH, 1, UINT16_MAX, 1);
    obs_properties_add_int(props, S_BAR_HEIGHT, T_BAR_HEIGHT, 10, UINT16_MAX, 1);
    obs_properties_add_int(props, S_BAR_SPACE, T_BAR_SPACING, 0, UINT16_MAX, 1);
    obs_properties_add_int(props, S_SAMPLE_RATE, T_SAMPLE_RATE, 128, UINT16_MAX, 10);
    obs_properties_add_color(props, S_COLOR, T_COLOR);

    int audio_source_index = 0;
#ifdef LINUX
    /* Add MPD stuff */
    if (local_mpd_connection) {
        obs_property_list_add_int(src, T_SOURCE_MPD, audio_source_index++);
        obs_properties_add_path(props, S_FIFO_PATH, T_FIFO_PATH, OBS_PATH_FILE, fifo_filter, "");
    }
#endif

    /* Go through all audio sources ... */

    obs_properties_add_bool(props, S_STEREO, T_STEREO);
    obs_properties_add_int(props, S_DETAIL, T_DETAIL, 1, UINT16_MAX, 1);

    obs_properties_add_int(props, S_REFRESH_RATE, T_REFRESH_RATE, 1, 255, 5);
    return props;
}

void register_visualiser()
{

#ifdef LINUX
    /* Connect to mpd, otherwise disable fifo */
    local_mpd_connection = mpd_connection_new(nullptr, 0, 0);
#endif

    obs_source_info si = {};
    si.id = "spectralizer";
    si.type = OBS_SOURCE_TYPE_INPUT;
    si.output_flags = OBS_SOURCE_VIDEO;
    si.get_properties = get_properties_for_visualiser;

    si.get_name = [](void*)
    { return T_SOURCE; };
    si.create = [](obs_data_t* settings, obs_source_t* source)
    {
        return static_cast<void*>(new visualizer_source(source, settings));
    };
    si.destroy = [](void* data)
    {
        delete reinterpret_cast<visualizer_source*>(data);
    };
    si.get_width = [](void* data)
    {
        return reinterpret_cast<visualizer_source*>(data)->get_width();
    };
    si.get_height = [](void* data)
    {
        return reinterpret_cast<visualizer_source*>(data)->get_height();
    };

    si.get_defaults = [](obs_data_t* settings)
    {
        obs_data_set_default_int(settings, S_DETAIL, 32);
        obs_data_set_default_double(settings, S_REFRESH_RATE, 30);
        obs_data_set_default_bool(settings, S_STEREO, false);
        obs_data_set_default_int(settings, S_SOURCE_MODE, VISUAL_BARS);
        obs_data_set_default_int(settings, S_AUDIO_SOURCE, 0);
        obs_data_set_default_int(settings, S_SAMPLE_RATE, 44100);
        obs_data_set_default_int(settings, S_BAR_FILTER_MODE, BAR_FILTER_NONE);
        obs_data_set_default_double(settings, S_BAR_FILTER_ARG, 0);
    };

    si.update = [](void* data, obs_data_t* settings)
    {
        reinterpret_cast<visualizer_source*>(data)->update(settings);
    };
    si.video_tick = [](void* data, float seconds)
    {
        reinterpret_cast<visualizer_source*>(data)->tick(seconds);
    };
    si.video_render = [](void* data, gs_effect_t* effect)
    {
        reinterpret_cast<visualizer_source*>(data)->render(effect);
    };

    obs_register_source(&si);
}

    void cleanup()
    {
#ifdef LINUX
        mpd_status_free(local_mpd_state);
        mpd_connection_free(local_mpd_connection);
        local_mpd_state = nullptr;
        local_mpd_connection = nullptr;
#endif
    }

}