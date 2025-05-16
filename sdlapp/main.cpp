#include <SDL_main.h>
#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include "smpsynth.h"

int init();
int loop();
void draw();
void release();
void applySamples(int channel);

bool running = false;

int main(int argc, char*argv[])
{
	if (init() == 0)
	{
		running = true;
		while (running)
		{
			if (loop())
				running = false;
			draw();
		}
		release();
	}
	return 0;
}

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
std::vector<short>audiobuffer;
SDL_AudioDeviceID audioDevId = 0;
SDL_AudioSpec usingAudioSpec = { 0 };
std::string status_text = "No Error.";
#define SMP_CHANNELS 4

std::vector<short>sample[SMP_CHANNELS];
std::vector<unsigned char>volEnv[SMP_CHANNELS];
std::vector<unsigned char>panEnv[SMP_CHANNELS];
int sampleSize[SMP_CHANNELS];
int volEnvSize[SMP_CHANNELS];
int panEnvSize[SMP_CHANNELS];
int volCh[SMP_CHANNELS];
int panCh[SMP_CHANNELS];
int sampleLoopStart[SMP_CHANNELS], sampleLoopEnd[SMP_CHANNELS];
int volLoopStart[SMP_CHANNELS], volLoopEnd[SMP_CHANNELS];
int panLoopStart[SMP_CHANNELS], panLoopEnd[SMP_CHANNELS];
unsigned char scaleCh[SMP_CHANNELS];
int inputChannel = 0;
int scaleShift = 48;

int init()
{
	SDL_Init(SDL_INIT_AUDIO);

	if (SDL_CreateWindowAndRenderer(640, 480, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL, &window, &renderer))
		return -1;
	SDL_SetWindowTitle(window, "SDL Window");

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = NULL;//不使用INI存储
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer2_Init(renderer);

	SDL_AudioSpec spec = { 0 };
	spec.format = AUDIO_S16;
	spec.channels = 2;
	spec.freq = 22050;
	spec.samples = 0;
	audioDevId = SDL_OpenAudioDevice(NULL, 0, &spec, &usingAudioSpec, 0);
	if(!audioDevId)
		status_text = SDL_GetError();
	smpsynth_init(usingAudioSpec.freq, SMP_CHANNELS);
	for (int i = 0; i < SMP_CHANNELS; i++)
	{
		sample[i].resize(sampleSize[i] = smpsynth_getStandardScaleSampleCount());
		for (int j = 0; j < sample[i].size(); j++)
			sample[i][j] = j < sample[i].size() / 2 ? -32768 : 32767;
		volEnv[i] = { SMPSYNTH_VOLUME_MAX,SMPSYNTH_VOLUME_MAX };
		panEnv[i] = { SMPSYNTH_PAN_CENTER,SMPSYNTH_PAN_CENTER };
		volEnvSize[i] = volEnv[i].size();
		panEnvSize[i] = panEnv[i].size();
		volCh[i] = SMPSYNTH_VOLUME_MAX;
		panCh[i] = SMPSYNTH_PAN_CENTER;
		sampleLoopStart[i] = 0;
		sampleLoopEnd[i] = sample[i].size();
		volLoopStart[i] = 0;
		volLoopEnd[i] = volEnv[i].size();
		panLoopStart[i] = 0;
		panLoopEnd[i] = panEnv[i].size();
		scaleCh[i] = 0;
		applySamples(i);
		smpsynth_setNoteScale(i, 0);
	}
	audiobuffer.resize(smpsynth_samplesPerTick()*SMPSYNTH_SOUND_CHANNELS);
	SDL_PauseAudioDevice(audioDevId, 0);
	return 0;
}

void applySamples(int channel)
{
	smpsynth_setStandardScaleSampleData(channel, sample[channel].data());
	smpsynth_setSampleVolumeEnvelope(channel, volEnv[channel].data(), volEnv[channel].size() * sizeof(volEnv[channel][0]));
	smpsynth_setSamplePanEnvelope(channel, panEnv[channel].data(), panEnv[channel].size() * sizeof(panEnv[channel][0]));
}

std::map<SDL_Scancode, int>keyMap = {
	std::make_pair(SDL_SCANCODE_Z,0),
	std::make_pair(SDL_SCANCODE_S,1),
	std::make_pair(SDL_SCANCODE_X,2),
	std::make_pair(SDL_SCANCODE_D,3),
	std::make_pair(SDL_SCANCODE_C,4),
	std::make_pair(SDL_SCANCODE_V,5),
	std::make_pair(SDL_SCANCODE_G,6),
	std::make_pair(SDL_SCANCODE_B,7),
	std::make_pair(SDL_SCANCODE_H,8),
	std::make_pair(SDL_SCANCODE_N,9),
	std::make_pair(SDL_SCANCODE_J,10),
	std::make_pair(SDL_SCANCODE_M,11),
	std::make_pair(SDL_SCANCODE_COMMA,12),
	std::make_pair(SDL_SCANCODE_L,13),
	std::make_pair(SDL_SCANCODE_PERIOD,14),
	std::make_pair(SDL_SCANCODE_SEMICOLON,15),
	std::make_pair(SDL_SCANCODE_SLASH,16),
	std::make_pair(SDL_SCANCODE_Q,12),
	std::make_pair(SDL_SCANCODE_2,13),
	std::make_pair(SDL_SCANCODE_W,14),
	std::make_pair(SDL_SCANCODE_3,15),
	std::make_pair(SDL_SCANCODE_E,16),
	std::make_pair(SDL_SCANCODE_R,17),
	std::make_pair(SDL_SCANCODE_5,18),
	std::make_pair(SDL_SCANCODE_T,19),
	std::make_pair(SDL_SCANCODE_6,20),
	std::make_pair(SDL_SCANCODE_Y,21),
	std::make_pair(SDL_SCANCODE_7,22),
	std::make_pair(SDL_SCANCODE_U,23),
	std::make_pair(SDL_SCANCODE_I,24),
	std::make_pair(SDL_SCANCODE_9,25),
	std::make_pair(SDL_SCANCODE_O,26),
	std::make_pair(SDL_SCANCODE_0,27),
	std::make_pair(SDL_SCANCODE_P,28),
	std::make_pair(SDL_SCANCODE_LEFTBRACKET,29),
	std::make_pair(SDL_SCANCODE_EQUALS,30),
	std::make_pair(SDL_SCANCODE_RIGHTBRACKET,31),
};

int mouse_x = -1, mouse_y = -1;

int loop()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		ImGui_ImplSDL2_ProcessEvent(&e);
		switch (e.type)
		{
		case SDL_QUIT:
			running = false;
			break;
		case SDL_WINDOWEVENT:
			switch (e.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
				status_text = "Window Size: " + std::to_string(e.window.data1) + "x" + std::to_string(e.window.data2);
				break;
			}
			break;
		case SDL_KEYDOWN:
			if (e.key.repeat == 0 && keyMap.find(e.key.keysym.scancode) != keyMap.end())
				smpsynth_setNoteScale(inputChannel, keyMap[e.key.keysym.scancode] + scaleShift);
			break;
		case SDL_KEYUP:
			smpsynth_setNoteScale(inputChannel, 0);
			break;
		case SDL_MOUSEBUTTONDOWN:
			mouse_x = e.motion.x;
			mouse_y = e.motion.y;
			break;
		case SDL_MOUSEBUTTONUP:
			mouse_x = -1;
			mouse_y = -1;
			break;
		case SDL_MOUSEMOTION:
			if (mouse_x != -1)
			{
				mouse_x = e.motion.x;
				mouse_y = e.motion.y;
			}
			break;
		}
	}

	//SDL_Audio尽量使用Push模式，可避免延迟问题
	while (SDL_GetQueuedAudioSize(audioDevId) < usingAudioSpec.samples*usingAudioSpec.channels*sizeof(short))
	{
		int sc = smpsynth_getTickStream(audiobuffer.data());
		SDL_QueueAudio(audioDevId, audiobuffer.data(), sc * sizeof(audiobuffer[0]));
	}
	return 0;
}

template<class Type>
bool draw_envelope(int x, int y, int width, int height, int padding, Type*data, size_t count, Type minValue, Type maxValue,
	int loopStart, int loopEnd, int mouseX, int mouseY)
{
	Uint8 old_r, old_g, old_b, old_a;
	SDL_GetRenderDrawColor(renderer, &old_r, &old_g, &old_b, &old_a);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_Rect r = { x-padding,y-padding,width+padding*2,height+padding*2 };
	SDL_RenderDrawRect(renderer, &r);
	int loopStart_x = SDL_min(x + loopStart * width / (count - 1), x + width);
	int loopEnd_x = SDL_min(x + loopEnd * width / (count - 1), x + width);
	SDL_RenderDrawLine(renderer, loopStart_x, y, loopStart_x, y + height);
	SDL_RenderDrawLine(renderer, loopEnd_x, y, loopEnd_x, y + height);
	SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
	for (size_t i = 0; i < count; i++)
	{
		int pt_x = x + i * width / (count - 1);
		int pt_y = y + height - (data[i] - minValue) * height / (maxValue - minValue);
		SDL_RenderDrawPoint(renderer, pt_x, pt_y);
	}
	SDL_SetRenderDrawColor(renderer, old_r, old_g, old_b, old_a);
	static int lastI = -1;
	if (mouseX >= x && mouseY >= y && mouseX <= x + width && mouseY <= y + height)
	{
		int i = (mouseX - x)*(count - 1) / width;
		if (i >= 0 && i < count)
		{
			Type v = (y + height - mouseY)*(maxValue - minValue) / height + minValue;
			data[i] = v;
			if (lastI != -1)
			{
				for (int _i = std::min(i, lastI); _i < std::max(i, lastI); _i++)
					data[_i] = v;
			}
			lastI = i;
			return true;
		}
	}
	else if (mouseX == -1)
	{
		lastI = -1;
	}
	return false;
}


void draw()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	//SDL绘图开始
	//SDL使用屏幕坐标系，原点在左上方
	if (draw_envelope(20, 20, 200, 100, 5, sample[inputChannel].data(), sample[inputChannel].size(), (short)-32768, (short)32767,sampleLoopStart[inputChannel],sampleLoopEnd[inputChannel], mouse_x, mouse_y))
		smpsynth_setSampleData(inputChannel, sample[inputChannel].data(), sample[inputChannel].size(), sampleLoopStart[inputChannel], sampleLoopEnd[inputChannel]);
	if (draw_envelope(20, 140, 200, 100, 5, volEnv[inputChannel].data(), volEnv[inputChannel].size(), (Uint8)0, (Uint8)255,volLoopStart[inputChannel],volLoopEnd[inputChannel], mouse_x, mouse_y))
		smpsynth_setSampleVolumeEnvelope(inputChannel, volEnv[inputChannel].data(), volEnv[inputChannel].size(),volLoopStart[inputChannel],volLoopEnd[inputChannel]);
	if (draw_envelope(20, 260, 200, 100, 5, panEnv[inputChannel].data(), panEnv[inputChannel].size(), (Uint8)0, (Uint8)255,panLoopStart[inputChannel],panLoopEnd[inputChannel], mouse_x, mouse_y))
		smpsynth_setSamplePanEnvelope(inputChannel, panEnv[inputChannel].data(), panEnv[inputChannel].size(),panLoopStart[inputChannel],panLoopEnd[inputChannel]);
	//SDL绘图结束

	// Start the Dear ImGui frame
	ImGui_ImplSDLRenderer2_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	//ImGUI开始
	if (ImGui::Begin("ImGUI"))
	{
		if (ImGui::BeginChild(1, ImVec2(300.0f, 50.0f)))
			ImGui::Text(status_text.c_str());
		ImGui::EndChild();
		ImGui::SliderInt("Scale Shift", &scaleShift, 0, 127);
		ImGui::SliderInt("Input Channel", &inputChannel, 0, SMP_CHANNELS - 1);
		if (ImGui::InputInt("Sample Count", &sampleSize[inputChannel]))
		{
			sampleSize[inputChannel] = SDL_max(2, sampleSize[inputChannel]);
			sample[inputChannel].resize(sampleSize[inputChannel]);
		}
		if (ImGui::InputInt("Volume Count", &volEnvSize[inputChannel]))
		{
			volEnvSize[inputChannel] = SDL_max(2, volEnvSize[inputChannel]);
			volEnv[inputChannel].resize(volEnvSize[inputChannel]);
		}
		if (ImGui::InputInt("Pan Count", &panEnvSize[inputChannel]))
		{
			panEnvSize[inputChannel] = SDL_max(2, panEnvSize[inputChannel]);
			panEnv[inputChannel].resize(panEnvSize[inputChannel]);
		}
		if (ImGui::SliderInt("Sample Loop Start", &sampleLoopStart[inputChannel], 0, sample[inputChannel].size()))
			smpsynth_setSampleData(inputChannel, sample[inputChannel].data(), sample[inputChannel].size(), sampleLoopStart[inputChannel], sampleLoopEnd[inputChannel]);
		if (ImGui::SliderInt("Sample Loop End", &sampleLoopEnd[inputChannel], 0, sample[inputChannel].size()))
			smpsynth_setSampleData(inputChannel, sample[inputChannel].data(), sample[inputChannel].size(), sampleLoopStart[inputChannel], sampleLoopEnd[inputChannel]);
		if (ImGui::SliderInt("Volume Loop Start", &volLoopStart[inputChannel], 0, volEnv[inputChannel].size()))
			smpsynth_setSampleVolumeEnvelope(inputChannel, volEnv[inputChannel].data(), volEnv[inputChannel].size(), volLoopStart[inputChannel], volLoopEnd[inputChannel]);
		if (ImGui::SliderInt("Volume Loop End", &volLoopEnd[inputChannel], 0, volEnv[inputChannel].size()))
			smpsynth_setSampleVolumeEnvelope(inputChannel, volEnv[inputChannel].data(), volEnv[inputChannel].size(), volLoopStart[inputChannel], volLoopEnd[inputChannel]);
		if (ImGui::SliderInt("Pan Loop Start", &panLoopStart[inputChannel], 0, panEnv[inputChannel].size()))
			smpsynth_setSamplePanEnvelope(inputChannel, panEnv[inputChannel].data(), panEnv[inputChannel].size(), panLoopStart[inputChannel], panLoopEnd[inputChannel]);
		if (ImGui::SliderInt("Pan Loop End", &panLoopEnd[inputChannel], 0, panEnv[inputChannel].size()))
			smpsynth_setSamplePanEnvelope(inputChannel, panEnv[inputChannel].data(), panEnv[inputChannel].size(), panLoopStart[inputChannel], panLoopEnd[inputChannel]);

		char label[16];
		SDL_snprintf(label, sizeof(label), "Volume Ch%d", inputChannel);
		if (ImGui::SliderInt(label, volCh + inputChannel, 0, SMPSYNTH_VOLUME_MAX))
			smpsynth_setNoteVolume(inputChannel, volCh[inputChannel]);
		SDL_snprintf(label, sizeof(label), "Pan Ch%d", inputChannel);
		if (ImGui::SliderInt(label, panCh + inputChannel, SMPSYNTH_PAN_LEFT, SMPSYNTH_PAN_RIGHT))
			smpsynth_setNotePan(inputChannel, panCh[inputChannel]);
	}
	ImGui::End();
	//ImGUI结束

	ImGui::Render();
	ImGuiIO& io = ImGui::GetIO();
	SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
	ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

	SDL_RenderPresent(renderer);
}

void release()
{
	SDL_CloseAudioDevice(audioDevId);
	// Cleanup
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
