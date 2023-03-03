#ifndef CNN_TRAIN_COOMMANDS_H
#define CNN_TRAIN_COOMMANDS_H

// C++
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;



string programAuthor = "徐照杰";
string programName = "CNN training program";
string programVersion = "Beta 1.1";
string programWebsite = "https://muyangplus.js.cool";

int train_batch_size = 4;
int valid_batch_size = 1;
int test_batch_size = 1;
string filePath = "./datasets";
std::vector<std::string> categories({ "person","bird" });
string savePath = "./checkpoints/AlexNet_aug_1e-3";

int start_iters = 1;        // 从第几个 iter 开始
int total_iters = 400000;   // 训练 batch 的总数
float learning_rate = 1e-3; // 学习率
int valid_inters = 1000;    // 验证一次的间隔
int save_iters = 5000;      // 保存模型的间隔

/*
 * 版本信息
 * -v [ --verson ]
 */
void command_version() {
    cout << programName << " " << programVersion << endl;
    cout << "opencv : " << CV_VERSION << endl;
    cout << "by " << programAuthor << " (" << programWebsite << ")" << endl;
    cout << endl;
}
/*
 * 帮助信息
 * -h [ --help ]
 */
void command_help() {
    command_version();
    cout << "Allowed options:" << endl;
    cout << "    -h [ --help ]                 Show help." << endl;
    cout << "    -v [ --version ]              Show version information." << endl;
    cout << "    -o [ --original ]             Use default configuration." << endl;
    cout << "    -t [ --train_batch_size ]     Set train batch size." << endl;
    cout << "           num                         the number of train batch size." << endl;
    cout << "    -p [ --filepath ]             Set file path." << endl;
    cout << "           str                         the path of files." << endl;
    cout << "    -c [ --categories ]           Set categories. Categories must be in the file directory" << endl;
    cout << "           str                         the list of categories." << endl;
    cout << "    -s [ --savepath ]             Set model save path." << endl;
    cout << "           str                         the path of models." << endl;
    cout << "    -i [ --start_iters ]          Set start iters." << endl;
    cout << "           num                         the number of start iters." << endl;
    cout << "    -e [ --total_iters ]          Set total iters." << endl;
    cout << "           num                         the number of total iters." << endl;
    cout << "    -l [ --learning_rate ]        Set learning rate." << endl;
    cout << "           num                         the number of learning rate." << endl;
    cout << "    -y [ --valid_inters ]         Set verification interval." << endl;
    cout << "           num                         the time of verification interval." << endl;
    cout << "    -m [ --save_iters ]           Set save model interval." << endl;
    cout << "           num                         the number of save model interval." << endl;
    cout << endl;
    cout << "Examples :" << endl;
    cout << "   .\\cnn_train.exe -t 4 -p \"./datasets\" -c \"person\" \"bird\" - s \"./checkpoints/AlexNet_aug_1e-3\" -i 1 -e 400000 -l 1e-3 -y 1000 -m 5000" << endl;
    cout << "   .\\cnn_train.exe -t 4 -p \"./datasets\" -c \"person\" \"bird\" -s \"./checkpoints/AlexNet_person_aug_1e-3\" -i 1 -e 200 -l 1e-3 -y 10 -m 50" << endl;
    cout << "   .\\cnn_train.exe -t 4 --filepath \"./datasets\" --categories \"bird\" \"cat\" \"dog\" \"panda\" --savepath \"./checkpoints/AlexNet_aug_1e-3\" -e 400000 -l 1e-3 -y 1000 -m 5000" << endl;
    cout << "   .\\cnn_train.exe -o" << endl;
    cout << endl;
}
/*
 * 使用默认的配置执行
 * -o [ --original ]
 */
void command_original() {
    cout << "Using the default configuration..." << endl << endl;
}
/*
 * 未知的命令
 */
void command_unkown() {
    cout << "Unkown command!" << endl << endl;
    command_help();
}
/*
 * 当前变量信息
 */
void command_show_info() {
    command_help();
    cout << "Parameter configurations:" <<  endl;
    cout << "train_batch_size = " << train_batch_size << endl;
    cout << "valid_batch_size = " << valid_batch_size << endl;
    cout << "test_batch_size = " << test_batch_size << endl;
    cout << "filePath = " << filePath << endl;
    cout << "categories = {";
    for (int i = 0; i < categories.size() - 1; i++) {
        cout << categories[i] << ",";
    }
    cout << categories[categories.size() - 1] << "}" << endl;
    cout << "savePath = " << savePath << endl;
    cout << "start_iters = " << start_iters << endl;
    cout << "total_iters = " << total_iters << endl;
    cout << "learning_rate = " << learning_rate << endl;
    cout << "valid_inters = " << valid_inters << endl;
    cout << "save_iters = " << save_iters << endl;
    cout << endl;
}
//主命令函数
int int_commands(int argc, char* argv[]) {
    //cout << "argv : " << argc << endl;
    for (int i = 0; i < argc; i++) {
        cout << "argv[" << i << "] : " << argv[i] << endl;
    }
    cout << endl;
    if (argc == 1) {
        //如果没有命令行参数
        command_help();
        return 1;
    }
    for (int i = 1; i < argc; i++) {
        cout << argv[i] << endl;
        string str = argv[i];
        //无附加参数命令
        if (str == "-h" || str == "--help") {
            command_help();
            return 0;
        } else if (str == "-v" || str == "--version") {
            command_version();
            return 0;
        } else if (str == "-o" || str == "--original") {
            command_original();
            return 0;
        } else if (i == argc) {
            //无额外参数命令判断完毕，如果这是最后一项，那命令行参数一定错误!
            return 2;
        }
        //有一个附加参数命令
        if (str == "-t" || str == "--train_batch_size") {
            i++;
            train_batch_size = atoi(argv[i]);
            continue;
        } else if (str == "-p" || str == "--filepath") {
            i++;
            filePath = argv [i];
            continue;
        } else if (str == "-s" || str == "--savepath") {
            i++;
            savePath = argv[i];
            continue;
        } else if (str == "-i" || str == "--start_iters") {
            i++;
            start_iters = atoi(argv[i]);
            continue;
        } else if (str == "-e" || str == "--total_iters") {
            i++;
            total_iters = atoi(argv[i]);
            continue;
        } else if (str == "-l" || str == "--learning_rate") {
            i++;
            learning_rate = atof(argv[i]);
            continue;
        } else if (str == "-y" || str == "--valid_inters") {
            i++;
            valid_inters = atoi(argv[i]);
            continue;
        } else if (str == "-m" || str == "--save_iters") {
            i++;
            save_iters = atoi(argv[i]);
            continue;
        }
        //有多个附加参数命令
        if (str == "-c" || str == "--categories") {
            string this_str;
            categories.clear();
            while (this_str[0] != '-') {
                i++;
                if (i == argc) {
                    break;
                }
                this_str = argv[i];
                if (this_str[0] != '-') {
                    categories.emplace_back(this_str);
                }
            } 
            i--;
            continue;
        }
        //未知命令
        return 2;
    }
    return 0;
}



#endif //CNN_TRAIN_COOMMANDS_H
