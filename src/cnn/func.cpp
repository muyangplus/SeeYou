// Self
#include "src/cnn/func.h"



namespace {
    inline data_type __exp(const data_type x) {
        if (x >= 88) return FLT_MAX; // ֱ�ӷ��� float �����ֵ, ��� data_type ���� double ������ø�
        else if (x <= -50) return 0.f;
        return std::exp(x);
    }
}


// �� batch_size ������, ÿ������ softmax �ɶ����ĸ���
std::vector<tensor> softmax(const std::vector<tensor>& input) {
    const int batch_size = input.size();
    const int num_classes = input[0]->get_length();
    std::vector<tensor> output;
    output.reserve(batch_size);
    for (int b = 0; b < batch_size; ++b) {
        tensor probs(new Tensor3D(num_classes)); // ������� 1000 �����־����˷�
        // ���������������ֵ, ��ֹ���, ���Ǹı䲻��ʲô, ���� -37 ֱ�ӵ��� 1, ���������ܽ������, �G
        const data_type max_value = input[b]->max();
        data_type sum_value = 0;
        for (int i = 0; i < num_classes; ++i) {
            probs->data[i] = __exp(input[b]->data[i] - max_value);
            sum_value += probs->data[i];
        }
        // ����֮�� = 1
        for (int i = 0; i < num_classes; ++i) probs->data[i] /= sum_value;
        // ȥ��һЩ nan
        for (int i = 0; i < num_classes; ++i) if (std::isnan(probs->data[i])) probs->data[i] = 0.f;
        output.emplace_back(std::move(probs));
    }
    return output;
}

// batch_size ������, ÿ������ 0, 1, 2 ����, ����  1 �͵õ� [0.0, 1.0, 0.0, 0.0]
std::vector<tensor> one_hot(const std::vector<int>& labels, const int num_classes) {
    const int batch_size = labels.size();
    std::vector<tensor> one_hot_code;
    one_hot_code.reserve(batch_size);
    for (int b = 0; b < batch_size; ++b) {
        tensor sample(new Tensor3D(num_classes));
        for (int i = 0; i < num_classes; ++i)
            sample->data[i] = 0;
        assert(labels[b] >= 0 && labels[b] < num_classes);
        sample->data[labels[b]] = 1.0;
        one_hot_code.emplace_back(sample);
    }
    return one_hot_code;
}

// ��������� probs, �ͱ�ǩ label ���㽻������ʧ, ������ʧֵ�ͻش����ݶ�
std::pair<data_type, std::vector<tensor> > cross_entroy_backward(
    const std::vector<tensor>& probs, const std::vector<tensor>& labels) {
    const int batch_size = labels.size();
    const int num_classes = probs[0]->get_length();
    std::vector<tensor> delta;
    delta.reserve(batch_size);
    data_type loss_value = 0;
    for (int b = 0; b < batch_size; ++b) {
        tensor piece(new Tensor3D(num_classes));
        for (int i = 0; i < num_classes; ++i) {
            piece->data[i] = probs[b]->data[i] - labels[b]->data[i];
            loss_value += std::log(probs[b]->data[i]) * labels[b]->data[i];
        }
        delta.emplace_back(piece);
    }
    loss_value = loss_value * (-1.0) / batch_size;
    return std::make_pair(loss_value, delta);
}

// С����� string
std::string float_to_string(const float value, const int precision) {
    std::stringstream buffer;
    buffer.precision(precision);
    buffer.setf(std::ios::fixed);
    buffer << value;
    return buffer.str();
}


