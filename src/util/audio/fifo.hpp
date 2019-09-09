/**
 * This file is part of spectralizer
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectralizer
 */

#ifdef LINUX

#include "audio_source.hpp"
#include <poll.h>

namespace audio
{

    class fifo : public audio_source
    {
    private:
        const char* m_file_path = nullptr;
        int m_fifo_fd = 0;
        bool open_fifo();
    public:
        fifo(source::config* cfg);

        void update(source::config* cfg) override;
        bool tick(float seconds, source::config* cfg) override;
        void clean_up() override;
    };

}
#endif //LINUX
