/**
 * This file is part of spectralizer
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectralizer
 */

#ifdef LINUX

#include "audio_processor.hpp"
#include <mutex>

namespace audio
{

    class fifo : public audio_processor
    {
    private:
        const char* m_file_path = nullptr;
        std::mutex m_data_mutex;
        int m_fifo_handle = 0;
        /* Reading method variables
         * They are stored outside of the function to preserve their values
         * because in cava reading is done in a while loop instead of a tick method */
        int16_t buffer[BUFFER_SIZE];
        float m_retry_timer = 0.f; /* Tries to reopen fifo every 5 seconds*/
        int n = 0,          /* audio_out array index */
                i,          /* Loop index */
                c = 0,      /* Buffer clean counter */
                bytes = 0;  /* Bytes read from fifo */

        bool open_fifo();
    public:
        fifo(source::config* cfg);

        void update(source::config* cfg) override;
        void tick(float seconds, source::config* cfg) override;
        void clean_up() override;
    };

}
#endif //LINUX
