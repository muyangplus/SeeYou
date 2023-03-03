// C++
#include <iostream>
#include <map>
#include <random>
#include <filesystem>
// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
// Self
#include  "src/cnn/pipeline.h"



namespace {
    void cv_show(const cv::Mat& one_image, const char* info = "") {
        cv::imshow(info, one_image);
        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    bool cv_write(const cv::Mat& source, const std::string save_path) {
        return cv::imwrite(save_path, source, std::vector<int>({ cv::IMWRITE_PNG_COMPRESSION, 0 }));
    }

    cv::Mat rotate(cv::Mat& src, double angle) {
        // ���� https://stackoverflow.com/questions/22041699/rotate-an-image-without-cropping-in-opencv-in-c
        // �Ƕ���������� 15-75 ֮��, ����������������
        cv::Point2f center((src.cols - 1) / 2.0, (src.rows - 1) / 2.0);
        cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
        cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), src.size(), angle).boundingRect2f();
        rot.at<double>(0, 2) += bbox.width / 2.0 - src.cols / 2.0;
        rot.at<double>(1, 2) += bbox.height / 2.0 - src.rows / 2.0;
        cv::warpAffine(src, src, rot, bbox.size());
        return src;
    }
}


using namespace pipeline;



void ImageAugmentor::make_augment(cv::Mat& origin, const bool show) {
    // ���ȴ��Ҳ�����˳��
    std::shuffle(ops.begin(), ops.end(), this->l);
    // �����������
    for (const auto& item : ops) {
        // ���һ���������
        const float prob = engine(e);
        // ��������㹻��, ִ�в���
        if (prob >= 1.0 - item.second) {
            // ����ת
            if (item.first == "hflip")
                cv::flip(origin, origin, 1);
            else if (item.first == "vflip")
                cv::flip(origin, origin, 0);
            else if (item.first == "crop") {
                // ��ȡͼ����Ϣ
                const int H = origin.rows;
                const int W = origin.cols;
                // ��ȡ��ȡ�ı���
                float crop_ratio = 0.7f + crop_engine(c);
                const int _H = int(H * crop_ratio);
                const int _W = int(W * crop_ratio);
                // ��ȡ��ȡ��λ��
                std::uniform_int_distribution<int> _H_pos(0, H - _H);
                std::uniform_int_distribution<int> _W_pos(0, W - _W);
                // ��ʼ��ȡͼ��
                origin = origin(cv::Rect(_W_pos(c), _H_pos(c), _W, _H)).clone();
            }
            else if (item.first == "rotate") {
                // ��ȡһ���漴�Ƕ�
                float angle = rotate_engine(r);
                if (minus_engine(r) & 1) angle = -angle;
                origin = rotate(origin, angle);
            }
            if (show == true) cv_show(origin);
        }
    }
}



std::map<std::string, pipeline::list_type> pipeline::get_images_for_classification(
    const std::filesystem::path dataset_path,
    const std::vector<std::string> categories,
    const std::pair<float, float> ratios) {
    // ���� dataset_path �ļ�����ָ�������
    list_type all_images_list;
    const int categories_num = categories.size();
    for (int i = 0; i < categories_num; ++i) {
        const auto images_dir = dataset_path / categories[i];
        assert(std::filesystem::exists(images_dir) && std::string(images_dir.string() + " ·��������!").c_str());
        auto walker = std::filesystem::directory_iterator(images_dir);
        for (const auto& iter : walker)
            all_images_list.emplace_back(iter.path().string(), i);
    }
    // ����ͼ���б�
    std::shuffle(all_images_list.begin(), all_images_list.end(), std::default_random_engine(212));
    // �����ݼ����ֳ�������
    const int total_size = all_images_list.size();
    assert(ratios.first > 0 && ratios.second > 0 && ratios.first + ratios.second < 1);
    const int train_size = int(total_size * ratios.first);
    const int test_size = int(total_size * ratios.second);
    std::map<std::string, list_type> results;
    results.emplace("train", list_type(all_images_list.begin(), all_images_list.begin() + train_size));
    results.emplace("test", list_type(all_images_list.begin() + train_size, all_images_list.begin() + train_size + test_size));
    results.emplace("valid", list_type(all_images_list.begin() + train_size + test_size, all_images_list.end()));
    std::cout << "train  :  " << results["train"].size() << "\n" << "test   :  " << results["test"].size() << "\n" << "valid  :  " << results["valid"].size() << "\n";
    return results;
}



DataLoader::DataLoader(const list_type& _images_list, const int _bs, const bool _aug, const bool _shuffle, const std::tuple<int, int, int> image_size, const int _seed)
    : images_list(_images_list),
    batch_size(_bs),
    augment(_aug),
    shuffle(_shuffle),
    H(std::get<0>(image_size)),
    W(std::get<1>(image_size)),
    C(std::get<2>(image_size)),
    seed(_seed) {
    this->images_num = this->images_list.size();  // �ܹ��ж�����ͼ��
    this->buffer.reserve(this->batch_size);  // Ԥ���ռ�
    for (int i = 0; i < this->batch_size; ++i) // ÿ��ͼ��
        this->buffer.emplace_back(new Tensor3D(C, H, W));
}

int DataLoader::length() const {
    return this->images_num;
}

//
DataLoader::batch_type DataLoader::generate_batch() {
    std::vector<tensor> images;
    std::vector<int> labels;
    images.reserve(this->batch_size);
    labels.reserve(this->batch_size);
    for (int i = 0; i < this->batch_size; ++i) {
        auto sample = this->add_to_buffer(i);
        images.emplace_back(sample.first);
        labels.emplace_back(sample.second);
    }
    return std::make_pair(std::move(images), std::move(labels));
}

// ��ȡ�� batch_index ��ͼ����Ϣ, ���� tensor
std::pair<tensor, int> DataLoader::add_to_buffer(const int batch_index) {
    // ��ȡͼ�����
    ++this->iter;
    if (this->iter == this->images_num) { // ȡ��ͷ��, ���¿�ʼ
        this->iter = 0;  // �±���Ϊ 0
        if (this->shuffle) { // Ȼ����һ�δ����б�
            std::shuffle(this->images_list.begin(), this->images_list.end(), std::default_random_engine(this->seed)); // std::cout << this->images_list[0].first << ", " << this->images_list[0].second << std::endl;
        }
    }
    // ��ȡͼ��
    const auto& image_path = this->images_list[this->iter].first;
    const int image_label = this->images_list[this->iter].second;
    cv::Mat origin = cv::imread(image_path);
    // ��ͼ����������ǿ
    if (this->augment) this->augmentor.make_augment(origin);
    // resize, ������������ǿ֮��
    cv::resize(origin, origin, { W, H });
    // ֱ�Ӷ� buffer �������, ��ͼ��ת���� float ����, ���� Tensor ��ʽ, C x H x W;
    // printf("%s%02x\n","Tensor����ָ�룺", buffer[batch_index]);
    this->buffer[batch_index]->read_from_opencv_mat(origin.data);
    // Tensor�ռ��С����ͼƬ�Ŀ�(W)������
    // printf("%s%i\n","Tensor�ռ��С��",origin.size());
    //this->buffer[batch_index]->forword(buffer);
    // ����ͼ�����ݺͱ�ǩ
    return std::pair<tensor, int>(this->buffer[batch_index], image_label);
}


