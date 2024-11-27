#include "Audio.h"
#include <fstream>
#include<cassert>
void Audio::Initialize()
{
	//DirectXの初期化処理の末尾に追加
	//XAudio2の初期化
	HRESULT result = XAudio2Create(&xAudio2,0,XAUDIO2_DEFAULT_PROCESSOR);

	//マスターボイスの生成
	result = xAudio2->CreateMasteringVoice(&masterVoice);

}

SoundData Audio::SoundLoadWave(const char* fileName)
{
	//ファイル名がnullptrでないことを確認
	assert(fileName != nullptr);
	
	HRESULT result;

	/*-------------
	①ファイルを開く
	-------------*/

	//ファイル入力ストリームのインスタンス
	std::ifstream file;
	//.wavファイルをバイナリモードで開く
	file.open(fileName, std::ios_base::binary);
	//ファイルが開けなかった場合
	assert(file.is_open() && "ファイルが開けませんでした");

	/*-----------------
	②.wavデータの読み込み
	------------------*/

	//RIFFチャンクヘッダーの読み込み
	RiffChunkHeader riffChunkHeader = {};
	file.read((char*)&riffChunkHeader, sizeof(riffChunkHeader));
	//ファイルがRIFF形式でない場合
	if (strncmp(riffChunkHeader.chunk.id, "RIFF", 4) != 0) {
		assert(0 && "RIFF形式ではありません");
	}
	//ファイルがWAVE形式でない場合
	if (strncmp(riffChunkHeader.type, "WAVE", 4) != 0) {
		assert(0 && "WAVE形式ではありません");
	}
	//フォーマットチャンクの読み込み
	FormatChunk formatChunk = {};
	//チャンクヘッダーの確認
	file.read((char*)&formatChunk, sizeof(formatChunk));
	if (strncmp(formatChunk.chunk.id, "fmt ", 4) != 0) {
		assert(0 && "fmt形式ではありません");
	}

	//チャンク本体の読み込み
	assert(formatChunk.chunk.size == sizeof(formatChunk.format));
	file.read((char*)&formatChunk.format, formatChunk.chunk.size);


	//③ファイルクローズ


	//④読み込んだ音声データを返す
}
