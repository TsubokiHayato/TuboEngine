#include"Logger.h"
#include<Windows.h>



namespace Logger {

    // log作成
    void Log(const std::string& message) {
        OutputDebugStringA(message.c_str());
    }

}