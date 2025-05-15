#pragma once

#define SMPSYNTH_PAN_LEFT 0
#define SMPSYNTH_PAN_CENTER 128
#define SMPSYNTH_PAN_RIGHT 255
#define SMPSYNTH_VOLUME_MAX 255
#define SMPSYNTH_VOLUME_DEFAULT SMPSYNTH_VOLUME_MAX
#define SMPSYNTH_SOUND_CHANNELS 2//固定使用双声道
#define SMPSYNTH_SOUND_SAMPLE_DATATYPE short//固定使用16bit
#define SMPSYNTH_STANDARD_SCALE_NUMBER 57
#define SMPSYNTH_STANDARD_SCALE_FREQUENCY 440

//指的是合成轨道数，而非输出声道数（左右声道）
int smpsynth_init(int sampleRate = 44100, int channels = 8, int initial_bpm = 120, int ticksPerBeat = 24);

void smpsynth_release();

//默认静音，loopEnd为0表示没有循环部分，单声道，为-1表示结尾，单位为采样数位置
void smpsynth_setSampleData(int channel, SMPSYNTH_SOUND_SAMPLE_DATATYPE*data, int sampleCount, int loopStart = 0, int loopEnd = -1);
void smpsynth_setStandardScaleSampleData(int channel, SMPSYNTH_SOUND_SAMPLE_DATATYPE*data);
int smpsynth_getStandardScaleSampleCount();

//默认满音量
void smpsynth_setSampleVolumeEnvelope(int channel, unsigned char*data, int bytes, int loopStart = 0, int loopEnd = -1);

//默认居中
void smpsynth_setSamplePanEnvelope(int channel, unsigned char*data, int bytes, int loopStart = 0, int loopEnd = -1);

//0为静音，57为标准音阶A，音符变0会使前一个音符进入播放完毕状态，然后停止，变非0会立即改变音阶,>=128会立即停止
void smpsynth_setNoteScale(int channel, unsigned char data);

//设定通道实时音量
void smpsynth_setNoteVolume(int channel, unsigned char data);

//设定通道实时平衡
void smpsynth_setNotePan(int channel, char data);

//设定速度
void smpsynth_setBpm(int bpm);

//获取生成帧的采样数量
int smpsynth_samplesPerTick();

//生成所有通道混合的音频，同时使时间计时器推进smpsynth_samplesPerTick()返回的采样数，返回值为生成的采样数
int smpsynth_getTickStream(SMPSYNTH_SOUND_SAMPLE_DATATYPE*streamData);

//TODO:增加PITCH功能
