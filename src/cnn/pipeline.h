#ifndef CNN_PIPELINE_H
#define CNN_PIPELINE_H

// C++
#include <map>
#include <random>
#include <filesystem>
// self
#include "src/cnn/data_format.h"


namespace pipeline {

    using list_type = std::vector<std::pair<std::string, int> >;
    // ���ļ��� dataset_path, ���� categories �õ���ͬ����ͼ���б�
    std::map<std::string, list_type> get_images_for_classification(
        const std::filesystem::path dataset_path,
        const std::vector<std::string> categories = {},
        const std::pair<float, float> ratios = { 0.8, 0.1 });

    // �ر���!!! �����������ǿ�Ļ�, �ٶȽ���Ϊԭ���� 1/4
    class ImageAugmentor {
    private:
        std::default_random_engine e, l, c, r; // e ������ò����ĸ���; l �������Ҳ����б�; c �����õ��ü���Ҫ�ĸ���; r �����õ���ת�ĸ���
        std::uniform_real_distribution<float> engine;
        std::uniform_real_distribution<float> crop_engine;
        std::uniform_real_distribution<float> rotate_engine;
        std::uniform_int_distribution<int> minus_engine;
        std::vector<std::pair<std::string, float> > ops;
    public:
        ImageAugmentor(const std::vector<std::pair<std::string, float> >& _ops = { {"hflip", 0.5}, {"vflip", 0.2}, {"crop", 0.7}, {"rotate", 0.5} })
            : e(212), l(826), c(320), r(520),
            engine(0.0, 1.0), crop_engine(0.0, 0.25), rotate_engine(15, 75), minus_engine(1, 10),
            ops(std::move(_ops)) {}
        void make_augment(cv::Mat& origin, const bool show = false);
    };

    class DataLoader {
        using batch_type = std::pair< std::vector<tensor>, std::vector<int> >; // batch ��һ�� pair
    private:
        list_type images_list; // ���ݼ��б�, image <==> label
        int images_num;        // ��������ݼ�һ���ж�����ͼ��Ͷ�Ӧ�ı�ǩ
        const int batch_size;  // ÿ�δ������ͼ��
        const bool augment;    // �Ƿ�Ҫ��ͼ����ǿ
        const bool shuffle;    // �Ƿ�Ҫ�����б�
        const int seed;        // ÿ����������б������
        int iter = -1;         // ��ǰ�ɵ��˵� iter ��ͼ��
        std::vector<tensor> buffer; // batch ������, ������ͼ������ tensor ��, ����ÿ��Ҫ��ȡͼ��ʱ���������
        const int H, W, C;     // �����ͼ��ߴ�
    public:
        explicit DataLoader(const list_type& _images_list, const int _bs = 1, const bool _aug = false, const bool _shuffle = true, const std::tuple<int, int, int> image_size = { 224, 224, 3 }, const int _seed = 212);
        int length() const;
        batch_type generate_batch();
    private:
        // ��ȡ�� batch_index ��ͼ����Ϣ, ���� tensor
        std::pair<tensor, int> add_to_buffer(const int batch_index);
        // ͼ����ǿ
        ImageAugmentor augmentor;
    };
}


#endif //CNN_PIPELINE_H
