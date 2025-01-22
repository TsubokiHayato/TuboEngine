#include"Logger.h"
#include<Windows.h>



namespace Logger {

    // log作成
    void Log(const std::string& message) {
		// デバッグ出力
        OutputDebugStringA(message.c_str());
    }

}