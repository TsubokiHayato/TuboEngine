#pragma once
#include<xaudio2.h>
#include <wrl/client.h>
#include <cstdint>
#pragma comment(lib,"xaudio2.lib")
using Microsoft::WRL::ComPtr;

//チャンクヘッダー
struct ChunkHeader
{
	char id[4];//チャンクID
	int32_t size;//チャンクサイズ
};

//RIFFチャンクヘッダー
struct RiffChunkHeader
{
	ChunkHeader chunk;//チャンクヘッダー
	char type[4];//\WAVE
};

//FMTチャンクヘッダー
struct FormatChunk
{

	ChunkHeader chunk;//"fmt "チャンクヘッダー
	WAVEFORMATEX format;//波形フォーマット

};

//音声データ
struct SoundData {
	//波形フォーマット
	WAVEFORMATEX wfex;//WaveFormatex
	//バッファのい先頭アドレス
	BYTE* pBuffer;
	//バッファのサイズ
	unsigned int bufferSize;
};

class Audio
{

public:

	void Initialize();
	SoundData SoundLoadWave(const char* fileName);

private:

	
	
	ComPtr<IXAudio2> xAudio2;
	
	IXAudio2MasteringVoice* masterVoice;
};
