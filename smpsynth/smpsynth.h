#pragma once

#define SMPSYNTH_PAN_LEFT 0
#define SMPSYNTH_PAN_CENTER 128
#define SMPSYNTH_PAN_RIGHT 255
#define SMPSYNTH_VOLUME_MAX 255
#define SMPSYNTH_VOLUME_DEFAULT SMPSYNTH_VOLUME_MAX
#define SMPSYNTH_SOUND_CHANNELS 2//固定使用双声道
#define SMPSYNTH_SOUND_SAMPLE_DATATYPE short//固定使用16bit
#define SMPSYNTH_STANDARD_SCALE 57

//指的是合成轨道数，而非输出声道数（左右声道）
int smpsynth_init(int sampleRate = 44100, int channels = 8, int initial_bpm = 120, int ticksPerBeat = 24);

void smpsynth_release();

//默认静音，loopEnd为0表示没有循环部分，单声道
void smpsynth_setSampleData(int channel, SMPSYNTH_SOUND_SAMPLE_DATATYPE*data, int sampleCount, int loopStart, int loopEnd);

//默认满音量
void smpsynth_setSampleVolumeEnvelope(int channel, unsigned char*data, int bytes, int loopStart, int loopEnd);

//默认居中
void smpsynth_setSamplePanEnvelope(int channel, unsigned char*data, int bytes, int loopStart, int loopEnd);

//0为静音，57为标准音阶A，音符变0会使前一个音符进入播放完毕状态，然后停止，变非0会立即改变音阶,>=128会立即停止
void smpsynth_setNoteScale(int channel, unsigned char data);

//只决定下一个音符的起始音量，音符播放途中不受该操作影响
void smpsynth_setNoteVolume(int channel, unsigned char data);

//只决定下一个音符的起始平衡，音符播放途中不受该操作影响
void smpsynth_setNotePan(int channel, char data);

//设定速度
void smpsynth_setBpm(int bpm);

//生成所有通道混合的音频，同时使时间计时器推进sampleCount对应的时间（或者直接就以Sample为单位），返回生成的采样数
int smpsynth_getTickStream(SMPSYNTH_SOUND_SAMPLE_DATATYPE*streamData);
