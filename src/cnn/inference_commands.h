#ifndef CNN_COOMMANDS_H
#define CNN_COOMMANDS_H

// C++
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;



string programAuthor = "徐照杰";
string programName = "CNN inference program";
string programVersion = "Bete 1.1";
string programWebsite = "https://muyangplus.js.cool";

string filePath = "./checkpoints/AlexNet_person_aug_1e-3/iter_200_train_1.000_valid_1.000.model";
std::vector<std::string> categories({ "person","bird" });
std::vector<std::string> images_list({ "./datasets/images/test_1.jpg","./datasets/images/test_2.jpg" });



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
    cout << "    -c [ --categories ]           Set categories Categories must be in the file directory" << endl;
    cout << "           str                         the list of categories." << endl;
    cout << "    -t [ --images ]               Set test images." << endl;
    cout << "           str                         the list of test images." << endl;
    cout << "    -p [ --filepath ]             Set model load path." << endl;
    cout << "           str                         the path of models." << endl;
    cout << endl;
    cout << "Examples :" << endl;
    cout << "   .\\cnn_inference.exe -c \"person\" \"bird\" -t \"./datasets/images/test_1.jpg\" \"./datasets/images/test_2.jpg\" -p \"./checkpoints/AlexNet_person_aug_1e-3/iter_200_train_1.000_valid_1.000.model\" " << endl;
    cout << "   .\\cnn_inference.exe -c \"person\" \"bird\" -t \"./datasets/images/test_5.jpg\" -p \"./checkpoints/AlexNet_person_aug_1e-3/iter_200_train_1.000_valid_1.000.model\" "<< endl;
    cout << "   .\\cnn_inference.exe --categories \"bird\" \"cat\" \"dog\" --testimage \"./datasets/images/test_3.jpg\" \"./datasets/images/test_4.jpg\" --filepath \"./checkpoints/AlexNet_person_aug_1e-3/iter_200_train_1.000_valid_1.000.model\" " << endl;
    cout << "   .\\cnn_inference.exe -o" << endl;
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
    cout << "Parameter configurations:" << endl;
    cout << "filePath = " << filePath << endl;
    cout << "categories = {";
    for (int i = 0; i < categories.size() - 1; i++) {
        cout << categories[i] << ",";
    }
    cout << categories[categories.size() - 1] << "}" << endl;
    cout << "images_list = {";
    for (int i = 0; i < images_list.size() - 1; i++) {
        cout << images_list[i] << ",";
    }
    cout << images_list[images_list.size() - 1] << "}" << endl;
    cout << endl;
}
//主命令函数
int int_commands(int argc, char* argv[]) {
    cout << "argv : " << argc << endl;
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
        }
        else if (str == "-v" || str == "--version") {
            command_version();
            return 0;
        }
        else if (str == "-o" || str == "--original") {
            command_original();
            return 0;
        }
        else if (i == argc) {
            //无额外参数命令判断完毕，如果这是最后一项，那命令行参数一定错误!
            return 2;
        }
        //有一个附加参数命令
        if (str == "-p" || str == "--filepath") {
            i++;
            filePath = argv[i];
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
        } else if (str == "-t" || str == "--images") {
            string this_str;
            images_list.clear();
            while (this_str[0] != '-') {
                i++;
                if (i == argc) {
                    break;
                }
                this_str = argv[i];
                if (this_str[0] != '-') {
                    images_list.emplace_back(this_str);
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



#endif //CNN_COOMMANDS_H
