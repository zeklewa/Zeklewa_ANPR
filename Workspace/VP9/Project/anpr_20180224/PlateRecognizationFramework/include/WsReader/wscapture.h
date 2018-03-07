
#include <stdio.h>
#include <string>
#include "decoder.h"

namespace vws {
// }
    void open(std::string url);
    frame_packet getImageInfo();

    bool isOpened();
    void restart(); // TBD
}