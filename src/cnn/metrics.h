#ifndef CNN_METRICS_H
#define CNN_METRICS_H

// C++
#include <vector>


class ClassificationEvaluator {
private:
    int correct_num = 0;  // ��ǰ�ۼƵ��ж���ȷ��������Ŀ
    int sample_num = 0;   // ��ǰ�ۼƵ�������Ŀ
public:
    ClassificationEvaluator() = default;
    // ��һ�� batch �¶��˼���
    void compute(const std::vector<int>& predict, const std::vector<int>& labels);
    // �鿴�ۼƵ���ȷ��
    float get() const;
    // ���¿�ʼͳ��
    void clear();
};



#endif //CNN_METRICS_H
