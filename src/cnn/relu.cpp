// C++
// self
#include "src/cnn/architectures.h"


using namespace architectures;


std::vector<tensor>  ReLU::forward(const std::vector<tensor>& input) {
    // ��ȡͼ����Ϣ
    const int batch_size = input.size();
    // ����ǵ�һ�ξ�����һ��
    if (output.empty()) {
        // ���������ռ�
        this->output.reserve(batch_size);
        for (int b = 0; b < batch_size; ++b)
            this->output.emplace_back(new Tensor3D(input[0]->C, input[0]->H, input[0]->W, this->name + "_output_" + std::to_string(b)));
    }
    // ֻ���� > 0 �Ĳ���
    const int total_length = input[0]->get_length();
    for (int b = 0; b < batch_size; ++b) {
        data_type* const src_ptr = input[b]->data;
        data_type* const out_ptr = this->output[b]->data;
        for (int i = 0; i < total_length; ++i)
            out_ptr[i] = src_ptr[i] >= 0 ? src_ptr[i] : 0;
    }
    return this->output;
}

std::vector<tensor> ReLU::backward(std::vector<tensor>& delta) { // ���û�� delta_output, ��Ϊ��״һģһ��, ���Լ���һЩ�ռ�ʹ��, ��Ϊ�˶�̬Ҫͳһ
    // ��ȡ��Ϣ
    const int batch_size = delta.size();
    // ����һ��������,  < 0 �Ĳ��ֹ��˵�
    const int total_length = delta[0]->get_length();
    for (int b = 0; b < batch_size; ++b) {
        data_type* src_ptr = delta[b]->data;
        data_type* out_ptr = this->output[b]->data;
        for (int i = 0; i < total_length; ++i)
            src_ptr[i] = out_ptr[i] <= 0 ? 0 : src_ptr[i]; // ��� > 0 �Ĳ����ݶȴ���� src_ptr ��������
    }
    // ��������, ����۲�
    for (int b = 0; b < batch_size; ++b) delta[b]->name = this->name + "_delta_" + std::to_string(b);
    return delta;
}


