// C++
#include <string>
#include <iostream>
#include <filesystem>
// OpenCV
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
// Self
#include "src/cnn/func.h"
#include "src/cnn/architectures.h"
#include "src/cnn/inference_commands.h"

namespace {
    void cv_show(const cv::Mat& one_image, const char* info = "") {
        cv::imshow(info, one_image);
        cv::waitKey(0);
        cv::destroyAllWindows();
    }
}

int main(int argc, char* argv[]) {

    switch (int_commands(argc, argv)) {
    case 0:
        command_show_info();
        break;
    case 1:
        return 0;
    case 2: 
        command_unkown();
        return 1;
    }
    
    // 输出不要放在缓冲区, 到时间了及时输出
    setvbuf(stdout, 0, _IOFBF, 1024);
    
    using namespace architectures;
    
    // 指定一些参数
    //const std::vector<std::string> categories({ "dog", "panda", "bird" });
    
    // 定义网络结构
    const int num_classes = categories.size(); // 分类的数目
    AlexNet network(num_classes);
    
    // 直接加载
    network.load_weights(filePath);
    
    // 准备测试的图片
    //std::vector<std::string> images_list({
    //    "./datasets/images/test_1.jpg",
    //    "./datasets/images/test_2.jpg",
    //    "./datasets/images/test_3.jpg",
    //    "./datasets/images/test_4.jpg"
    //    });
    
    // 准备一块图像内容存放的空间
    const std::tuple<int, int, int> image_size({ 3, 224, 224 });
    tensor buffer_data(new Tensor3D(image_size, "inference_buffer"));
    std::vector<tensor> image_buffer({ buffer_data });
    
    // 去掉梯度计算
    WithoutGrad guard;
    
    // 逐一读取图像, 做变换
    for (const auto& image_path : images_list) {
        // 读取图像
        cv::Mat origin = cv::imread(image_path);
        if (origin.empty() || !std::filesystem::exists(image_path)) {
            std::cout << "图像 " << image_path << " 读取失败 !\n";
            continue;
        }
        // 图像 resize 到规定的大小, 224 X 224
        cv::resize(origin, origin, { std::get<1>(image_size), std::get<2>(image_size) });
        // 转化为 tensor 数据
        image_buffer[0]->read_from_opencv_mat(origin.data);
        // 经过卷积神经网络得到输出
        const auto output = network.forward(image_buffer);
        // softmax 得到输出
        const auto prob = softmax(output);
        prob[0]->print();
        // 找到最大概率的输出
        const int max_index = prob[0]->argmax();
        std::cout << "分类结果是  :  " << categories[max_index] << ", 概率 " << prob[0]->data[max_index] << std::endl;
        cv_show(origin);
    }
    return 0;
}


