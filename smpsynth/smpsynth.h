#pragma once

#define SMPSYNTH_PAN_LEFT 0
#define SMPSYNTH_PAN_CENTER 128
#define SMPSYNTH_PAN_RIGHT 255
#define SMPSYNTH_VOLUME_MAX 255
#define SMPSYNTH_VOLUME_DEFAULT SMPSYNTH_VOLUME_MAX
#define SMPSYNTH_SOUND_CHANNELS 2//�̶�ʹ��˫����
#define SMPSYNTH_SOUND_SAMPLE_DATATYPE short//�̶�ʹ��16bit
#define SMPSYNTH_STANDARD_SCALE 57

//ָ���Ǻϳɹ�������������������������������
int smpsynth_init(int sampleRate = 44100, int channels = 8, int initial_bpm = 120, int ticksPerBeat = 24);

void smpsynth_release();

//Ĭ�Ͼ�����loopEndΪ0��ʾû��ѭ�����֣�������
void smpsynth_setSampleData(int channel, SMPSYNTH_SOUND_SAMPLE_DATATYPE*data, int sampleCount, int loopStart, int loopEnd);

//Ĭ��������
void smpsynth_setSampleVolumeEnvelope(int channel, unsigned char*data, int bytes, int loopStart, int loopEnd);

//Ĭ�Ͼ���
void smpsynth_setSamplePanEnvelope(int channel, unsigned char*data, int bytes, int loopStart, int loopEnd);

//0Ϊ������57Ϊ��׼����A��������0��ʹǰһ���������벥�����״̬��Ȼ��ֹͣ�����0�������ı�����,>=128������ֹͣ
void smpsynth_setNoteScale(int channel, unsigned char data);

//ֻ������һ����������ʼ��������������;�в��ܸò���Ӱ��
void smpsynth_setNoteVolume(int channel, unsigned char data);

//ֻ������һ����������ʼƽ�⣬��������;�в��ܸò���Ӱ��
void smpsynth_setNotePan(int channel, char data);

//�趨�ٶ�
void smpsynth_setBpm(int bpm);

//��������ͨ����ϵ���Ƶ��ͬʱʹʱ���ʱ���ƽ�sampleCount��Ӧ��ʱ�䣨����ֱ�Ӿ���SampleΪ��λ�����������ɵĲ�����
int smpsynth_getTickStream(SMPSYNTH_SOUND_SAMPLE_DATATYPE*streamData);
