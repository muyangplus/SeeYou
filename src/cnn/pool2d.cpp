// self
#include "src/cnn/architectures.h"


using namespace architectures;

std::vector<tensor> MaxPool2D::forward(const std::vector<tensor>& input) {
    // ��ȡ������Ϣ
    const int batch_size = input.size();
    const int H = input[0]->H;
    const int W = input[0]->W;
    const int C = input[0]->C;
    // ��������Ĵ�С
    const int out_H = std::floor(((H - kernel_size + 2 * padding) / step)) + 1;
    const int out_W = std::floor(((W - kernel_size + 2 * padding) / step)) + 1;
    // ��һ�ξ����óػ���(ͬ�� batch_size �����ø���, ����������, Ҫ��������)
    if (this->output.empty()) {
        // ���������ռ�
        this->output.reserve(batch_size);
        for (int b = 0; b < batch_size; ++b)
            this->output.emplace_back(new Tensor3D(C, out_H, out_W, this->name + "_output_" + std::to_string(b)));
        // �����򴫲��� delta ����ռ�
        if (!no_grad) {
            this->delta_output.reserve(batch_size);
            for (int b = 0; b < batch_size; ++b)
                this->delta_output.emplace_back(new Tensor3D(C, H, W, this->name + "_delta_" + std::to_string(b)));
            // mask �� batch ��ÿһ��ͼ������ռ�
            this->mask.reserve(batch_size);
            for (int b = 0; b < batch_size; ++b)
                this->mask.emplace_back(std::vector<int>(C * out_H * out_W, 0));
        }
        // ��һ�ξ�����һ��, ���� kernel_size ���� offset
        int pos = 0;
        for (int i = 0; i < kernel_size; ++i)
            for (int j = 0; j < kernel_size; ++j)
                this->offset[pos++] = i * W + j;
    }
    // ������� backward, ÿ�� forward Ҫ�ǵð� mask ȫ�����Ϊ 0
    const int out_length = out_H * out_W;
    int* mask_ptr = nullptr;
    if (!no_grad) {
        const int mask_size = C * out_length;
        for (int b = 0; b < batch_size; ++b) {
            int* const mask_ptr = this->mask[b].data();
            for (int i = 0; i < mask_size; ++i) mask_ptr[i] = 0;
        }
    }
    // ��ʼ�ػ�
    const int length = H * W;
    const int H_kernel = H - kernel_size;
    const int W_kernel = W - kernel_size;
    const int window_range = kernel_size * kernel_size;
    for (int b = 0; b < batch_size; ++b) { // batch ��ÿһ��ͼ���Ӧ������ͼ�ֿ��ػ�
        // 16 X 111 X 111 �� 16 X 55 X 55
        for (int i = 0; i < C; ++i) {  // ÿ��ͨ��
            // �������õ��˵� b ��ͼ�ĵ� i ��ͨ��, һ��ָ�����ݴ�С 55 X 55 ��ָ��
            data_type* const cur_image_features = input[b]->data + i * length;
            // �� b ������ĵ� i ��ͨ����, ͬ����ָ�����ݴ�С 55 X 55 ��ָ��
            data_type* const output_ptr = this->output[b]->data + i * out_length;
            // ��¼�� b �����, ��¼��Ч���� 111 X 111 ���ͼ�ϵ�λ��, һ���� 55 X 55 ��ֵ
            if (!no_grad) mask_ptr = this->mask[b].data() + i * out_length;
            int cnt = 0;  // ��ǰ�ػ������λ��
            for (int x = 0; x <= H_kernel; x += step) {
                data_type* const row_ptr = cur_image_features + x * W; // ��ȡ���ͨ��ͼ��ĵ� x ��ָ��
                for (int y = 0; y <= W_kernel; y += step) {
                    // �ҵ��ֲ��� kernel_size X kernel_size ������, �����ֵ
                    data_type max_value = row_ptr[y];
                    int max_index = 0; // ��¼���ֵ��λ��
                    for (int k = 1; k < window_range; ++k) { // �� 1 ��ʼ��Ϊ 0 �Ѿ��ȹ���, max_value = row_ptr[y]
                        data_type comp = row_ptr[y + offset[k]];
                        if (max_value < comp) {
                            max_value = comp;
                            max_index = offset[k];
                        }
                    }
                    // �ֲ����ֵ�����Ķ�Ӧλ����
                    output_ptr[cnt] = max_value;
                    // �������Ҫ backward, ��¼ mask
                    if (!no_grad) {
                        max_index += x * W + y; // �� i ��ͨ��, i * out_H * out_W Ϊ���Ķ�άƽ��, ƫ���� max_index
                        mask_ptr[cnt] = i * length + max_index;
                    }
                    ++cnt;
                }
            } // if(this->name == "max_pool_2" and b == 0 and i == 0)
        }
    }
    return this->output;
}

// ���򴫲�
std::vector<tensor> MaxPool2D::backward(std::vector<tensor>& delta) {
    // ��ȡ������ݶ���Ϣ
    const int batch_size = delta.size();
    // B X 128 X 6 X 6, ���� 0
    for (int b = 0; b < batch_size; ++b) this->delta_output[b]->set_zero();
    // batch ÿ��ͼ��, ���� mask ��ǵ�λ��, �� delta �е�ֵ� delta_output ��ȥ
    const int total_length = delta[0]->get_length();
    for (int b = 0; b < batch_size; ++b) {
        int* mask_ptr = this->mask[b].data();
        // ��ȡ delta �� b ��������������ݶ���ʼ��ַ
        data_type* const src_ptr = delta[b]->data;
        // ��ȡ���ص�������ݶ�, �� b ���ݶȵ���ʼ��ַ
        data_type* const res_ptr = this->delta_output[b]->data;
        for (int i = 0; i < total_length; ++i)
            res_ptr[mask_ptr[i]] = src_ptr[i]; // res_ptr ����Чλ�� mask_ptr[i] ���������������ݶȡ� src_ptr[i]
    }
    return this->delta_output;
}


