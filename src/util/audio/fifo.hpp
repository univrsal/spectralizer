/**
 * This file is part of spectraliser
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectraliser
 */

/* Heavily based on
 * https://github.com/karlstav/cava/blob/master/input/fifo.c */
#ifdef LINUX
#include "audio_processor.hpp"
#include <mutex>

class fifo : public audio_processor
{
private:

    pthread_t m_read_thread;
    bool m_thread_state = false;
    const char* m_file_path = nullptr;
public:
    std::mutex m_data_mutex;
    int m_fifo_handle = 0;

    void update(source::config* cfg) override;
    void close_processor() override;
    bool open_fifo();
    bool running() const;
};

void* read_thread_method(void* arg);
#endif //LINUX
