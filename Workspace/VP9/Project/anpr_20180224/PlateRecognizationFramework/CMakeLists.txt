project(PlateRecognizationFramework VERSION 1.0.0)

include_directories(
    ./include 
    ../PlateDetector/include 
    ../PlateExtractor/include
    ./include/WsReader
    ./include/GroupPlates
    ./include/Crop
    ./include/Onvif
)

add_executable(PlateRecognizationFramework
    Main.cpp
    src/IOData.cpp
    src/ObjectUtils.cpp
    src/VideoReader.cpp
    src/PlateRecognizator.cpp
    src/PlateReader.cpp
    src/PlateRecognitionAction.cpp
    src/Crop.cpp
    src/connectCMS.cpp
    src/estimate_square_cascade.cpp
    src/bckgrnd_subs.cpp
    
    src/GroupPlates/group_plates.cpp
    src/GroupPlates/tracking_thanhnn.cpp
    src/GroupPlates/r_distance.cpp

    src/WsReader/cvBufferCapture.cpp
    src/WsReader/decoder.cpp
    src/WsReader/libdownload.cpp
    src/WsReader/util.cpp
    src/WsReader/wscapture.cpp
    src/WsReader/wsclient.cpp

    src/Crop/common_function.cpp
    src/Crop/crop_char_long.cpp
    src/Crop/crop_char_square.cpp
    src/Crop/crop_longplate_corner.cpp
    src/Crop/cropping_v2_color.cpp
    src/Crop/cropping_v2.cpp
    src/Crop/cropping_v3_color.cpp
    src/Crop/cropping_v3.cpp

    src/Onvif/Onvif.cpp
    src/Onvif/Onvif_controller.cpp
    src/Onvif/xmlParser.cpp
)


target_link_libraries(PlateRecognizationFramework
    PlateDetector
    PlateExtractor
    ${OpenCV_LIBS}
    ${GCC_COVERAGE_COMPILE_FLAGS}
    pthread
    curl
    json
    websockets
    avcodec
    avformat
    avutil
    swscale
)
