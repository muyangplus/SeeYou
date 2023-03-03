// C++
#include <iostream>
// Self
#include "src/cnn/architectures.h"


using namespace architectures;

Conv2D::Conv2D(std::string _name, const int _in_channels, const int _out_channels, const int _kernel_size, const int _stride)
    : Layer(_name), bias(_out_channels), in_channels(_in_channels), out_channels(_out_channels), kernel_size(_kernel_size), stride(_stride),
    params_for_one_kernel(_in_channels* _kernel_size* _kernel_size),
    offset(_kernel_size* _kernel_size) {
    // ��֤�����Ϸ���
    assert(_kernel_size & 1 && _kernel_size >= 3 && "����˵Ĵ�С������������ !");
    assert(_in_channels > 0 && _out_channels > 0 && _stride > 0);
    // ���ȸ�Ȩֵ���� weights ��ƫ�� b ����ռ�
    this->weights.reserve(out_channels);
    for (int o = 0; o < out_channels; ++o) {
        // һ���� out_channels �������, ÿ��������� in_channels X kernel_size X kernel_size ������
        weights.emplace_back(new Tensor3D(in_channels, kernel_size, kernel_size, this->name + "_" + std::to_string(o)));
    }
    // �����ʼ��, �����õ�����̬�ֲ���ʼ��
    this->seed.seed(212);
    std::normal_distribution<float> engine(0.0, 1.0);
    for (int o = 0; o < out_channels; ++o) bias[o] = engine(this->seed) / random_times;
    for (int o = 0; o < out_channels; ++o) {
        data_type* data_ptr = this->weights[o]->data;
        for (int i = 0; i < params_for_one_kernel; ++i)
            data_ptr[i] = engine(this->seed) / random_times;
    }
}

// ��������� forward ����, batch_num X in_channels X H X W
std::vector<tensor> Conv2D::forward(const std::vector<tensor>& input) {
    // ��ȡ��������ͼ����Ϣ
    const int batch_size = input.size();
    const int H = input[0]->H;
    const int W = input[0]->W;
    const int length = H * W; // һ����ά����ͼ�Ĵ�С, ������ƫ������
    // �������������ͼ��С
    const int out_H = std::floor((H - kernel_size - 2 * padding) / stride) + 1;
    const int out_W = std::floor((W - kernel_size - 2 * padding) / stride) + 1;
    const int out_length = out_H * out_W; // ���������ͼ, һ��ͨ��������ж��, 111 X 111, 7 X 7 ����
    // Ϊ�����׼��
    const int radius = int((kernel_size - 1) / 2);
    // ����ǵ�һ�ξ�����һ��, ����ռ�(��������Բ��, �����״����һ����, ���ܻ����, Ҫ���·���, ��ʱ������)
    if (this->output.empty()) {
        // std::cout << this->name << " ��һ�η������������\n";
        // �������������
        this->output.reserve(batch_size);
        for (int b = 0; b < batch_size; ++b)  // B X 16 X 111 X 111
            this->output.emplace_back(new Tensor3D(out_channels, out_H, out_W, this->name + "_output_" + std::to_string(b)));
        // �������� offset ֻ��һ��, ��Ȼ����, ��û�оֲ�������
        int pos = 0;
        for (int x = -radius; x <= radius; ++x)
            for (int y = -radius; y <= radius; ++y) {
                this->offset[pos] = x * W + y;
                ++pos;
            }
    }
    // ��¼����, ������� backward �Ļ�, ���� backward �� w ���ݶ�Ҫ��
    if (!no_grad) this->__input = input;
    // Ϊ�����׼��
    const int H_radius = H - radius; // ����ÿ��ѭ�����¼��� H - radius
    const int W_radius = W - radius;
    const int window_range = kernel_size * kernel_size; // �����һ����άƽ��Ĵ�С, ������ƫ�Ƶ�
    const int* const __offset = this->offset.data(); // ��ȡƫ����ָ��
    // ����ÿ��ͼ��ֿ����
    for (int b = 0; b < batch_size; ++b) {
        // ��ȡ�� b ��ͼ�����ʼ��ַ, in_channels X 224 X 224
        data_type* const cur_image_features = input[b]->data;
        for (int o = 0; o < out_channels; ++o) { // ÿ�������
            data_type* const out_ptr = this->output[b]->data + o * out_length;// �� o ������˻�õ�һ�� out_H X out_W ������ͼ
            data_type* const cur_w_ptr = this->weights[o]->data;  // in_channels x 3 x 3
            int cnt = 0; // ��¼ÿ�ξ�������ŵ�λ��
            for (int x = radius; x < H_radius; x += stride) {
                for (int y = radius; y < W_radius; y += stride) { // ����ͼ��ƽ��ÿһ����
                    data_type sum_value = 0.f;
                    const int coord = x * W + y; // ��ǰ��������ͨ��������ͼ��λ��
                    for (int i = 0; i < in_channels; ++i) { // ÿ�����ж��ͨ��
                        const int start = i * length + coord; // ����ĵ� i ������ͼ�� (x, y) ����λ��
                        const int start_w = i * window_range; // �� o ������˵ĵ� i ��ͨ��
                        for (int k = 0; k < window_range; ++k)// �����ֲ�����
                            sum_value += cur_image_features[start + __offset[k]] * cur_w_ptr[start_w + k];
                    }
                    sum_value += this->bias[o]; // �����Ǽ��� b
                    out_ptr[cnt] = sum_value;   // һ�����������͵Ľ��, �ŵ������ cnt λ����, �������ȴ洢
                    ++cnt;  // ��ŵ�λ�� + 1
                }
            } // std::cout << "cnt = " << std::sqrt(cnt) << std::endl;
        }
    }
    return this->output;  // ���ؾ�����, ������� out_ptr ������
}

// �Ż��Ļ�, ��һЩ���ϵ����ݷŵ�ջ��, �ֲ�������
std::vector<tensor> Conv2D::backward(std::vector<tensor>& delta) {
    // ��ȡ�ش����ݶ���Ϣ, ֮ǰ forward ����Ƕ��, delta ���Ƕ��(�������쳣����)
    const int batch_size = delta.size();
    const int out_H = delta[0]->H;
    const int out_W = delta[0]->W;
    const int out_length = out_H * out_W;
    // ��ȡ֮ǰ forward ����������ͼ��Ϣ
    const int H = this->__input[0]->H;
    const int W = this->__input[0]->W;
    const int length = H * W;
    // ��һ�ξ�������, �����������ݶȷ���ռ�
    if (this->weights_gradients.empty()) {
        // weights
        this->weights_gradients.reserve(out_channels);
        for (int o = 0; o < out_channels; ++o)
            this->weights_gradients.emplace_back(new Tensor3D(in_channels, kernel_size, kernel_size, this->name + "_weights_gradients_" + std::to_string(o)));
        // bias
        this->bias_gradients.assign(out_channels, 0);
    }
    // ����Ĭ�ϲ���¼�ݶȵ���ʷ��Ϣ, W, b ֮ǰ���ݶ�ȫ�����
    for (int o = 0; o < out_channels; ++o) this->weights_gradients[o]->set_zero();
    for (int o = 0; o < out_channels; ++o) this->bias_gradients[o] = 0;
    // ���� weights, bias ���ݶ�
    for (int b = 0; b < batch_size; ++b) { // ��� batch ÿ��ͼ���Ӧһ���ݶ�, ����ݶ�ȡƽ��
        // ����, ����ÿ�������
        for (int o = 0; o < out_channels; ++o) {
            // �� b ��ͼ����ݶ�, �ҵ��� o ��ͨ������ʼ��ַ
            data_type* o_delta = delta[b]->data + o * out_H * out_W;
            // ����˵�ÿ�� in ͨ��, �ֿ���
            for (int i = 0; i < in_channels; ++i) {
                // �� b �����룬�ҵ��� i ��ͨ������ʼ��ַ
                data_type* in_ptr = __input[b]->data + i * H * W;
                // �� o �������, �ҵ��� i ��ͨ������ʼ��ַ
                data_type* w_ptr = weights_gradients[o]->data + i * kernel_size * kernel_size;
                // // �������Ǿ���˵�һ��ͨ������ÿ���������ݶ�
                for (int k_x = 0; k_x < kernel_size; ++k_x) {
                    for (int k_y = 0; k_y < kernel_size; ++k_y) {
                        // ��¼һ��ͼ��� W �ݶ�
                        data_type sum_value = 0;
                        for (int x = 0; x < out_H; ++x) {
                            // delta �����ͨ���ĵ� x �У�ÿ�� out_W ����
                            data_type* delta_ptr = o_delta + x * out_W;
                            // ��Ӧ������ I �����ͨ���ĵ� (x * stride + k_x) �У� ÿ�� W ������
                            // ע�� * stride ÿ���� input ���������ҵ�, + k_x ������ֱ�����ϵ�ƫ����; ����� y * stride + k_y ͬ��
                            data_type* input_ptr = in_ptr + (x * stride + k_x) * W;
                            for (int y = 0; y < out_W; ++y) {
                                // ��ǰ w ���ݶ�, �ɲ�����������ͷ��ص��ݶ���ˣ��ۼ�
                                sum_value += delta_ptr[y] * input_ptr[y * stride + k_y];
                            }
                        }
                        // ���µ� weight_gradients, ע������� batch_size�������� +=�� ���� =, һ�� batch ���ݶ��ۼ�
                        w_ptr[k_x * kernel_size + k_y] += sum_value / batch_size;
                    }
                }
            }
            // ���� b ���ݶ�
            data_type sum_value = 0;
            // ��Ҫ������ͨ��������ݶ�
            for (int d = 0; d < out_length; ++d) sum_value += o_delta[d];
            // ���� batch_size
            bias_gradients[o] += sum_value / batch_size;
        }
    }
    // �������������������ݶ� delta_output
    // ��һ�ξ��� backward �� delta_output �����ڴ�
    if (this->delta_output.empty()) {
        this->delta_output.reserve(batch_size);
        for (int b = 0; b < batch_size; ++b)
            this->delta_output.emplace_back(new Tensor3D(in_channels, H, W, this->name + "_delta_" + std::to_string(b)));
    }
    // delta_output ��ʼ��Ϊ 0, ��һ�����Ƕ����
    for (int o = 0; o < batch_size; ++o) this->delta_output[o]->set_zero();
    // ��ת 180, padding �Ǹ�̫����, ����ֱ�Ӳ�����ķ���, �þ������λÿ�������Ӧ�Ĳ�������Ȩֵ w
    const int radius = (kernel_size - 1) / 2;
    const int H_radius = H - radius;
    const int W_radius = W - radius;
    const int window_range = kernel_size * kernel_size;
    // ��� batch ���ݶȷֿ���
    for (int b = 0; b < batch_size; ++b) {
        // in_channels X 224 X 224
        data_type* const cur_image_features = this->delta_output[b]->data;
        // 16 ������˵����, 16 x 111 x 111
        for (int o = 0; o < out_channels; ++o) { // ÿ�������
            data_type* const out_ptr = delta[b]->data + o * out_length;// �� o ������˻�õ�һ�� out_H X out_W ������ͼ
            data_type* const cur_w_ptr = this->weights[o]->data;  // in_channels x 3 x 3
            int cnt = 0; // ��¼ÿ�ξ�������ŵ�λ��
            for (int x = radius; x < H_radius; x += stride) {
                for (int y = radius; y < W_radius; y += stride) { // ����ͼ��ƽ��ÿһ����
                    // data_type sum_value = 0.f;
                    const int coord = x * W + y; // ��ǰ��������ͨ��������ͼ��λ��
                    for (int i = 0; i < in_channels; ++i) { // ÿ�����ж��ͨ��
                        const int start = i * length + coord; // ����ĵ� i ������ͼ�� (x, y) ����λ��
                        const int start_w = i * window_range; // �� o ������˵ĵ� i ��ͨ��
                        for (int k = 0; k < window_range; ++k) { // �����ֲ�����
                            // sum_value += cur_image_features[start + offset[k]] * cur_w_ptr[start_w + k];
                            cur_image_features[start + offset[k]] += cur_w_ptr[start_w + k] * out_ptr[cnt];
                        }
                    }
                    ++cnt; // ������λ��ǰ������ĵڼ�����
                }
            }
        }
    }
    // ����
    return this->delta_output;
}

// ���²���
void Conv2D::update_gradients(const data_type learning_rate) {
    assert(!this->weights_gradients.empty());
    // ���ݶȸ��µ� W �� b
    for (int o = 0; o < out_channels; ++o) {
        data_type* w_ptr = weights[o]->data;
        data_type* wg_ptr = weights_gradients[o]->data;
        // ����������Ȩֵ����
        for (int i = 0; i < params_for_one_kernel; ++i)
            w_ptr[i] -= learning_rate * wg_ptr[i];
        // ���� bias
        bias[o] -= learning_rate * bias_gradients[o];
    }
}

// ����Ȩֵ
void Conv2D::save_weights(std::ofstream& writer) const {
    // ��Ҫ������� weights, bias
    const int filter_size = sizeof(data_type) * params_for_one_kernel;
    for (int o = 0; o < out_channels; ++o)
        writer.write(reinterpret_cast<const char*>(&weights[o]->data[0]), static_cast<std::streamsize>(filter_size));
    writer.write(reinterpret_cast<const char*>(&bias[0]), static_cast<std::streamsize>(sizeof(data_type) * out_channels));
}

// ����Ȩֵ
void Conv2D::load_weights(std::ifstream& reader) {
    const int filter_size = sizeof(data_type) * params_for_one_kernel;
    for (int o = 0; o < out_channels; ++o)
        reader.read((char*)(&weights[o]->data[0]), static_cast<std::streamsize>(filter_size));
    reader.read((char*)(&bias[0]), static_cast<std::streamsize>(sizeof(data_type) * out_channels));
}


// ��ȡ��һ������Ĳ���ֵ
int Conv2D::get_params_num() const {
    return (this->params_for_one_kernel + 1) * this->out_channels;
}


