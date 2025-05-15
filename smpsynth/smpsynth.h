#pragma once

#define SMPSYNTH_PAN_LEFT 0
#define SMPSYNTH_PAN_CENTER 128
#define SMPSYNTH_PAN_RIGHT 255
#define SMPSYNTH_VOLUME_MAX 255
#define SMPSYNTH_VOLUME_DEFAULT SMPSYNTH_VOLUME_MAX
#define SMPSYNTH_SOUND_CHANNELS 2//�̶�ʹ��˫����
#define SMPSYNTH_SOUND_SAMPLE_DATATYPE short//�̶�ʹ��16bit
#define SMPSYNTH_STANDARD_SCALE_NUMBER 57
#define SMPSYNTH_STANDARD_SCALE_FREQUENCY 440

//ָ���Ǻϳɹ�������������������������������
int smpsynth_init(int sampleRate = 44100, int channels = 8, int initial_bpm = 120, int ticksPerBeat = 24);

void smpsynth_release();

//Ĭ�Ͼ�����loopEndΪ0��ʾû��ѭ�����֣���������Ϊ-1��ʾ��β����λΪ������λ��
void smpsynth_setSampleData(int channel, SMPSYNTH_SOUND_SAMPLE_DATATYPE*data, int sampleCount, int loopStart = 0, int loopEnd = -1);
void smpsynth_setStandardScaleSampleData(int channel, SMPSYNTH_SOUND_SAMPLE_DATATYPE*data);
int smpsynth_getStandardScaleSampleCount();

//Ĭ��������
void smpsynth_setSampleVolumeEnvelope(int channel, unsigned char*data, int bytes, int loopStart = 0, int loopEnd = -1);

//Ĭ�Ͼ���
void smpsynth_setSamplePanEnvelope(int channel, unsigned char*data, int bytes, int loopStart = 0, int loopEnd = -1);

//0Ϊ������57Ϊ��׼����A��������0��ʹǰһ���������벥�����״̬��Ȼ��ֹͣ�����0�������ı�����,>=128������ֹͣ
void smpsynth_setNoteScale(int channel, unsigned char data);

//�趨ͨ��ʵʱ����
void smpsynth_setNoteVolume(int channel, unsigned char data);

//�趨ͨ��ʵʱƽ��
void smpsynth_setNotePan(int channel, char data);

//�趨�ٶ�
void smpsynth_setBpm(int bpm);

//��ȡ����֡�Ĳ�������
int smpsynth_samplesPerTick();

//��������ͨ����ϵ���Ƶ��ͬʱʹʱ���ʱ���ƽ�smpsynth_samplesPerTick()���صĲ�����������ֵΪ���ɵĲ�����
int smpsynth_getTickStream(SMPSYNTH_SOUND_SAMPLE_DATATYPE*streamData);

//TODO:����PITCH����
