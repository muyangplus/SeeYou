﻿// C++
#include <iostream>
// OpenCV
#include <opencv.hpp>
// Self
#include "src/core/capture.h"

int main(int argc, char* argv[]) {
	bool runOnWindows = false;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	std::cout << "Running on Windows..." << std::endl;
	runOnWindows = true;
	//system("chcp 65001");
#else
	std::cout << "Running on Linux..." << std::endl;
	runOnWindows = false;
#endif

	FaceAlgo::initFaceModels("./models/yunet.onnx", "./models/face_recognizer_fast.onnx", "./data/registFace");
	int id = 0;
	while (true) {
		std::cout << "Please input your capture id:";
		std::cin >> id;
		if (Capture::initCapture(id, 1)) {
			break;
		}
		else {
			std::cout << "Cann't find this capture." << std::endl;
		}
	}

	Capture::recognition(id, runOnWindows, 0);

	return 0;
}


