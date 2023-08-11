#include <stdio.h>
#include <vector>
#include <SDL.h>
#include <Windows.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer.h"
#include "math.h"

#if !SDL_VERSION_ATLEAST(2,0,17)
#error The SDL backend has to be 2.0.17+ because of SDL_RenderGeomatry() function
#endif

std::vector<SDL_Point> points;

const int Width = 1500;
const int Height = 750;

SDL_Rect winRect{ 0, 0, Width, Height };

int main(int argc, char* argv[]) {
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		printf("[ERROR] %s\n", SDL_GetError());
		return -1;
	}

	ShowWindow(GetConsoleWindow(), SW_HIDE);

#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

	SDL_Window* window = SDL_CreateWindow("Wave Maker", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_BORDERLESS);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (!window || !renderer) {
		SDL_Log("ERROR CREATING WINDOW OR RENDERER");
		return -1;
	}

	// imgui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer_Init(renderer);

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 12.0f;
	style.WindowTitleAlign = ImVec2(0.50f, 0.35f);
	style.FrameRounding = 12.0f;
	style.GrabRounding = 12.0f;

	ImVec4 background = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
	ImVec4 waveColor = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
	ImVec4 waveGlowColor = ImVec4(0.00f, 0.00f, 0.00f, 0.05f);

	bool displayWave{ false };
	bool renderClear{ true };
	bool waveGlow{ false };

	bool blendMode{ false };

	float waveSpeed{ -0.0001f };
	float frequency{ 4.0f };
	double time{ 0 };
	points.resize(Width + 1);

	bool running{ true };
	while (running) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			ImGui_ImplSDL2_ProcessEvent(&e);
			switch (e.type) {
			case SDL_QUIT:
				running = false;
				break;
			}
		}

		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		{
			ImGui::Begin("CONTROL PANEL");

			if (ImGui::Button("TERMINATE"))
				running = false;

			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			ImGui::Checkbox("Show Wave", &displayWave);
			ImGui::Checkbox("Wave Glow", &waveGlow);
			ImGui::Checkbox("Render Clear", &renderClear);

			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			ImGui::SliderFloat("Frequency", &frequency, -50.0f, 50.0f);
			ImGui::SliderFloat("Wave Speed", &waveSpeed, -0.01f, 0.01f, "%.6f");

			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			ImGui::ColorEdit3("Background Color", (float*)&background, ImGuiColorEditFlags_PickerHueWheel);
			ImGui::ColorEdit3("Wave Color", (float*)&waveColor, ImGuiColorEditFlags_PickerHueWheel);
			ImGui::ColorEdit4("Wave Glow Color", (float*)&waveGlowColor);

			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			ImGui::Text("App average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

			ImGui::End();
		}

		ImGui::Render();
		SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		if (renderClear)
			SDL_RenderClear(renderer);

		if (waveGlow && !renderClear) {
			if (!blendMode) {
				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
				blendMode *= 1;
			}
			SDL_SetRenderDrawColor(renderer, (uint8_t)(waveGlowColor.x * 255), (uint8_t)(waveGlowColor.y * 255), (uint8_t)(waveGlowColor.z * 255), (uint8_t)(waveGlowColor.w * 255));
			SDL_RenderFillRect(renderer, &winRect);

		}

		SDL_SetRenderDrawColor(renderer, (uint8_t)(waveColor.x * 255), (uint8_t)(waveColor.y * 255), (uint8_t)(waveColor.z * 255), (uint8_t)(waveColor.w * 255));
		if (displayWave) {
			for (int i = 0; i < Width; ++i) {
				points[i].x = i;
				points[i].y = static_cast<int>(sinFuncs::getY(time, i, Width, Height - 20, frequency));
				if (points[i].x < points[i + 1].x)
					SDL_RenderDrawLine(renderer, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);

				time += waveSpeed * 1000.0f / io.Framerate;
			}
		}

		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
		SDL_SetRenderDrawColor(renderer, (uint8_t)(background.x * 255), (uint8_t)(background.y * 255), (uint8_t)(background.z * 255), (uint8_t)(background.w * 255));
		SDL_RenderPresent(renderer);
	}

	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}