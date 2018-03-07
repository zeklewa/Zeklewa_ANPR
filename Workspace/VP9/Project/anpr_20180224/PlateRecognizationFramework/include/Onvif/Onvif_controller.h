#include <queue>
#include <stack>
#include <mutex>
#include <chrono>
#include <string>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "ObjectUtils.h"


namespace pr
{
class Onvif_controller
{
  public:
    Onvif_controller(std::queue<image_details> &queue_);
    void operator()();

  public:
    std::queue<image_details> &OnvifDetailsQueue;
    pr::ObjectUtils objects;
};
}
