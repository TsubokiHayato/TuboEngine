#include "Audio.h"
#include <fstream>
#include<cassert>
void Audio::Initialize()
{
	//DirectXの初期化処理の末尾に追加
	//XAudio2の初期化
	HRESULT result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

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

	//データチャンクの読み込み
	ChunkHeader dataChunkHeader = {};
	file.read((char*)&dataChunkHeader, sizeof(dataChunkHeader));
	if (strncmp(dataChunkHeader.id, "data", 4) != 0) {
		assert(0 && "data形式ではありません");
	}
	if (strncmp(dataChunkHeader.id, "data", 4) == 0) {
		//読み取り位置をJunkチャンクの終わりまで進める
		file.seekg(dataChunkHeader.size, std::ios_base::cur);
		//再読み込み
		file.read((char*)&dataChunkHeader, sizeof(dataChunkHeader));
	}


	if (strncmp(dataChunkHeader.id, "data", 4) != 0) {
		assert(0 && "data形式ではありません");
	}

	//Dataチャンクのデータ(波形データ)を読み込む
	char* pBuffer = new char[dataChunkHeader.size];
	file.read(pBuffer, dataChunkHeader.size);

	//③ファイルクローズ
	file.close();

	//④読み込んだ音声データを返す

	//returnする為の音声データ
	SoundData soundData = {};

	//WAVEFORMATEXの設定
	soundData.wfex = formatChunk.format;
	//波形データの設定
	soundData.pBuffer = (BYTE*)pBuffer;
	//波形データのサイズの設定
	soundData.bufferSize = dataChunkHeader.size;

	return soundData;
}

void Audio::SoundUnload(SoundData* soundData)
{
	//バッファメモリを開放
	delete[] soundData->pBuffer;

	soundData->pBuffer = nullptr;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

void Audio::SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData)
{

	HRESULT result;

	//波形フォーマットを先にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice=nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	//再生する波形データの設定
	XAUDIO2_BUFFER buffer = {};
	buffer.pAudioData = soundData.pBuffer;
	buffer.AudioBytes = soundData.bufferSize;
	buffer.Flags = XAUDIO2_END_OF_STREAM;

	//再生
	result = pSourceVoice->SubmitSourceBuffer(&buffer);
	result = pSourceVoice->Start(0);
}
