// C++
#include <string>
#include <iostream>
// self
#include "src/cnn/architectures.h"

using namespace architectures;


namespace {
    inline data_type square(const data_type x) {
        return x * x;
    }
}


BatchNorm2D::BatchNorm2D(std::string _name, const int _out_channels, const data_type _eps, const data_type _momentum)
    : Layer(_name), out_channels(_out_channels), eps(_eps), momentum(_momentum),
    gamma(_out_channels, 1.0), beta(_out_channels, 0),
    moving_mean(_out_channels, 0), moving_var(_out_channels, 0),
    buffer_mean(_out_channels, 0), buffer_var(_out_channels, 0) {}


std::vector<tensor> BatchNorm2D::forward(const std::vector<tensor>& input) {
    // ��ȡ������Ϣ
    const int batch_size = input.size();
    const int H = input[0]->H;
    const int W = input[0]->W;
    // ��һ�ξ��� forward, ����ռ�
    if (this->output.empty()) {
        this->output.reserve(batch_size);
        for (int b = 0; b < batch_size; ++b) this->output.emplace_back(new Tensor3D(out_channels, H, W, this->name + "_output_" + std::to_string(b)));
        this->normed_input.reserve(batch_size);
        for (int b = 0; b < batch_size; ++b) this->normed_input.emplace_back(new Tensor3D(out_channels, H, W, this->name + "_normed_" + std::to_string(b)));
    }
    // ��¼����
    if (!no_grad) this->__input = input;
    // ����ע���Ƿ�Ҫ��Ϊ 0
    // ��ʼ��һ��
    const int feature_map_length = H * W;  // һ�Ŷ�άƽ������ͼ�Ĵ�С
    const int output_length = batch_size * feature_map_length;  // һ�����������������
    for (int o = 0; o < out_channels; ++o) {
        // �����ѵ��
        if (!no_grad) {
            // �����ֵ u
            data_type u = 0;
            for (int b = 0; b < batch_size; ++b) {
                data_type* const src_ptr = input[b]->data + o * feature_map_length; // �����ҵ��˵� o ������ĵ� b ��ͼ������ָ��
                for (int i = 0; i < feature_map_length; ++i)
                    u += src_ptr[i];
            }
            // ����� backward, ��ס��ֵ
            u = u / output_length;
            // �󷽲� sigma
            data_type var = 0;
            for (int b = 0; b < batch_size; ++b) {
                data_type* const src_ptr = input[b]->data + o * feature_map_length;
                for (int i = 0; i < feature_map_length; ++i)
                    var += square(src_ptr[i] - u);
            }
            var = var / output_length;
            if (!no_grad) {
                buffer_mean[o] = u;
                buffer_var[o] = var;
            }
            // �Ե� o ���������һ��
            const data_type var_inv = 1. / std::sqrt(var + eps);
            for (int b = 0; b < batch_size; ++b) {
                data_type* const src_ptr = input[b]->data + o * feature_map_length;
                data_type* const norm_ptr = normed_input[b]->data + o * feature_map_length;
                data_type* const des_ptr = output[b]->data + o * feature_map_length;
                for (int i = 0; i < feature_map_length; ++i) {
                    norm_ptr[i] = (src_ptr[i] - u) * var_inv;
                    des_ptr[i] = gamma[o] * norm_ptr[i] + beta[o];
                }
            }
            // ������ʷ�ľ�ֵ�ͷ���(����Ҫ����ѵ���ͷ�ѵ���ڼ�, Ҳ���� train �� eval ������!!!!!!)
            moving_mean[o] = (1 - momentum) * moving_mean[o] + momentum * u;
            moving_var[o] = (1 - momentum) * moving_var[o] + momentum * var;
        }
        else {
            // ֱ�ӹ�һ��
            const data_type u = moving_mean[o];
            const data_type var_inv = 1. / std::sqrt(moving_var[o] + eps);
            for (int b = 0; b < batch_size; ++b) {
                data_type* const src_ptr = input[b]->data + o * feature_map_length;
                data_type* const norm_ptr = normed_input[b]->data + o * feature_map_length;
                data_type* const des_ptr = output[b]->data + o * feature_map_length;
                for (int i = 0; i < feature_map_length; ++i) {
                    norm_ptr[i] = (src_ptr[i] - u) * var_inv;
                    des_ptr[i] = gamma[o] * norm_ptr[i] + beta[o];
                }
            }
        }
    }
    return this->output;
}

// batch norm �� delta Ҳ���Ծ͵��޸�
std::vector<tensor> BatchNorm2D::backward(std::vector<tensor>& delta) {
    // ��ȡ��Ϣ
    const int batch_size = delta.size();
    const int feature_map_length = delta[0]->H * delta[0]->W;
    const int output_length = batch_size * feature_map_length;
    // ��Ϊ�ǵ�һ�η���, ���Ը� gradients ����ռ�
    if (gamma_gradients.empty()) {
        gamma_gradients.assign(out_channels, 0);
        beta_gradients.assign(out_channels, 0);
        norm_gradients = std::shared_ptr<Tensor3D>(new Tensor3D(batch_size, delta[0]->H, delta[0]->W));
    }
    // ÿ�ζ������, ��������ʷ�ݶ���Ϣ
    for (int o = 0; o < out_channels; ++o) gamma_gradients[o] = beta_gradients[o] = 0;
    // �Ӻ���ǰ��
    for (int o = 0; o < out_channels; ++o) {
        // ��� u, var, norm ���ݶ�
        norm_gradients->set_zero(); // B X H X W
        // ��һ���� beta �� gamma �Ƚϼ�, ���� norm ���ݶ�
        for (int b = 0; b < batch_size; ++b) {
            data_type* const delta_ptr = delta[b]->data + o * feature_map_length;
            data_type* const norm_ptr = normed_input[b]->data + o * feature_map_length;
            data_type* const norm_g_ptr = norm_gradients->data + b * feature_map_length;
            for (int i = 0; i < feature_map_length; ++i) {
                gamma_gradients[o] += delta_ptr[i] * norm_ptr[i];
                beta_gradients[o] += delta_ptr[i];
                norm_g_ptr[i] += delta_ptr[i] * gamma[o];
            }
        }
        // ������, �ǶԷ��� var ���ݶ�, mean ������ var, ����Ҫ���� var ���ݶ�
        data_type var_gradient = 0;
        const data_type u = buffer_mean[o];
        const data_type var_inv = 1. / std::sqrt(buffer_var[o] + eps);
        const data_type var_inv_3 = var_inv * var_inv * var_inv;
        for (int b = 0; b < batch_size; ++b) {
            data_type* const src_ptr = __input[b]->data + o * feature_map_length;
            data_type* const norm_g_ptr = norm_gradients->data + b * feature_map_length;
            for (int i = 0; i < feature_map_length; ++i)
                var_gradient += norm_g_ptr[i] * (src_ptr[i] - u) * (-0.5) * var_inv_3;
        }
        // ��������Ծ�ֵ u �ľ�ֵ
        data_type u_gradient = 0;
        const data_type inv = var_gradient / output_length;
        for (int b = 0; b < batch_size; ++b) {
            data_type* const src_ptr = __input[b]->data + o * feature_map_length;
            data_type* const norm_g_ptr = norm_gradients->data + b * feature_map_length;
            for (int i = 0; i < feature_map_length; ++i)
                u_gradient += norm_g_ptr[i] * (-var_inv) + inv * (-2) * (src_ptr[i] - u);
        }
        // ������󷵻ظ��������ݶ�
        for (int b = 0; b < batch_size; ++b) {
            data_type* const src_ptr = __input[b]->data + o * feature_map_length;
            data_type* const norm_g_ptr = norm_gradients->data + b * feature_map_length;
            data_type* const back_ptr = delta[b]->data + o * feature_map_length;
            for (int i = 0; i < feature_map_length; ++i)
                back_ptr[i] = norm_g_ptr[i] * var_inv + inv * 2 * (src_ptr[i] - u) + u_gradient / output_length;
        }
    }
    return delta;
}


void BatchNorm2D::update_gradients(const data_type learning_rate) {
    for (int o = 0; o < out_channels; ++o) {
        gamma[o] -= learning_rate * gamma_gradients[o];
        beta[o] -= learning_rate * beta_gradients[o];
    }
}

void BatchNorm2D::save_weights(std::ofstream& writer) const {
    const int stream_size = sizeof(data_type) * out_channels;
    writer.write(reinterpret_cast<const char*>(&gamma[0]), static_cast<std::streamsize>(stream_size));
    writer.write(reinterpret_cast<const char*>(&beta[0]), static_cast<std::streamsize>(stream_size));
    writer.write(reinterpret_cast<const char*>(&moving_mean[0]), static_cast<std::streamsize>(stream_size));
    writer.write(reinterpret_cast<const char*>(&moving_var[0]), static_cast<std::streamsize>(stream_size));
}

void BatchNorm2D::load_weights(std::ifstream& reader) {
    const int stream_size = sizeof(data_type) * out_channels;
    reader.read((char*)(&gamma[0]), static_cast<std::streamsize>(stream_size));
    reader.read((char*)(&beta[0]), static_cast<std::streamsize>(stream_size));
    reader.read((char*)(&moving_mean[0]), static_cast<std::streamsize>(stream_size));
    reader.read((char*)(&moving_var[0]), static_cast<std::streamsize>(stream_size));
}


