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
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	///音声データの読み込み
	/// </summary>
	SoundData SoundLoadWave(const char* fileName);
	/// <summary>
	///	終了処理
	/// </summary>
	void SoundUnload(SoundData* soundData);

	/// <summary>
	/// サウンドの再生
	/// </summary>
	/// <param name="xAudio2"></param>
	/// <param name="soundData"></param>
	void SoundPlayWave(IXAudio2* xAudio2,const SoundData& soundData);
private:

	
	
	ComPtr<IXAudio2> xAudio2;
	
	IXAudio2MasteringVoice* masterVoice;
};
