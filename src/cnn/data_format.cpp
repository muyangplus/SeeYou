// C++
#include <tuple>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>
#include <iostream>
// Self
#include "src/cnn/data_format.h"

// ��ͼ��ָ���ж�ȡ
void Tensor3D::read_from_opencv_mat(const uchar* const img_ptr) {
    // �� img_ptr �����л�ȡͼ������
    const int length = H * W;
    const int length_2 = 2 * length;
    for (int i = 0; i < length; ++i) { // OpenCV ���ڴ�����˳�������
        const int p = 3 * i;
        this->data[i] = img_ptr[p] * 1.f / 255;
        this->data[length + i] = img_ptr[p + 1] * 1.f / 255;
        this->data[length_2 + i] = img_ptr[p + 2] * 1.f / 255;
    }
}

void Tensor3D::set_zero() {
    const int length = C * H * W;
    // for(int i = 0;i < length; ++i) data[i] = 0;
    std::memset(this->data, 0, sizeof(data_type) * length);
}

// �ҵ����һά���������ֵ
data_type Tensor3D::max() const {
    return this->data[argmax()];
}

// �ҵ����һά�������ֵ��λ��
int Tensor3D::argmax() const {
    const int length = C * H * W;
    if (data == nullptr) return 0;
    data_type max_value = this->data[0];
    int max_index = 0;
    for (int i = 1; i < length; ++i)
        if (this->data[i] > max_value) {
            max_value = this->data[i];
            max_index = i;
        }
    return max_index;
}

// �ҵ����һά���������ֵ
data_type Tensor3D::min() const {
    return this->data[argmin()];
}

// �ҵ����һά�������ֵ��λ��
int Tensor3D::argmin() const {
    const int length = C * H * W;
    if (data == nullptr) return 0;
    data_type min_value = this->data[0];
    int min_index = 0;
    for (int i = 1; i < length; ++i)
        if (this->data[i] < min_value) {
            min_value = this->data[i];
            min_index = i;
        }
    return min_index;
}


void Tensor3D::div(const data_type times) {
    const int length = C * H * W;
    for (int i = 0; i < length; ++i) data[i] /= times;
}

void Tensor3D::normalize(const std::vector<data_type> mean, const std::vector<data_type> std_div) {
    if (C != 3) return;
    const int ch_size = H * W;
    for (int ch = 0; ch < C; ++ch) {
        data_type* const ch_ptr = this->data + ch * ch_size;
        for (int i = 0; i < ch_size; ++i)
            ch_ptr[i] = (ch_ptr[i] - mean[ch]) / std_div[ch];
    }
}

cv::Mat Tensor3D::opecv_mat(const int CH) const {
    // ֻ���û�н��� normalize �� Tensor ����ȡ�����ݲ鿴, �Ӳ�����, ����
    cv::Mat origin;
    if (CH == 3) {
        origin = cv::Mat(H, W, CV_8UC3);
        const int length = H * W;
        for (int i = 0; i < length; ++i) {
            const int p = 3 * i;
            origin.data[p] = cv::saturate_cast<uchar>(255 * data[i]);
            origin.data[p + 1] = cv::saturate_cast<uchar>(255 * data[i + length]);
            origin.data[p + 2] = cv::saturate_cast<uchar>(255 * data[i + length + length]);
        }
    }
    else if (CH == 1) {
        origin = cv::Mat(H, W, CV_8UC1);
        const int length = H * W;
        for (int i = 0; i < length; ++i)
            origin.data[i] = cv::saturate_cast<uchar>(255 * data[i]);
    }
    return origin;
}

int Tensor3D::get_length() const { 
    return C * H * W;
}

//int Tensor3D::get_length_C_H() const {
//    return C * H;
//}

std::tuple<int, int, int> Tensor3D::get_shape() const {
    return std::make_tuple(C, H, W);
}

void Tensor3D::print_shape() const {
    std::cout << this->name << "  ==>  " << this->C << " x " << this->H << " x " << this->W << "\n";
}

void Tensor3D::print(const int _C) const {
    std::cout << this->name << "  ������ :\n";
    const int start = _C * H * W;
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j)
            std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(3) << this->data[start + i * W + j] << "   ";
        std::cout << "\n";
    }
}

std::shared_ptr<Tensor3D> Tensor3D::rot180() const {
    std::shared_ptr<Tensor3D> rot(new Tensor3D(C, H, W, this->name + "_rot180"));
    const int ch_size = H * W;
    for (int c = 0; c < C; ++c) {
        data_type* old_ptr = this->data + c * ch_size;
        data_type* ch_ptr = rot->data + c * ch_size;
        for (int i = 0; i < ch_size; ++i)
            ch_ptr[i] = old_ptr[ch_size - 1 - i];
    }
    return rot;
}

std::shared_ptr<Tensor3D> Tensor3D::pad(const int padding) const {
    std::shared_ptr<Tensor3D> padded(new Tensor3D(C, H + 2 * padding, W + 2 * padding, this->name + "_rot180"));
    const int new_W = (W + 2 * padding);
    const int ch_size = (H + 2 * padding) * new_W;
    // padded ��Χ�� 0
    std::memset(padded->data, 0, sizeof(data_type) * C * ch_size);
    for (int c = 0; c < C; ++c)
        for (int i = 0; i < H; ++i)
            std::memcpy(padded->data + c * ch_size + (padding + i) * new_W + padding,
                this->data + c * H * W + i * W, W * sizeof(data_type));
    return padded;
}

Tensor3D::~Tensor3D() noexcept {
    if (this->data != nullptr) {
        delete this->data;
        this->data = nullptr;
        // std::cout << this->name << " ����һ��\n";
    }
};








