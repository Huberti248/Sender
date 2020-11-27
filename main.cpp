#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <iomanip>
#include <filesystem>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL_net.h>
//#include <SDL_gpu.h>
#include <SFML/Network.hpp>
//#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <functional>
#ifdef __ANDROID__
#include <android/log.h> //__android_log_print(ANDROID_LOG_VERBOSE, "Sender", "Example number log: %d", number);
#include <jni.h>
#include "vendor/PUGIXML/src/pugixml.hpp"
#else
#include <pugixml.hpp>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

// NOTE: Remember to uncomment it on every release
//#define RELEASE 1

#if defined _MSC_VER && defined RELEASE
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

//240 x 240 (smart watch)
//240 x 320 (QVGA)
//360 x 640 (Galaxy S5)
//640 x 480 (480i - Smallest PC monitor)

int windowWidth = 240;
int windowHeight = 320;
SDL_Point mousePos;
SDL_Point realMousePos;
bool keys[SDL_NUM_SCANCODES];
bool buttons[SDL_BUTTON_X2 + 1];
SDL_Renderer* renderer;

#define BG_COLOR 0,0,0,0
#define TEXT_COLOR 0,0,0,0

void logOutputCallback(void* userdata, int category, SDL_LogPriority priority, const char* message)
{
	std::cout << message << std::endl;
}

int random(int min, int max)
{
	return min + rand() % ((max + 1) - min);
}

int SDL_QueryTextureF(SDL_Texture* texture, Uint32* format, int* access, float* w, float* h)
{
	int wi, hi;
	int result = SDL_QueryTexture(texture, format, access, &wi, &hi);
	if (w) {
		*w = wi;
	}
	if (h) {
		*h = hi;
	}
	return result;
}

SDL_bool SDL_PointInFRect(const SDL_Point* p, const SDL_FRect* r)
{
	return ((p->x >= r->x) && (p->x < (r->x + r->w)) &&
		(p->y >= r->y) && (p->y < (r->y + r->h))) ? SDL_TRUE : SDL_FALSE;
}

std::ostream& operator<<(std::ostream& os, SDL_FRect r)
{
	os << r.x << " " << r.y << " " << r.w << " " << r.h;
	return os;
}

std::ostream& operator<<(std::ostream& os, SDL_Rect r)
{
	SDL_FRect fR;
	fR.w = r.w;
	fR.h = r.h;
	fR.x = r.x;
	fR.y = r.y;
	os << fR;
	return os;
}

struct Text {
	std::string text;
	SDL_Surface* surface = 0;
	SDL_Texture* t = 0;
	SDL_FRect dstR{};
	bool autoAdjustW = false;
	bool autoAdjustH = false;
	float wMultiplier = 1;
	float hMultiplier = 1;

	~Text()
	{
		if (surface) {
			SDL_FreeSurface(surface);
		}
		if (t) {
			SDL_DestroyTexture(t);
		}
	}

	Text()
	{

	}

	Text(const Text& rightText)
	{
		text = rightText.text;
		if (surface) {
			SDL_FreeSurface(surface);
		}
		if (t) {
			SDL_DestroyTexture(t);
		}
		if (rightText.surface) {
			surface = SDL_ConvertSurface(rightText.surface, rightText.surface->format, SDL_SWSURFACE);
		}
		if (rightText.t) {
			t = SDL_CreateTextureFromSurface(renderer, surface);
		}
		dstR = rightText.dstR;
		autoAdjustW = rightText.autoAdjustW;
		autoAdjustH = rightText.autoAdjustH;
		wMultiplier = rightText.wMultiplier;
		hMultiplier = rightText.hMultiplier;
	}

	Text& operator=(const Text& rightText)
	{
		text = rightText.text;
		if (surface) {
			SDL_FreeSurface(surface);
		}
		if (t) {
			SDL_DestroyTexture(t);
		}
		if (rightText.surface) {
			surface = SDL_ConvertSurface(rightText.surface, rightText.surface->format, SDL_SWSURFACE);
		}
		if (rightText.t) {
			t = SDL_CreateTextureFromSurface(renderer, surface);
		}
		dstR = rightText.dstR;
		autoAdjustW = rightText.autoAdjustW;
		autoAdjustH = rightText.autoAdjustH;
		wMultiplier = rightText.wMultiplier;
		hMultiplier = rightText.hMultiplier;
		return *this;
	}

	void setText(SDL_Renderer* renderer, TTF_Font* font, std::string text, SDL_Color c = { 255,255,255 })
	{
		this->text = text;
#if 1 // NOTE: renderText
		if (surface) {
			SDL_FreeSurface(surface);
		}
		if (t) {
			SDL_DestroyTexture(t);
		}
		if (text.empty()) {
			surface = TTF_RenderUTF8_Blended(font, " ", c);
		}
		else {
			surface = TTF_RenderUTF8_Blended(font, text.c_str(), c);
		}
		if (surface) {
			t = SDL_CreateTextureFromSurface(renderer, surface);
		}
#endif
		if (autoAdjustW) {
			SDL_QueryTextureF(t, 0, 0, &dstR.w, 0);
		}
		if (autoAdjustH) {
			SDL_QueryTextureF(t, 0, 0, 0, &dstR.h);
		}
		dstR.w *= wMultiplier;
		dstR.h *= hMultiplier;
	}

	void setText(SDL_Renderer* renderer, TTF_Font* font, int value, SDL_Color c = { 255,255,255 })
	{
		setText(renderer, font, std::to_string(value), c);
	}

	void draw(SDL_Renderer* renderer)
	{
		SDL_RenderCopyF(renderer, t, 0, &dstR);
	}
};

int eventWatch(void* userdata, SDL_Event* event)
{
	// WARNING: Be very careful of what you do in the function, as it may run in a different thread
	if (event->type == SDL_APP_TERMINATING || event->type == SDL_APP_WILLENTERBACKGROUND) {

	}
	return 0;
}

std::string getSystemFontFile(const std::string& faceName)
{
#ifdef _WIN32
	static const LPWSTR fontRegistryPath = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
	HKEY hKey;
	LONG result;
	std::wstring wsFaceName(faceName.begin(), faceName.end());

	// Open Windows font registry key
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, fontRegistryPath, 0, KEY_READ, &hKey);
	if (result != ERROR_SUCCESS) {
		return "";
	}

	DWORD maxValueNameSize, maxValueDataSize;
	result = RegQueryInfoKey(hKey, 0, 0, 0, 0, 0, 0, 0, &maxValueNameSize, &maxValueDataSize, 0, 0);
	if (result != ERROR_SUCCESS) {
		return "";
	}

	DWORD valueIndex = 0;
	LPWSTR valueName = new WCHAR[maxValueNameSize];
	LPBYTE valueData = new BYTE[maxValueDataSize];
	DWORD valueNameSize, valueDataSize, valueType;
	std::wstring wsFontFile;

	// Look for a matching font name
	do {

		wsFontFile.clear();
		valueDataSize = maxValueDataSize;
		valueNameSize = maxValueNameSize;

		result = RegEnumValue(hKey, valueIndex, valueName, &valueNameSize, 0, &valueType, valueData, &valueDataSize);

		valueIndex++;

		if (result != ERROR_SUCCESS || valueType != REG_SZ) {
			continue;
		}

		std::wstring wsValueName(valueName, valueNameSize);

		// Found a match
		if (_wcsnicmp(wsFaceName.c_str(), wsValueName.c_str(), wsFaceName.length()) == 0) {

			wsFontFile.assign((LPWSTR)valueData, valueDataSize);
			break;
		}
	} while (result != ERROR_NO_MORE_ITEMS);

	delete[] valueName;
	delete[] valueData;

	RegCloseKey(hKey);

	if (wsFontFile.empty()) {
		return "";
	}

	// Build full font file path
	WCHAR winDir[MAX_PATH];
	GetWindowsDirectory(winDir, MAX_PATH);

	std::wstringstream ss;
	ss << winDir << "\\Fonts\\" << wsFontFile;
	wsFontFile = ss.str();

	return std::string(wsFontFile.begin(), wsFontFile.end());
#else
	return faceName;
#endif
}

enum class State {
	LoginAndRegister,
	MessageList,
	MessageContent,
	MessageSend,
};

struct Message {
	Text topicText;
	Text senderNameText;
	Text senderSurnameText;
	Text dateText;
	Text contentText;
	SDL_FRect r{};
};

float getValueFromValueAndPercent(float value, float percent)
{
	return value * percent / 100.f;
}

enum MsSelectedWidget {
	Name,
	Surname,
	Topic,
	Content,

	Count, // WARNING: Keep it last
};

void drawInBorders(Text& text, SDL_FRect r, SDL_Renderer* renderer, TTF_Font* font)
{
	std::string currentText = text.text;
	while (text.dstR.x + text.dstR.w > r.x + r.w) {
		if (text.text.empty()) {
			break;
		}
		else {
			text.text.erase(0, 1);
			text.setText(renderer, font, text.text, { TEXT_COLOR });
		}
	}
	text.draw(renderer);
	text.setText(renderer, font, currentText, { TEXT_COLOR });
}

void sendMessage(Text& msNameInputText, Text& msSurnameInputText, Text& msTopicInputText, Text& msContentInputText, Text& nameInputText, Text& surnameInputText,
	MsSelectedWidget& msSelectedWidget, SDL_Renderer* renderer, TTF_Font* font)
{
	// TODO: Use more secure HTTP(s) -> curl
	// TODO: Do it on second thread (to don't block GUI) ???
	std::stringstream ss;
	ss
		<< "receiverName=" << msNameInputText.text
		<< "&receiverSurname=" << msSurnameInputText.text
		<< "&topic=" << msTopicInputText.text
		<< "&content=" << msContentInputText.text
		<< "&senderName=" << nameInputText.text
		<< "&senderSurname=" << surnameInputText.text;
	sf::Http::Request request("/", sf::Http::Request::Method::Post, ss.str());
	sf::Http http("http://senderprogram.000webhostapp.com/");
	sf::Http::Response response = http.sendRequest(request);
	// TODO: Handle errors? E.g. no internet connection.
	msNameInputText.setText(renderer, font, "");
	msSurnameInputText.setText(renderer, font, "");
	msTopicInputText.setText(renderer, font, "");
	msContentInputText.setText(renderer, font, "");
	msSelectedWidget = MsSelectedWidget::Name;
}

enum class Scroll {
	None,
	Up,
	Down,
};

int main(int argc, char* argv[])
{
	std::srand(std::time(0));
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
	SDL_LogSetOutputFunction(logOutputCallback, 0);
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	SDL_GetMouseState(&mousePos.x, &mousePos.y);
#if 1 && !RELEASE // TODO: Remember to turn it off on reelase
	SDL_Window * window = SDL_CreateWindow("Sender", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);
	windowWidth = dm.w;
	windowHeight = dm.h;
#else
	SDL_Window* window = SDL_CreateWindow("Sender", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP);
	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);
	windowWidth = dm.w;
	windowHeight = dm.h;
#endif
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	TTF_Font* robotoF = TTF_OpenFont("res/roboto.ttf", 72);
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	SDL_RenderSetScale(renderer, w / (float)windowWidth, h / (float)windowHeight);
	SDL_AddEventWatch(eventWatch, 0);
	bool running = true;
	State state = State::LoginAndRegister;
#if 1 // NOTE: LoginAndRegister
	Text nameText;
	nameText.setText(renderer, robotoF, u8"Imię");
	nameText.dstR.w = 40;
	nameText.dstR.h = 20;
	nameText.dstR.x = windowWidth / 2 - nameText.dstR.w / 2;
	nameText.dstR.y = 300;
	SDL_FRect nameR;
	nameR.w = 200;
	nameR.h = 30;
	nameR.x = windowWidth / 2 - nameR.w / 2;
	nameR.y = nameText.dstR.y + nameText.dstR.h + 6;
	Text nameInputText;
	nameInputText.setText(renderer, robotoF, "");
	nameInputText.dstR = nameR;
	nameInputText.dstR.x += 3;
	nameInputText.autoAdjustW = true;
	nameInputText.wMultiplier = 0.5;
	Text surnameText;
	surnameText.setText(renderer, robotoF, "Nazwisko");
	surnameText.dstR.w = 80;
	surnameText.dstR.h = 20;
	surnameText.dstR.x = windowWidth / 2 - surnameText.dstR.w / 2;
	surnameText.dstR.y = nameR.y + nameR.h + 15;
	SDL_FRect surnameR = nameR;
	surnameR.y = surnameText.dstR.y + surnameText.dstR.h + 6;
	Text surnameInputText;
	surnameInputText.setText(renderer, robotoF, "");
	surnameInputText.dstR = surnameR;
	surnameInputText.dstR.x += 3;
	surnameInputText.autoAdjustW = true;
	surnameInputText.wMultiplier = 0.5;
	bool isNameSelected = true;
#endif
#if 1 // NOTE: MessageList
	std::vector<Message> messages;
	SDL_Texture* writeMessageT = IMG_LoadTexture(renderer, "res/writeMessage.png");
	SDL_FRect writeMessageBtnR;
	writeMessageBtnR.w = 256;
	writeMessageBtnR.h = 60;
	writeMessageBtnR.x = 0;
	writeMessageBtnR.y = windowHeight - writeMessageBtnR.h;
#endif
#if 1 // NOTE: MessageContent
	SDL_Texture * closeT = IMG_LoadTexture(renderer, "res/close.png");
	SDL_FRect closeBtnR;
	closeBtnR.w = 64;
	closeBtnR.h = 64;
	closeBtnR.x = 0;
	closeBtnR.y = 0;
	Text topicText; // TODO: Do something when it goes out of right window border
	topicText.setText(renderer, robotoF, "");
	topicText.dstR = closeBtnR;
	topicText.dstR.h = 20;
	topicText.dstR.x = closeBtnR.x + closeBtnR.w;
	topicText.autoAdjustW = true;
	topicText.wMultiplier = 0.3;
	SDL_FRect contentR;
	contentR.w = windowWidth;
	contentR.h = windowHeight - closeBtnR.h;
	contentR.x = 0;
	contentR.y = closeBtnR.y + closeBtnR.h;
	SDL_FRect scrollR;
	scrollR.w = 20;
	scrollR.h = contentR.h;
	scrollR.x = windowWidth - scrollR.w;
	scrollR.y = contentR.y;
	contentR.w = windowWidth - scrollR.w;
	SDL_FRect scrollBtnR;
	scrollBtnR.w = scrollR.w;
	scrollBtnR.h = 30;
	scrollBtnR.x = scrollR.x;
	scrollBtnR.y = scrollR.y;
	Scroll scroll = Scroll::None;
	std::vector<Text> texts;
#endif
#if 1 // NOTE: MessageSend
	SDL_Texture * sendT = IMG_LoadTexture(renderer, "res/send.png");
	SDL_FRect msSendBtnR;
	msSendBtnR.w = 256;
	msSendBtnR.h = 64;
	msSendBtnR.x = 0;
	msSendBtnR.y = windowHeight - msSendBtnR.h;
	MsSelectedWidget msSelectedWidget = MsSelectedWidget::Name;
	SDL_FRect msNameR;
	msNameR.w = getValueFromValueAndPercent(windowWidth - closeBtnR.w, 100.f / 3.f);
	msNameR.h = getValueFromValueAndPercent(closeBtnR.h, 70);
	msNameR.x = closeBtnR.x + closeBtnR.w;
	msNameR.y = 0;
	Text msNameText;
	msNameText.setText(renderer, robotoF, u8"Imię", { TEXT_COLOR });
	msNameText.dstR.w = 40;
	msNameText.dstR.h = 20;
	msNameText.dstR.x = msNameR.x + msNameR.w / 2 - msNameText.dstR.w / 2;
	msNameText.dstR.y = 0;
	msNameR.y = msNameText.dstR.y + msNameText.dstR.h;
	Text msNameInputText;
	msNameInputText.setText(renderer, robotoF, "", { TEXT_COLOR });
	msNameInputText.dstR = msNameR;
	msNameInputText.dstR.h *= 0.5;
	msNameInputText.dstR.x += 3;
	msNameInputText.dstR.y = msNameR.y + msNameR.h / 2 - msNameInputText.dstR.h / 2;
	msNameInputText.autoAdjustW = true;
	msNameInputText.wMultiplier = 0.3;
	SDL_FRect msSurnameR = msNameR;
	msSurnameR.x = msNameR.x + msNameR.w;
	Text msSurnameInputText;
	msSurnameInputText.setText(renderer, robotoF, "", { TEXT_COLOR });
	msSurnameInputText.dstR = msSurnameR;
	msSurnameInputText.dstR.h *= 0.5;
	msSurnameInputText.dstR.x += 3;
	msSurnameInputText.dstR.y = msSurnameR.y + msSurnameR.h / 2 - msSurnameInputText.dstR.h / 2;
	msSurnameInputText.autoAdjustW = true;
	msSurnameInputText.wMultiplier = 0.3;
	Text msSurnameText;
	msSurnameText.setText(renderer, robotoF, "Nazwisko", { TEXT_COLOR });
	msSurnameText.dstR.w = 80;
	msSurnameText.dstR.h = 20;
	msSurnameText.dstR.x = msSurnameInputText.dstR.x + msSurnameInputText.dstR.w / 2 - msSurnameText.dstR.w / 2;
	msSurnameText.dstR.y = msNameText.dstR.y;
	msSurnameText.autoAdjustW = true;
	SDL_FRect msTopicR = msSurnameR;
	msTopicR.x = msSurnameR.x + msSurnameR.w;
	Text msTopicText;
	msTopicText.setText(renderer, robotoF, "Temat", { TEXT_COLOR });
	msTopicText.dstR.w = 60;
	msTopicText.dstR.h = 20;
	msTopicText.dstR.x = msTopicR.x + msTopicR.w / 2 - msTopicText.dstR.w / 2;
	msTopicText.dstR.y = msNameText.dstR.y;
	msTopicText.autoAdjustW = true;
	Text msTopicInputText;
	msTopicInputText.setText(renderer, robotoF, "", { TEXT_COLOR });
	msTopicInputText.dstR = msTopicR;
	msTopicInputText.dstR.h *= 0.5;
	msTopicInputText.dstR.x += 3;
	msTopicInputText.dstR.y = msTopicR.y + msTopicR.h / 2 - msTopicInputText.dstR.h / 2;
	msTopicInputText.autoAdjustW = true;
	msTopicInputText.wMultiplier = 0.3;
	SDL_FRect msContentR;
	msContentR.w = windowWidth;
	msContentR.h = windowHeight - closeBtnR.h - msSendBtnR.h;
	msContentR.x = 0;
	msContentR.y = closeBtnR.y + closeBtnR.h;
	Text msContentInputText;
	msContentInputText.setText(renderer, robotoF, "", { TEXT_COLOR });
	msContentInputText.dstR = closeBtnR;
	msContentInputText.dstR.h = 20;
	msContentInputText.dstR.x += 3;
	msContentInputText.dstR.y = closeBtnR.y + closeBtnR.h;
	msContentInputText.autoAdjustW = true;
	msContentInputText.wMultiplier = 0.3;
	bool textInputEventInMsContentInputText = false;
#endif
	int messageIndexToShow = -1;
	while (running) {
		if (state == State::LoginAndRegister) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					running = false;
					// TODO: On mobile remember to use eventWatch function (it doesn't reach this code when terminating)
				}
				if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
					SDL_RenderSetScale(renderer, event.window.data1 / (float)windowWidth, event.window.data2 / (float)windowHeight);
				}
				if (event.type == SDL_KEYDOWN) {
					keys[event.key.keysym.scancode] = true;
					if (event.key.keysym.scancode == SDL_SCANCODE_TAB) {
						isNameSelected = !isNameSelected;
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) {
						if (isNameSelected) {
							if (!nameInputText.text.empty()) {
								nameInputText.text.pop_back();
								nameInputText.setText(renderer, robotoF, nameInputText.text, { TEXT_COLOR });
							}
						}
						else {
							if (!surnameInputText.text.empty()) {
								surnameInputText.text.pop_back();
								surnameInputText.setText(renderer, robotoF, surnameInputText.text, { TEXT_COLOR });
							}
						}
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
						state = State::MessageList;
					}
				}
				if (event.type == SDL_KEYUP) {
					keys[event.key.keysym.scancode] = false;
				}
				if (event.type == SDL_MOUSEBUTTONDOWN) {
					buttons[event.button.button] = true;
					if (SDL_PointInFRect(&mousePos, &nameR)) {
						isNameSelected = true;
					}
					else if (SDL_PointInFRect(&mousePos, &surnameR)) {
						isNameSelected = false;
					}
				}
				if (event.type == SDL_MOUSEBUTTONUP) {
					buttons[event.button.button] = false;
				}
				if (event.type == SDL_MOUSEMOTION) {
					float scaleX, scaleY;
					SDL_RenderGetScale(renderer, &scaleX, &scaleY);
					mousePos.x = event.motion.x / scaleX;
					mousePos.y = event.motion.y / scaleY;
					realMousePos.x = event.motion.x;
					realMousePos.y = event.motion.y;
				}
				if (event.type == SDL_TEXTINPUT) {
					if (isNameSelected) {
						nameInputText.setText(renderer, robotoF, nameInputText.text + event.text.text, { TEXT_COLOR });
					}
					else {
						surnameInputText.setText(renderer, robotoF, surnameInputText.text + event.text.text, { TEXT_COLOR });
					}
				}
			}
			SDL_SetRenderDrawColor(renderer, BG_COLOR);
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderFillRectF(renderer, &nameR);
			SDL_RenderFillRectF(renderer, &surnameR);
			SDL_SetRenderDrawColor(renderer, 52, 131, 235, 255);
			if (isNameSelected) {
				SDL_RenderDrawRectF(renderer, &nameR);
			}
			else {
				SDL_RenderDrawRectF(renderer, &surnameR);
			}
			nameText.draw(renderer);
			surnameText.draw(renderer);
			drawInBorders(nameInputText, nameR, renderer, robotoF);
			drawInBorders(surnameInputText, surnameR, renderer, robotoF);
			SDL_RenderPresent(renderer);
		}
		else if (state == State::MessageList) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					running = false;
					// TODO: On mobile remember to use eventWatch function (it doesn't reach this code when terminating)
				}
				if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
					SDL_RenderSetScale(renderer, event.window.data1 / (float)windowWidth, event.window.data2 / (float)windowHeight);
				}
				if (event.type == SDL_KEYDOWN) {
					keys[event.key.keysym.scancode] = true;
				}
				if (event.type == SDL_KEYUP) {
					keys[event.key.keysym.scancode] = false;
				}
				if (event.type == SDL_MOUSEBUTTONDOWN) {
					buttons[event.button.button] = true;
					for (int i = 0; i < messages.size(); ++i) {
						if (messages[i].r.y + messages[i].r.h < writeMessageBtnR.y) {
							if (SDL_PointInFRect(&mousePos, &messages[i].r)) {
								state = State::MessageContent;
								messageIndexToShow = i;
								texts.clear();
								{
									std::stringstream ss(messages[messageIndexToShow].contentText.text);
									texts.push_back(Text());
									texts.back() = messages[messageIndexToShow].contentText;
#if 1 // NOTE: Purpose - delete new line characters which shouldn't be displayed + split into new lines
									while (std::getline(ss, texts.back().text, '\n')) {
										texts.back().autoAdjustW = true;
										texts.back().wMultiplier = 0.3;
										texts.back().dstR.h = writeMessageBtnR.h / 3;
										texts.back().dstR.x = 0;
										texts.back().dstR.y = closeBtnR.y + closeBtnR.h;
										texts.back().setText(renderer, robotoF, texts.back().text);
										texts.push_back(Text());
										texts.back() = messages[messageIndexToShow].contentText;
									}
									texts.pop_back();
#endif
									if (!texts.empty()) {
										for (int i = 1; i < texts.size(); ++i) {
											texts[i].dstR.y = texts[i - 1].dstR.y + texts[i - 1].dstR.h;
										}
										if (scroll == Scroll::Up) {
											for (Text& text : texts) {
												text.dstR.y += text.dstR.h;
											}
										}
										else if (scroll == Scroll::Down) {
											for (Text& text : texts) {
												text.dstR.y -= text.dstR.h;
											}
										}
										scroll = Scroll::None;
									}
								}
								scrollBtnR.y = scrollR.y;
								break;
							}
						}
					}
					if (SDL_PointInFRect(&mousePos, &writeMessageBtnR)) {
						state = State::MessageSend;
					}
				}
				if (event.type == SDL_MOUSEBUTTONUP) {
					buttons[event.button.button] = false;
				}
				if (event.type == SDL_MOUSEMOTION) {
					float scaleX, scaleY;
					SDL_RenderGetScale(renderer, &scaleX, &scaleY);
					mousePos.x = event.motion.x / scaleX;
					mousePos.y = event.motion.y / scaleY;
					realMousePos.x = event.motion.x;
					realMousePos.y = event.motion.y;
				}
				if (event.type == SDL_MOUSEWHEEL) {
					if (event.wheel.y > 0) // scroll up
					{
						if (!messages.empty()) {
							float minY = messages.front().r.y;
							for (int i = 1; i < messages.size(); ++i) {
								minY = std::min(minY, messages[i].r.y);
							}
							if (minY < 0) {
								for (Message& message : messages) {
									message.r.y += windowHeight;
									message.topicText.dstR.y += windowHeight;
									message.senderNameText.dstR.y += windowHeight;
									message.senderSurnameText.dstR.y += windowHeight;
									message.dateText.dstR.y += windowHeight;
								}
							}
						}
					}
					else if (event.wheel.y < 0) // scroll down
					{
						if (!messages.empty()) {
							float maxY = messages.front().r.y;
							for (int i = 1; i < messages.size(); ++i) {
								maxY = std::max(maxY, messages[i].r.y);
							}
							if (maxY >= windowHeight) {
								for (Message& message : messages) {
									message.r.y -= windowHeight;
									message.topicText.dstR.y -= windowHeight;
									message.senderNameText.dstR.y -= windowHeight;
									message.senderSurnameText.dstR.y -= windowHeight;
									message.dateText.dstR.y -= windowHeight;
								}
							}
						}
					}
				}
			}
			// TODO: Use more secure HTTP(s) -> curl
			// TODO: Do this from time to time + do so that it won't block UI
			std::stringstream ss;
			ss << "name=" << nameInputText.text << "&surname=" << surnameInputText.text;
			sf::Http::Request request("/", sf::Http::Request::Method::Post, ss.str());
			sf::Http http("http://senderprogram.000webhostapp.com/");
			sf::Http::Response response = http.sendRequest(request);
			if (response.getStatus() == sf::Http::Response::Status::Ok) {
				/*
				ASCII Oct	Unicode Name					Common Name			Usage
				34			INFORMATION SEPARATOR FOUR		file separator		End of file. Or between a concatenation of what might otherwise be separate files.
				35			INFORMATION SEPARATOR THREE		group separator		Between sections of data. Not needed in simple data files.
				36			INFORMATION SEPARATOR TWO		record separator	End of a record or row.
				37			INFORMATION SEPARATOR ONE		unit separator		Between fields of a record, or members of a row.
				*/
				std::string body = response.getBody();
				{
					std::stringstream ss(body);
					std::getline(ss, body, '\035');
					std::getline(ss, body, '\035');
				}
				{
					std::stringstream ss(body);
					std::vector<Message> newMessages;
					std::string message;
					{
						int i = 0;
						while (std::getline(ss, message, '\036')) {
							std::stringstream ss(message);
							newMessages.push_back(Message());
							std::getline(ss, newMessages.back().topicText.text, '\037');
							std::getline(ss, newMessages.back().senderNameText.text, '\037');
							std::getline(ss, newMessages.back().senderSurnameText.text, '\037');
							std::getline(ss, newMessages.back().dateText.text, '\037');
							std::getline(ss, newMessages.back().contentText.text, '\037');

							newMessages.back().r.w = windowWidth;
							newMessages.back().r.h = 20;
							newMessages.back().r.x = 0;
							newMessages.back().r.y = newMessages.back().r.h * i;

							newMessages.back().topicText.dstR = newMessages.back().r;
							newMessages.back().topicText.autoAdjustW = true;
							newMessages.back().topicText.setText(renderer, robotoF, newMessages.back().topicText.text);
							newMessages.back().topicText.dstR.w *= 0.3;

							newMessages.back().dateText.dstR = newMessages.back().r;
							newMessages.back().dateText.autoAdjustW = true;
							newMessages.back().dateText.setText(renderer, robotoF, newMessages.back().dateText.text);
							newMessages.back().dateText.dstR.w *= 0.3;
							newMessages.back().dateText.dstR.x = windowWidth - newMessages.back().dateText.dstR.w - 3;

							newMessages.back().senderSurnameText.dstR = newMessages.back().r;
							newMessages.back().senderSurnameText.autoAdjustW = true;
							newMessages.back().senderSurnameText.setText(renderer, robotoF, newMessages.back().senderSurnameText.text);
							newMessages.back().senderSurnameText.dstR.w *= 0.3;
							newMessages.back().senderSurnameText.dstR.x = newMessages.back().dateText.dstR.x - newMessages.back().senderSurnameText.dstR.w - 10;

							newMessages.back().senderNameText.dstR = newMessages.back().r;
							newMessages.back().senderNameText.autoAdjustW = true;
							newMessages.back().senderNameText.setText(renderer, robotoF, newMessages.back().senderNameText.text);
							newMessages.back().senderNameText.dstR.w *= 0.3;
							newMessages.back().senderNameText.dstR.x = newMessages.back().senderSurnameText.dstR.x - newMessages.back().senderNameText.dstR.w - 10;

							newMessages.back().contentText.dstR = newMessages.back().topicText.dstR;
							newMessages.back().contentText.dstR.x = closeBtnR.x + closeBtnR.w + 2;
							newMessages.back().contentText.autoAdjustW = true;
							newMessages.back().contentText.setText(renderer, robotoF, newMessages.back().contentText.text);
							newMessages.back().contentText.dstR.w *= 0.3;
							++i;
						}
					}
					if (messages.size() != newMessages.size()) {
						messages = newMessages;
					}
					else {
						for (int i = 0; i < messages.size(); ++i) {
							if (messages[i].topicText.text != newMessages[i].topicText.text ||
								messages[i].senderNameText.text != newMessages[i].senderNameText.text ||
								messages[i].senderSurnameText.text != newMessages[i].senderSurnameText.text ||
								messages[i].dateText.text != newMessages[i].dateText.text ||
								messages[i].contentText.text != newMessages[i].contentText.text) {
								messages = newMessages;
								break;
							}
						}
					}
				}
			}
			// TODO: Handle errors? E.g. no internet connection.
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			SDL_RenderClear(renderer);
			for (int i = 0; i < messages.size(); ++i) {
				if (messages[i].r.y + messages[i].r.h < writeMessageBtnR.y) {
					SDL_SetRenderDrawColor(renderer, 37, 37, 68, 0);
					SDL_RenderFillRectF(renderer, &messages[i].r);
					SDL_SetRenderDrawColor(renderer, 25, 25, 25, 0);
					SDL_RenderDrawRectF(renderer, &messages[i].r);
					messages[i].topicText.draw(renderer);
					messages[i].senderNameText.draw(renderer);
					messages[i].senderSurnameText.draw(renderer);
					messages[i].dateText.draw(renderer);
				}
			}
			SDL_RenderCopyF(renderer, writeMessageT, 0, &writeMessageBtnR);
			SDL_RenderPresent(renderer);
		}
		else if (state == State::MessageContent) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					running = false;
					// TODO: On mobile remember to use eventWatch function (it doesn't reach this code when terminating)
				}
				if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
					SDL_RenderSetScale(renderer, event.window.data1 / (float)windowWidth, event.window.data2 / (float)windowHeight);
				}
				if (event.type == SDL_KEYDOWN) {
					keys[event.key.keysym.scancode] = true;
				}
				if (event.type == SDL_KEYUP) {
					keys[event.key.keysym.scancode] = false;
				}
				if (event.type == SDL_MOUSEBUTTONDOWN) {
					buttons[event.button.button] = true;
					if (SDL_PointInFRect(&mousePos, &closeBtnR)) {
						state = State::MessageList;
					}
				}
				if (event.type == SDL_MOUSEBUTTONUP) {
					buttons[event.button.button] = false;
				}
				if (event.type == SDL_MOUSEMOTION) {
					float scaleX, scaleY;
					SDL_RenderGetScale(renderer, &scaleX, &scaleY);
					mousePos.x = event.motion.x / scaleX;
					mousePos.y = event.motion.y / scaleY;
					realMousePos.x = event.motion.x;
					realMousePos.y = event.motion.y;
				}
				if (event.type == SDL_MOUSEWHEEL) {
					if (event.wheel.y > 0) // scroll up
					{
						scroll = Scroll::Up;
					}
					else if (event.wheel.y < 0) // scroll down
					{
						scroll = Scroll::Down;
					}
				}
			}
			topicText.setText(renderer, robotoF, messages[messageIndexToShow].topicText.text);
			topicText.dstR.x = closeBtnR.x + closeBtnR.w + (windowWidth - closeBtnR.w) / 2 - topicText.dstR.w / 2;
			topicText.dstR.y = closeBtnR.h / 2 - topicText.dstR.h / 2;
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			SDL_RenderClear(renderer);
			SDL_RenderCopyF(renderer, closeT, 0, &closeBtnR);
			topicText.draw(renderer);
			if (!texts.empty()) {
				if (scroll == Scroll::Up) {
					if (texts.front().dstR.y != contentR.y) {
						for (Text& text : texts) {
							text.dstR.y += text.dstR.h;
						}
						float maxVisible = contentR.h / texts.front().dstR.h; // NOTE: Assume that items have same height
						float percent = 100 / (texts.size() - maxVisible);
						float max = 0 + scrollR.h - scrollBtnR.h;
						float min = 0;
						scrollBtnR.y -= (percent * (max - min) / 100) + min;
					}
				}
				else if (scroll == Scroll::Down) {
					if (texts.back().dstR.y + texts.back().dstR.h > contentR.y + contentR.h) {
						for (Text& text : texts) {
							text.dstR.y -= text.dstR.h;
						}
						float maxVisible = contentR.h / texts.front().dstR.h; // NOTE: Assume that items have same height
						float percent = 100 / (texts.size() - maxVisible);
						float max = 0 + scrollR.h - scrollBtnR.h;
						float min = 0;
						scrollBtnR.y += (percent * (max - min) / 100) + min;
					}
				}
				scroll = Scroll::None;
				for (Text& text : texts) {
					if (text.dstR.y >= contentR.y) {
						text.draw(renderer);
					}
				}
				float maxVisible = contentR.h / texts.front().dstR.h; // NOTE: Assume that items have same height
				if (maxVisible < texts.size()) {
					SDL_SetRenderDrawColor(renderer, 23, 23, 23, 255);
					SDL_RenderFillRectF(renderer, &scrollR);
					SDL_SetRenderDrawColor(renderer, 77, 77, 77, 255);
					SDL_RenderFillRectF(renderer, &scrollBtnR);
				}
			}
			SDL_RenderPresent(renderer);
		}
		else if (state == State::MessageSend) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				if (msSelectedWidget == MsSelectedWidget::Name) {
					if (event.type == SDL_TEXTINPUT) {
						msNameInputText.setText(renderer, robotoF, msNameInputText.text + event.text.text, { TEXT_COLOR });
					}
					if (event.type == SDL_KEYDOWN) {
						if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) {
							if (!msNameInputText.text.empty()) {
								msNameInputText.text.pop_back();
								msNameInputText.setText(renderer, robotoF, msNameInputText.text, { TEXT_COLOR });
							}
						}
					}
				}
				else if (msSelectedWidget == MsSelectedWidget::Surname) {
					if (event.type == SDL_TEXTINPUT) {
						msSurnameInputText.setText(renderer, robotoF, msSurnameInputText.text + event.text.text, { TEXT_COLOR });
					}
					if (event.type == SDL_KEYDOWN) {
						if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) {
							if (!msSurnameInputText.text.empty()) {
								msSurnameInputText.text.pop_back();
								msSurnameInputText.setText(renderer, robotoF, msSurnameInputText.text, { TEXT_COLOR });
							}
						}
					}
				}
				else if (msSelectedWidget == MsSelectedWidget::Topic) {
					if (event.type == SDL_TEXTINPUT) {
						msTopicInputText.setText(renderer, robotoF, msTopicInputText.text + event.text.text, { TEXT_COLOR });
					}
					if (event.type == SDL_KEYDOWN) {
						if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) {
							if (!msTopicInputText.text.empty()) {
								msTopicInputText.text.pop_back();
								msTopicInputText.setText(renderer, robotoF, msTopicInputText.text, { TEXT_COLOR });
							}
						}
					}
				}
				else if (msSelectedWidget == MsSelectedWidget::Content) {
					if (event.type == SDL_TEXTINPUT) {
						msContentInputText.setText(renderer, robotoF, msContentInputText.text + event.text.text, { TEXT_COLOR });
						textInputEventInMsContentInputText = true;
					}
					if (event.type == SDL_KEYDOWN) {
						if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) {
							if (!msContentInputText.text.empty()) {
								msContentInputText.text.pop_back();
								if (!msContentInputText.text.empty() && msContentInputText.text.back() == '\036') {
									msContentInputText.text.pop_back();
								}
								msContentInputText.setText(renderer, robotoF, msContentInputText.text, { TEXT_COLOR });
							}
						}
						if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
							msContentInputText.setText(renderer, robotoF, msContentInputText.text + "\n", { TEXT_COLOR });
						}
					}
				}

				if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					running = false;
					// TODO: On mobile remember to use eventWatch function (it doesn't reach this code when terminating)
				}
				if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
					SDL_RenderSetScale(renderer, event.window.data1 / (float)windowWidth, event.window.data2 / (float)windowHeight);
				}
				if (event.type == SDL_KEYDOWN) {
					keys[event.key.keysym.scancode] = true;
					if (event.key.keysym.scancode == SDL_SCANCODE_TAB) {
						msSelectedWidget = (MsSelectedWidget)((int)msSelectedWidget + 1);
						if (msSelectedWidget >= MsSelectedWidget::Count) {
							msSelectedWidget = (MsSelectedWidget)0;
						}
					}
				}
				if (event.type == SDL_KEYUP) {
					keys[event.key.keysym.scancode] = false;
				}
				if (event.type == SDL_MOUSEBUTTONDOWN) {
					buttons[event.button.button] = true;
					if (SDL_PointInFRect(&mousePos, &closeBtnR)) {
						state = State::MessageList;
					}
					if (SDL_PointInFRect(&mousePos, &msSendBtnR)) {
						sendMessage(msNameInputText, msSurnameInputText, msTopicInputText, msContentInputText, nameInputText, surnameInputText, msSelectedWidget, renderer, robotoF);
					}
					else if (SDL_PointInFRect(&mousePos, &msNameR)) {
						msSelectedWidget = MsSelectedWidget::Name;
					}
					else if (SDL_PointInFRect(&mousePos, &msSurnameR)) {
						msSelectedWidget = MsSelectedWidget::Surname;
					}
					else if (SDL_PointInFRect(&mousePos, &msTopicR)) {
						msSelectedWidget = MsSelectedWidget::Topic;
					}
					else if (SDL_PointInFRect(&mousePos, &msContentR)) {
						msSelectedWidget = MsSelectedWidget::Content;
					}
				}
				if (event.type == SDL_MOUSEBUTTONUP) {
					buttons[event.button.button] = false;
				}
				if (event.type == SDL_MOUSEMOTION) {
					float scaleX, scaleY;
					SDL_RenderGetScale(renderer, &scaleX, &scaleY);
					mousePos.x = event.motion.x / scaleX;
					mousePos.y = event.motion.y / scaleY;
					realMousePos.x = event.motion.x;
					realMousePos.y = event.motion.y;
				}
			}
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderFillRectF(renderer, &msNameR);
			SDL_RenderFillRectF(renderer, &msSurnameR);
			SDL_RenderFillRectF(renderer, &msTopicR);
			SDL_RenderFillRectF(renderer, &msContentR);
			SDL_RenderCopyF(renderer, closeT, 0, &closeBtnR);
			msNameText.draw(renderer);
			msSurnameText.draw(renderer);
			msTopicText.draw(renderer);
			drawInBorders(msNameInputText, msNameR, renderer, robotoF);
			drawInBorders(msSurnameInputText, msSurnameR, renderer, robotoF);
			drawInBorders(msTopicInputText, msTopicR, renderer, robotoF);

			{
				std::vector<Text> texts;
				std::stringstream ss(msContentInputText.text);
				texts.push_back(Text());
				texts.back() = msContentInputText;
#if 1 // NOTE: Purpose - delete new line characters which shouldn't be displayed + split into new lines
				while (std::getline(ss, texts.back().text, '\n')) {
					texts.back().setText(renderer, robotoF, texts.back().text, { TEXT_COLOR });
					texts.push_back(Text());
					texts.back() = msContentInputText;
				}
				texts.pop_back();
#endif
				if (!texts.empty()) {
					if (textInputEventInMsContentInputText) {
						textInputEventInMsContentInputText = false;
						if (texts.back().dstR.x + texts.back().dstR.w > msContentR.x + msContentR.w - 20) {
							std::size_t lastSpacePos = texts.back().text.rfind(" ");
							if (lastSpacePos != std::string::npos) {
								msContentInputText.text[msContentInputText.text.rfind(" ")] = '\n';
								texts.back().text[lastSpacePos] = '\n';
								texts.back().setText(renderer, robotoF, texts.back().text.substr(0, lastSpacePos + 1), { TEXT_COLOR });
								texts.push_back(Text());
								texts.back() = msContentInputText;
								texts.back().text = texts.back().text.substr(lastSpacePos + 1);
							}
							else {
								msContentInputText.text.push_back('\n');
							}
						}
					}
					for (int i = 1; i < texts.size(); ++i) {
						texts[i].dstR.y = texts[i - 1].dstR.y + texts[i - 1].dstR.h;
					}
					bool shouldScrollUp = false;
					for (Text& text : texts) {
						if (text.dstR.y < msContentR.y) {
							shouldScrollUp = true;
							break;
						}
					}
					if (texts.back().dstR.y + texts.back().dstR.h > msContentR.y + msContentR.h) {
						while (texts.back().dstR.y + texts.back().dstR.h > msContentR.y + msContentR.h) {
							for (Text& text : texts) {
								text.dstR.y -= text.dstR.h;
							}
						}
					}
					else if (shouldScrollUp) {
						for (Text& text : texts) {
							text.dstR.y += text.dstR.h;
						}
					}
					for (Text& text : texts) {
						if (text.dstR.y >= msContentR.y) {
							text.draw(renderer);
						}
					}
				}
			}

			SDL_SetRenderDrawColor(renderer, 52, 131, 235, 255);
			if (msSelectedWidget == MsSelectedWidget::Name) {
				SDL_RenderDrawRectF(renderer, &msNameR);
			}
			else if (msSelectedWidget == MsSelectedWidget::Surname) {
				SDL_RenderDrawRectF(renderer, &msSurnameR);
			}
			else if (msSelectedWidget == MsSelectedWidget::Topic) {
				SDL_RenderDrawRectF(renderer, &msTopicR);
			}
			else if (msSelectedWidget == MsSelectedWidget::Content) {
				SDL_RenderDrawRectF(renderer, &msContentR);
			}
			SDL_RenderCopyF(renderer, sendT, 0, &msSendBtnR);
			SDL_RenderPresent(renderer);
		}
	}
	// TODO: On mobile remember to use eventWatch function (it doesn't reach this code when terminating)
	return 0;
}