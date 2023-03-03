// C++
#include <iostream>
#include <time.h>
#include <queue>
// Self
#include "src/core/result_back.h"
#include "src/core/chcp.h"

bool ResultBack::outCheckByTime(int waittingTime, int pos) {
	int now = time(0);
	if (now - waittingTime >= lastRunningTime[pos]) {
		lastRunningTime[pos] = now;
		return true;
	}
	else {
		return false;
	}
}

bool ResultBack::outCheckByResult(std::string message, int showTime) {
	int now = time(0);
	while (!resultList.empty()) {
		int messageTime = resultList.front().time;
		if (now - waittingTime >= messageTime) {
			resultList.pop();
		}
		else break;
	}
	
	bool flag = true;
	int loop = resultList.size();
	while(loop--) {
		if (resultList.front().message == message) flag = false;
		resultList.push({ resultList.front().message, resultList.front().time });
		resultList.pop();
	}

	if (flag) {
		resultList.push({ message,showTime });
		return true;
	}
	else return false;
}

void ResultBack::matchedFace(std::string name) {
	std::string print = "ƥ�䵽�沿���� : " + name;
	if(GBK) print = Chcp::GbkToUtf8(print.c_str());
	if (ResultBack::outCheckByResult(print, time(0))) std::cout << print << std::endl;
	return;
}

void ResultBack::too_far() {
	std::string print = "��������ͷ̫Զ�ˣ��뿿������ͷ";
	if (GBK) print = Chcp::GbkToUtf8(print.c_str());
	if (ResultBack::outCheckByResult(print, time(0))) std::cout << print << std::endl;
	return;
}

void ResultBack::no_mask() {
	std::string print = "δ�������";
	if (GBK) print = Chcp::GbkToUtf8(print.c_str());
	if (ResultBack::outCheckByResult(print, time(0))) std::cout << print << std::endl;
	return;
}

/*
void ResultBack::show_your_QRcode() {
	std::string print = "������ʾ���Ľ�����";
	if (GBK) print = Chcp::GbkToUtf8(print.c_str());
	if (ResultBack::outCheckByResult(print, time(0))) std::cout << print << std::endl;
	return;
}

void ResultBack::green_QRcode() {
	std::string print = "������Ϊ��ɫ";
	if (GBK) print = Chcp::GbkToUtf8(print.c_str());
	if (ResultBack::outCheckByResult(print, time(0))) std::cout << print << std::endl;
	return;
}

void ResultBack::yellow_QRcode() {
	std::string print = "������Ϊ��ɫ";
	if (GBK) print = Chcp::GbkToUtf8(print.c_str());
	if (ResultBack::outCheckByResult(print, time(0))) std::cout << print << std::endl;
	return;
}

void ResultBack::red_QRcode() {
	std::string print = "������Ϊ��ɫ";
	if (GBK) print = Chcp::GbkToUtf8(print.c_str());
	if (ResultBack::outCheckByResult(print, time(0))) std::cout << print << std::endl;
	return;
}

void ResultBack::error_QRcode() {
	std::string print = "����Ľ�������ɫ";
	if (GBK) print = Chcp::GbkToUtf8(print.c_str());
	if (ResultBack::outCheckByResult(print, time(0))) std::cout << print << std::endl;
	return;
}
*/
