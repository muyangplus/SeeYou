#ifndef CNN_FUNC_H
#define CNN_FUNC_H

#include "src/cnn/data_format.h"


// �� batch_size ������, ÿ������ softmax �ɶ����ĸ���
std::vector<tensor> softmax(const std::vector<tensor>& input);

// batch_size ������, ÿ������ 0, 1, 2 ����, ����  1 �͵õ� [0.0, 1.0, 0.0, 0.0]
std::vector<tensor> one_hot(const std::vector<int>& labels, const int num_classes);

// ��������� probs, �ͱ�ǩ label ���㽻������ʧ, ������ʧֵ�ͻش����ݶ�
std::pair<data_type, std::vector<tensor> > cross_entroy_backward(
    const std::vector<tensor>& probs, const std::vector<tensor>& labels);

// С����� string
std::string float_to_string(const float value, const int precision);

#endif //CNN_FUNC_H
