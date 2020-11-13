#include "audio_source.hpp"

namespace audio {
static auto fifo_filter = "Fifo file(*.fifo);;"
						  "All Files (*.*)";

static bool add_source(void *data, obs_source_t *src)
{
	uint32_t caps = obs_source_get_output_flags(src);
	obs_property_t *list = (obs_property_t *)data;

	if ((caps & OBS_SOURCE_AUDIO) == 0)
		return true;
	const char *name = obs_source_get_name(src);
	obs_property_list_add_string(list, name, name);
	return true;
}

static bool source_selected(obs_properties_t *props, obs_property_t *, obs_data_t *data)
{
	auto *id = obs_data_get_string(data, S_AUDIO_SOURCE);
	auto *sr = obs_properties_get(props, S_SAMPLE_RATE);
	obs_property_t *fifo = nullptr;
#ifdef LINUX
	fifo = obs_properties_get(props, S_FIFO_PATH);
#endif
	if (strcmp(id, "mpd") == 0) {
		obs_property_set_visible(sr, true);
		if (fifo) {
			obs_property_set_visible(fifo, true);
		}
	}
	return true;
}

void audio_source::properties(obs_properties_t *props)
{
	auto *src =
		obs_properties_add_list(props, S_AUDIO_SOURCE, T_AUDIO_SOURCE, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_set_modified_callback(src, source_selected);

	obs_properties_add_color(props, S_COLOR, T_COLOR);

	obs_property_list_add_string(src, T_AUDIO_SOURCE_NONE, defaults::audio_source);
#ifdef LINUX
	/* Add MPD stuff */
	obs_property_list_add_string(src, T_SOURCE_MPD, "mpd");
	auto *path = obs_properties_add_path(props, S_FIFO_PATH, T_FIFO_PATH, OBS_PATH_FILE, fifo_filter, "");
	obs_property_set_visible(path, false);
	obs_properties_add_bool(props, S_AUTO_CLEAR, T_AUTO_CLEAR);
#endif

	obs_enum_sources(add_source, src);
}

void audio_source::resize_buffer()
{
	if (m_buffer)
		bfree(m_buffer);
	m_buffer = static_cast<pcm_stereo_sample *>(bzalloc(m_sample_size * sizeof(pcm_stereo_sample)));
}

long long audio_source::buffer_size()
{
	return m_sample_size;
}

void audio_source::update(obs_data_t *d)
{
	auto new_id = obs_data_get_string(d, S_AUDIO_SOURCE);
	if (m_source_id != new_id) {
		m_source_id = new_id;
		source_changed();
	}

	m_sample_rate = obs_data_get_int(d, S_SAMPLE_RATE);
	auto sample_size = obs_data_get_int(d, S_SAMPLE_SIZE);
	if (sample_size != m_sample_size) {
		resize_buffer();
	}
}
}
