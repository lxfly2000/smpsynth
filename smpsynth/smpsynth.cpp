#include "smpsynth.h"
#include <vector>

#define PI 3.141592653545

struct SmpSynthChannel
{
	std::vector<SMPSYNTH_SOUND_SAMPLE_DATATYPE>sample;//单声道-32768-32767
	std::vector<SMPSYNTH_SOUND_SAMPLE_DATATYPE>buffer;//双声道-32768-32767
	std::vector<unsigned char>volumeEnvelope;//0-255
	std::vector<unsigned char>panEnvelope;//[0,128,255]
	int sampleLoopStart = 0;//采样循环开始点采样位置
	int sampleLoopEnd = 0;//为0表示没有循环
	int volumeEnvelopeLoopStart = 0, volumeEnvelopeLoopEnd = 0;//音量线循环起止位置
	int panEnvelopeLoopStart = 0, panEnvelopeLoopEnd = 0;//平衡线循环起止位置
	int currentSynthSampleCount = 0;//已经合成的采样数
	int currentSamplePos = 0;//当前采样源的位置（变化速度受音阶影响）
	int currentVolumeEnvelopePos = 0;//当前音量线的位置
	int currentPanEnvelopePos = 0;//当前平衡线的位置
	int keyOffSamplePos = 0;//音阶置0前采样源的位置（变化速度受音阶影响）
	int keyOffVolumeEnvelopePos = 0;//音阶置0前音量线的位置
	int keyOffPanEnvelopePos = 0;//音阶置0前平衡线的位置
	unsigned char currentChannelScale = 0;//当前用户指定的音阶
	unsigned char currentChannelSynthScale = 0;//当前发声的音阶
	unsigned char currentChannelVolume = SMPSYNTH_VOLUME_DEFAULT;//当前音量
	unsigned char currentChannelPan = SMPSYNTH_PAN_CENTER;//当前平衡
};

struct SmpSynthData
{
	int sampleRate = 44100;
	int bpm = 120;
	int ticksPerBeat = 24;
	std::vector<SmpSynthChannel>channels;
	std::vector<SMPSYNTH_SOUND_SAMPLE_DATATYPE>mixBuffer;
};

static SmpSynthData synthData;

int smpsynth_init(int sampleRate, int channels, int initial_bpm, int ticksPerBeat)
{
	synthData.sampleRate = sampleRate;
	synthData.channels.resize(channels);
	synthData.ticksPerBeat = ticksPerBeat;
	smpsynth_setBpm(initial_bpm);
	return 0;
}

void smpsynth_release()
{
	synthData.channels.clear();
}

void smpsynth_setSampleData(int channel, SMPSYNTH_SOUND_SAMPLE_DATATYPE*data, int sampleCount, int loopStart, int loopEnd)
{
	auto &ch = synthData.channels[channel];
	ch.sample.assign(data, data + sampleCount);
	ch.sampleLoopStart = loopStart;
	ch.sampleLoopEnd = loopEnd;
}

void smpsynth_setSampleVolumeEnvelope(int channel, unsigned char * data, int bytes, int loopStart, int loopEnd)
{
	auto &ch = synthData.channels[channel];
	ch.volumeEnvelope.assign(data, data + bytes);
	ch.volumeEnvelopeLoopStart = loopStart;
	ch.volumeEnvelopeLoopEnd = loopEnd;
}

void smpsynth_setSamplePanEnvelope(int channel, unsigned char * data, int bytes, int loopStart, int loopEnd)
{
	auto &ch = synthData.channels[channel];
	ch.panEnvelope.assign(data, data + bytes);
	ch.panEnvelopeLoopStart = loopStart;
	ch.panEnvelopeLoopEnd = loopEnd;
}

void smpsynth_setNoteScale(int channel, unsigned char data)
{
	auto &ch = synthData.channels[channel];
	if (data >= 128)
	{
		ch.currentChannelScale = 0;
		ch.currentChannelSynthScale = 0;
		ch.currentSynthSampleCount = 0;
		ch.currentSamplePos = 0;
		ch.currentPanEnvelopePos = 0;
		ch.currentVolumeEnvelopePos = 0;
		ch.keyOffPanEnvelopePos = 0;
		ch.keyOffSamplePos = 0;
		ch.keyOffVolumeEnvelopePos = 0;
	}
	else
	{
		ch.currentChannelScale = data;
		if (data > 0)
		{
			ch.currentChannelSynthScale = data;
			ch.currentSynthSampleCount = 0;
			ch.currentSamplePos = 0;
			ch.currentPanEnvelopePos = 0;
			ch.currentVolumeEnvelopePos = 0;
			ch.keyOffPanEnvelopePos = 0;
			ch.keyOffSamplePos = 0;
			ch.keyOffVolumeEnvelopePos = 0;
		}
		else
		{
			ch.keyOffPanEnvelopePos = ch.currentPanEnvelopePos;
			ch.keyOffSamplePos = ch.currentSamplePos;
			ch.keyOffVolumeEnvelopePos = ch.currentVolumeEnvelopePos;
		}
	}
}

void smpsynth_setNoteVolume(int channel, unsigned char data)
{
	synthData.channels[channel].currentChannelVolume = data;
}

void smpsynth_setNotePan(int channel, char data)
{
	synthData.channels[channel].currentChannelPan = data;
}

void smpsynth_setBpm(int bpm)
{
	synthData.bpm = bpm;
	//同时会影响TickBuffer长度
	size_t bufferLength = _samples_per_tick()*SMPSYNTH_SOUND_CHANNELS;
	for (size_t i = 0; i < synthData.channels.size(); i++)
	{
		if (synthData.channels[i].buffer.size() != bufferLength)
			synthData.channels[i].buffer.resize(bufferLength);
	}
	if (synthData.mixBuffer.size() != bufferLength)
		synthData.mixBuffer.resize(bufferLength);
}

int _samples_per_tick()
{
	return synthData.sampleRate * 60 / synthData.bpm / synthData.ticksPerBeat;
}

void pan_sample(unsigned char pan, short*left, short*right)
{
	//https://dsp.stackexchange.com/questions/21691/algorithm-to-pan-audio
	float angle = (float)(pan - SMPSYNTH_PAN_CENTER) / (SMPSYNTH_PAN_RIGHT - SMPSYNTH_PAN_CENTER) * PI / 4;
	float ampA = sqrtf(2.0f) / 2 * (cosf(angle) - sinf(angle));//left
	float ampB = sqrtf(2.0f) / 2 * (cosf(angle) + sinf(angle));//right
	*right = (short)(*right * ampB);
	*left = (short)(*left * ampA);
}

#define SCALE_SAMPLE_COUNT(scale,sample) ((int)((sample)*powf(2.0f, ((scale) - SMPSYNTH_STANDARD_SCALE) / 12.0f)))

void _synthSample(int channel,int sampleCount)
{
	auto& ch = synthData.channels[channel];
	//i是buffer位置，samplePos是sample位置
	for (int i = 0; i < sampleCount; i++)
	{
		//复制采样
		ch.currentSamplePos = SCALE_SAMPLE_COUNT(ch.currentChannelSynthScale, ch.currentSynthSampleCount + i);
		int samplePos = ch.currentSamplePos;
		if (ch.sampleLoopEnd)//有循环
		{
			if (ch.currentChannelScale == ch.currentChannelSynthScale)//音阶持续
			{
				if (samplePos >= ch.sampleLoopEnd)
					samplePos = (samplePos - ch.sampleLoopStart) % (ch.sampleLoopEnd - ch.sampleLoopStart) + ch.sampleLoopStart;
			}
			else//音阶结束
			{
				if (samplePos >= ch.sampleLoopEnd)
					samplePos = samplePos - ch.keyOffSamplePos + (ch.keyOffSamplePos - ch.sampleLoopStart) % (ch.sampleLoopEnd - ch.sampleLoopStart) + ch.sampleLoopStart;
			}
		}
		if (samplePos >= ch.sample.size())
		{
			ch.buffer[i * 2] = 0;
			ch.buffer[i * 2 + 1] = 0;
			sampleCount = i + 1;
			ch.currentChannelSynthScale = 0;
			break;
		}
		else
		{
			ch.buffer[i * 2] = ch.sample[samplePos];
			ch.buffer[i * 2 + 1] = ch.sample[samplePos];
		}
		//应用音量线
		ch.currentVolumeEnvelopePos = ch.currentSynthSampleCount + i;
		int volumeEnvelopePos = ch.currentVolumeEnvelopePos;
		if (ch.volumeEnvelopeLoopEnd)
		{
			if (ch.currentChannelScale == ch.currentChannelSynthScale)
			{
				if (volumeEnvelopePos >= ch.volumeEnvelopeLoopEnd)
					volumeEnvelopePos = (volumeEnvelopePos - ch.volumeEnvelopeLoopStart) % (ch.volumeEnvelopeLoopEnd - ch.volumeEnvelopeLoopStart) + ch.volumeEnvelopeLoopStart;
			}
			else
			{
				if (volumeEnvelopePos >= ch.volumeEnvelopeLoopEnd)
					volumeEnvelopePos = volumeEnvelopePos - ch.keyOffVolumeEnvelopePos + (ch.keyOffVolumeEnvelopePos - ch.volumeEnvelopeLoopStart) % (ch.volumeEnvelopeLoopEnd - ch.volumeEnvelopeLoopStart) + ch.volumeEnvelopeLoopStart;
			}
		}
		if (volumeEnvelopePos >= ch.volumeEnvelope.size())
			volumeEnvelopePos = ch.volumeEnvelope.size() - 1;
		ch.buffer[i * 2] = ch.buffer[i * 2] * ch.volumeEnvelope[volumeEnvelopePos] / SMPSYNTH_VOLUME_DEFAULT;
		ch.buffer[i * 2 + 1] = ch.buffer[i * 2 + 1] * ch.volumeEnvelope[volumeEnvelopePos] / SMPSYNTH_VOLUME_DEFAULT;
		//应用平衡线
		ch.currentPanEnvelopePos = ch.currentSynthSampleCount + i;
		int panEnvelopePos = ch.currentPanEnvelopePos;
		if (ch.panEnvelopeLoopEnd)
		{
			if (ch.currentChannelScale == ch.currentChannelSynthScale)
			{
				if (panEnvelopePos >= ch.panEnvelopeLoopEnd)
					panEnvelopePos = (panEnvelopePos - ch.panEnvelopeLoopStart) % (ch.panEnvelopeLoopEnd - ch.panEnvelopeLoopStart) + ch.panEnvelopeLoopStart;
			}
			else
			{
				if (panEnvelopePos >= ch.panEnvelopeLoopEnd)
					panEnvelopePos = panEnvelopePos - ch.keyOffPanEnvelopePos + (ch.keyOffPanEnvelopePos - ch.panEnvelopeLoopStart) % (ch.panEnvelopeLoopEnd - ch.panEnvelopeLoopStart) + ch.panEnvelopeLoopStart;
			}
		}
		if (panEnvelopePos >= ch.panEnvelope.size())
			panEnvelopePos = ch.panEnvelope.size() - 1;
		pan_sample(ch.panEnvelope[panEnvelopePos], &ch.buffer[i * 2], &ch.buffer[i * 2 + 1]);
	}
	ch.currentSynthSampleCount += sampleCount;
	for (size_t i = sampleCount * SMPSYNTH_SOUND_CHANNELS; i < ch.buffer.size(); i++)
		ch.buffer[i] = 0;
}

int smpsynth_getTickStream(SMPSYNTH_SOUND_SAMPLE_DATATYPE*streamData)
{
	memset(synthData.mixBuffer.data(), 0, synthData.mixBuffer.size() * sizeof(synthData.mixBuffer[0]));
	for (int i = 0; i < synthData.channels.size(); i++)
	{
		auto &ch = synthData.channels[i];
		//先合成Sample
		if (ch.currentChannelSynthScale)
		{
			int synthSampleCount = _samples_per_tick();
			_synthSample(i, synthSampleCount);
			//再合成轨道参数
			for (size_t j = 0; j < ch.buffer.size(); j++)
				ch.buffer[j] = ch.buffer[j] * ch.currentChannelVolume / SMPSYNTH_VOLUME_MAX;
			for (size_t j = 0; j < ch.buffer.size() / 2; j++)
				pan_sample(ch.currentChannelPan, &ch.buffer[j * 2], &ch.buffer[j * 2 + 1]);

			//混音
			for (size_t j = 0; j < synthData.mixBuffer.size(); j++)
			{
				SMPSYNTH_SOUND_SAMPLE_DATATYPE data1 = synthData.mixBuffer[j];
				SMPSYNTH_SOUND_SAMPLE_DATATYPE data2 = ch.buffer[j];
				if (data1 < 0 && data2 < 0)
					synthData.mixBuffer[j] = data1 + data2 - (data1 * data2 / -(pow(2, 16 - 1) - 1));
				else
					synthData.mixBuffer[j] = data1 + data2 - (data1 * data2 / (pow(2, 16 - 1) - 1));
			}
		}
	}
	if (streamData)
		memcpy(streamData, synthData.mixBuffer.data(), synthData.mixBuffer.size() * sizeof(synthData.mixBuffer[0]));
	return (int)synthData.mixBuffer.size();
}
