#ifndef CNN_ARCHITECTURES_H
#define CNN_ARCHITECTURES_H


// C++
#include <list>
#include <fstream>
// self
#include "src/cnn/pipeline.h"


namespace architectures {
    using namespace pipeline;

    // �����ʼ���õ�, C++ ������ɵ����ֹ���, softmax ֮ǰ�Ķ��ü���, ֱ�ӱ���, �ӵ�
    extern data_type random_times;

    // ȫ�ֱ���, �Ƿ�Ҫ backward, �����ٶ���Ҫ��һЩ
    extern bool no_grad;

    // ���������ڹر��ݶ���ؼ���
    class WithoutGrad final {
    public:
        explicit WithoutGrad() {
            architectures::no_grad = true;
        }
        ~WithoutGrad() noexcept {
            architectures::no_grad = false;
        }
    };

    // ����ͳһ�����������͵�, ����������, �������麯����̬, Ч�ʶ��� forward backward, save_weights ��ЩӰ����ʵ���Ǻܴ�
    // backward û�� const, ��Ϊ relu �Ǿ͵ز�����, �G
    class Layer {
    public:
        const std::string name;  // ��һ�������
        std::vector<tensor> output;  // ���������
    public:
        Layer(std::string& _name) : name(std::move(_name)) {}
        virtual std::vector<tensor> forward(const std::vector<tensor>& input) = 0;
        virtual std::vector<tensor> backward(std::vector<tensor>& delta) = 0;
        virtual void update_gradients(const data_type learning_rate = 1e-4) {}
        virtual void save_weights(std::ofstream& writer) const {}
        virtual void load_weights(std::ifstream& reader) {}
        virtual std::vector<tensor> get_output() const { return this->output; }
    };


    class Conv2D : public Layer {
    private:
        // �����Ĺ�����Ϣ
        std::vector<tensor> weights; // ����˵�Ȩֵ����, out_channels X in_channels X kernel_size X kernel_size
        std::vector<data_type> bias; // ƫ��(����д�� tensor1D)
        const int in_channels;  // Ҫ�˲�������ͼ�м���ͨ��
        const int out_channels; // ��һ�����м��������
        const int kernel_size;  // ����˵ı߳�
        const int stride;       // ����Ĳ���
        const int params_for_one_kernel;   // һ������˵Ĳ�������
        const int padding = 0;  // padding �����, ������ƻ���������ĳ���, ������������, �Ժ���ʱ����˵��
        std::default_random_engine seed;   // ��ʼ��������
        std::vector<int> offset; // �����ƫ����, �����õ�
       // ��ʷ��Ϣ
        std::vector<tensor> __input; // ���ݶ���Ҫ, ��ʵ�洢����ָ��
        // ������, ����ÿ�����·���
        std::vector<tensor> delta_output; // �洢�ش�����һ����ݶ�
        std::vector<tensor> weights_gradients; // Ȩֵ���ݶ�
        std::vector<data_type> bias_gradients; // bias ���ݶ�
    public:
        Conv2D(std::string _name, const int _in_channels = 3, const int _out_channels = 16, const int _kernel_size = 3, const int _stride = 2);
        // ��������� forward ����, batch_num X in_channels X H X W
        std::vector<tensor> forward(const std::vector<tensor>& input);
        // �Ż��Ļ�, ��һЩ���ϵ����ݷŵ�ջ��, �ֲ�������
        std::vector<tensor> backward(std::vector<tensor>& delta);
        // �����ݶ�
        void update_gradients(const data_type learning_rate = 1e-4);
        // ����Ȩֵ
        void save_weights(std::ofstream& writer) const;
        // ����Ȩֵ
        void load_weights(std::ifstream& reader);
        // ��ȡ��һ������Ĳ���ֵ
        int get_params_num() const;
    };


    class MaxPool2D : public Layer {
    private:
        // ��һ��Ĺ�������
        const int kernel_size;
        const int step;
        const int padding; // ��ʱ��֧��
        // ������, ����ÿ�����·����
        std::vector< std::vector<int> > mask; // ��¼��Щλ�������ݶȻش���, �� b ��ͼ, ÿ��ͼһ�� std::vector<int>
        std::vector<tensor> delta_output; // ���ص� delta
        std::vector<int> offset;  // ƫ����ָ��, ��֮ǰ Conv2D ��һ��
    public:
        MaxPool2D(std::string _name, const int _kernel_size = 2, const int _step = 2)
            : Layer(_name), kernel_size(_kernel_size), step(_step), padding(0),
            offset(_kernel_size* _kernel_size, 0) {}
        // ǰ��
        std::vector<tensor> forward(const std::vector<tensor>& input);
        // ���򴫲�
        std::vector<tensor> backward(std::vector<tensor>& delta);

    };


    class ReLU : public Layer {
    public:
        ReLU(std::string _name) : Layer(_name) {}
        std::vector<tensor> forward(const std::vector<tensor>& input);
        std::vector<tensor> backward(std::vector<tensor>& delta);
    };


    // ���Ա任��
    class LinearLayer : public Layer {
    private:
        // ���Բ�Ĺ�����Ϣ
        const int in_channels;                // �������Ԫ����
        const int out_channels;               // �������Ԫ����
        std::vector<data_type> weights;       // Ȩֵ����(������ʵ���Ըĳ� Tensor1D, �������Ϳ���ͳһ, ������� weights_gradients ���ø�)
        std::vector<data_type> bias;          // ƫ��
        // ��ʷ��Ϣ
        std::tuple<int, int, int> delta_shape;// ������, delta ����״, �� 1 X 4096 �� 128 * 4 * 4 ����
        std::vector<tensor> __input;          // �ݶȻش���ʱ����Ҫ���� Wx + b, ��Ҫ���� x
        // �����ǻ�����
        std::vector<tensor> delta_output;     // delta �ش���������ݶ�
        std::vector<data_type> weights_gradients; // ������, Ȩֵ������ݶ�
        std::vector<data_type> bias_gradients;    // bias ���ݶ�
    public:
        LinearLayer(std::string _name, const int _in_channels, const int _out_channels);
        // �� Wx + b ��������
        std::vector<tensor> forward(const std::vector<tensor>& input);
        std::vector<tensor> backward(std::vector<tensor>& delta);
        void update_gradients(const data_type learning_rate = 1e-4);
        void save_weights(std::ofstream& writer) const;
        void load_weights(std::ifstream& reader);
    };


    // ��� BatchNorm �ǲ�ͬͨ����, ������ʵ�ֻ��治֪��, ������ʱ�����
    // Ŀǰֻ���� Conv ��� BN
    class BatchNorm2D : public Layer {
    private:
        // ������Ϣ
        const int out_channels;
        const data_type eps;
        const data_type momentum;
        // Ҫѧϰ�Ĳ���(����ֱ���� vector ����, ��ͳһҲ���ⲻ��)
        std::vector<data_type> gamma;
        std::vector<data_type> beta;
        // Ҫ��������ʷ��Ϣ
        std::vector<data_type> moving_mean;
        std::vector<data_type> moving_var;
        // ������, ����ÿ�����·���
        std::vector<tensor> normed_input;
        std::vector<data_type> buffer_mean;
        std::vector<data_type> buffer_var;
        // �������ݶ���Ϣ
        std::vector<data_type> gamma_gradients;
        std::vector<data_type> beta_gradients;
        // ��ʱ���ݶ���Ϣ, ��ʵҲ�ǻ�����
        tensor norm_gradients;
        // ���ݶ���Ҫ�õ�
        std::vector<tensor> __input;
    public:
        BatchNorm2D(std::string _name, const int _out_channels, const data_type _eps = 1e-5, const data_type _momentum = 0.1);
        std::vector<tensor> forward(const std::vector<tensor>& input);
        std::vector<tensor> backward(std::vector<tensor>& delta);
        void update_gradients(const data_type learning_rate = 1e-4);
        void save_weights(std::ofstream& writer) const;
        void load_weights(std::ifstream& reader);
    };


    // ��ʱֻ֧�� Conv ��, �� dropout һ����� linear �������Ӳ�
    // ��Ȼѵ����������ѵ��, ���ǲ����е�����
    class Dropout : public Layer {
    private:
        // ��������
        data_type p;
        int selected_num;
        std::vector<int> sequence;
        std::default_random_engine drop;
        // backward ��Ҫ��
        std::vector<int> mask;
    public:
        Dropout(std::string _name, const data_type _p = 0.5) : Layer(_name), p(_p), drop(1314) {}
        std::vector<tensor> forward(const std::vector<tensor>& input);
        std::vector<tensor> backward(std::vector<tensor>& delta);
    };



    // ����д��һ�����ܵ� CNN ����ṹ, ������� AlexNet
    class AlexNet {
    public:
        bool print_info = false;
    private:
        std::list< std::shared_ptr<Layer> > layers_sequence;
    public:
        AlexNet(const int num_classes = 3, const bool batch_norm = false);
        // ǰ��
        std::vector<tensor> forward(const std::vector<tensor>& input);
        // �ݶȷ���
        void backward(std::vector<tensor>& delta_start);
        // �ݶȸ��µ�Ȩֵ
        void update_gradients(const data_type learning_rate = 1e-4);
        // ����ģ��
        void save_weights(const std::filesystem::path& save_path) const;
        // ����ģ��
        void load_weights(const std::filesystem::path& checkpoint_path);
        // GradCam ���ӻ�
        cv::Mat grad_cam(const std::string& layer_name) const;
    };
}



#endif //CNN_ARCHITECTURES_H
