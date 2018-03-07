project(PlateExtractor VERSION 1.0.0)

include_directories(./include ../PlateDetector/include)

file(GLOB PlateExtractor_SRC
    "src/*.cpp"
)
find_package(Protobuf REQUIRED)

find_library(CAFFE2_LIB caffe2)

add_library(PlateExtractor
    ${PlateExtractor_SRC}
)

target_link_libraries(PlateExtractor ${CAFFE2_LIB} ${PROTOBUF_LIBRARY})