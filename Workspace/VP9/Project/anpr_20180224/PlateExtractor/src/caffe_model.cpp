#include "caffe_model.h"
using std::cout;
using std::endl;
using std::string;
using std::vector;
using namespace caffe2;
using namespace cv;
CNN::CNN(string init_model, string predict_model, int width, int height) : width(width), height(height)
{
    CAFFE_ENFORCE(ReadProtoFromFile(init_model, &init_net));
    CAFFE_ENFORCE(ReadProtoFromFile(predict_model, &predict_net));
    predictor = new Predictor(init_net, predict_net);
    google::protobuf::ShutdownProtobufLibrary();
}

vector<float> CNN::predict(const Mat &input_mat)
{
    TensorCPU input = mat2data(input_mat);
    Predictor::TensorVector inputVec({&input}), outputVec;
    predictor->run(inputVec, &outputVec);
    auto &output = *(outputVec[0]);
    const auto probs = output.data<float>();
    vector<float> rep;
    for (auto i = 0; i < output.size(); i++)
        rep.push_back(probs[i]);
    return rep;
}

TensorCPU CNN::mat2data(const Mat &img)
{
    cv::Mat img_resize;
    cv::resize(img, img_resize, Size(width, height));
    img_resize.convertTo(img_resize, CV_32FC3, 1. / 255, 0);
    vector<cv::Mat> channels(3);
    cv::split(img_resize, channels);
    std::vector<float> data;
    for (auto &c : channels)
    {
        data.insert(data.end(), (float *)c.datastart, (float *)c.dataend);
    }
    std::vector<TIndex> dims({1, img_resize.channels(), img_resize.rows, img_resize.cols});
    TensorCPU input(dims, data, NULL);
    return input;
}