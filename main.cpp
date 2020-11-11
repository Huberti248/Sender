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

// NOTE: Remember to uncomment it on every release
//#define RELEASE

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

SDL_Texture* renderText(SDL_Texture* previousTexture, TTF_Font* font, SDL_Renderer* renderer, const std::string& text, SDL_Color color)
{
	if (previousTexture) {
		SDL_DestroyTexture(previousTexture);
	}
	SDL_Surface* surface;
	if (text.empty()) {
		surface = TTF_RenderUTF8_Blended(font, " ", color);
	}
	else {
		surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
	}
	if (surface) {
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface);
		return texture;
	}
	else {
		return 0;
	}
}

struct Text {
	std::string text;
	SDL_Texture* t = 0;
	SDL_FRect dstR{};
	bool autoAdjustW = false;
	bool autoAdjustH = false;

	~Text()
	{
		if (t) {
			SDL_DestroyTexture(t);
		}
	}

	void setText(SDL_Renderer* renderer, TTF_Font* font, std::string text, SDL_Color c = { 255,255,255 })
	{
		this->text = text;
		t = renderText(t, font, renderer, text, c);
		if (autoAdjustW) {
			SDL_QueryTextureF(t, 0, 0, &dstR.w, 0);
		}
		if (autoAdjustH) {
			SDL_QueryTextureF(t, 0, 0, 0, &dstR.h);
		}
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

void inputHandleEvent(SDL_Event event, Text& inputText, SDL_Renderer* renderer, TTF_Font* font)
{
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) {
			if (!inputText.text.empty()) {
				inputText.text.pop_back();
				inputText.setText(renderer, font, inputText.text, { TEXT_COLOR });
			}
		}
	}
	if (event.type == SDL_TEXTINPUT) {
		inputText.setText(renderer, font, inputText.text + event.text.text, { TEXT_COLOR });
	}
}

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

int main(int argc, char* argv[])
{
	std::srand(std::time(0));
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
	SDL_LogSetOutputFunction(logOutputCallback, 0);
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	SDL_GetMouseState(&mousePos.x, &mousePos.y);
#if 1 // TODO: Remember to turn it off on reelase
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
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
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
	nameText.dstR.w = 50;
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
	nameInputText.autoAdjustW = true;
	Text surnameText;
	surnameText.setText(renderer, robotoF, "Nazwisko");
	surnameText.dstR.w = 100;
	surnameText.dstR.h = 20;
	surnameText.dstR.x = windowWidth / 2 - surnameText.dstR.w / 2;
	surnameText.dstR.y = nameR.y + nameR.h + 15;
	SDL_FRect surnameR = nameR;
	surnameR.y = surnameText.dstR.y + surnameText.dstR.h + 6;
	Text surnameInputText;
	surnameInputText.setText(renderer, robotoF, "");
	surnameInputText.dstR = surnameR;
	surnameInputText.autoAdjustW = true;
	bool isNameSelected = true;
#endif
#if 1 // NOTE: MessageList
	std::vector<Message> messages;
	SDL_Texture* sendT = IMG_LoadTexture(renderer, "res/send.png");
	SDL_FRect sendBtnR;
	sendBtnR.w = 128;
	sendBtnR.h = 128;
	sendBtnR.x = 0;
	sendBtnR.y = windowHeight - sendBtnR.h;
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
	topicText.dstR.x = closeBtnR.x + closeBtnR.w;
	topicText.autoAdjustW = true;
	Text contentText; // TODO: Is sould go to next row when it gets out of right window border
	contentText.setText(renderer, robotoF, "");
	contentText.dstR = closeBtnR;
	contentText.dstR.y = closeBtnR.y + closeBtnR.h;
	contentText.autoAdjustW = true;
#endif
#if 1 // NOTE: MessageSend
	MsSelectedWidget msSelectedWidget = MsSelectedWidget::Name;
	SDL_FRect msNameR;
	msNameR.w = getValueFromValueAndPercent(windowWidth - closeBtnR.w, 100.f / 3.f);
	msNameR.h = closeBtnR.h;
	msNameR.x = closeBtnR.x + closeBtnR.w;
	msNameR.y = closeBtnR.y;
	Text msNameInputText;
	msNameInputText.setText(renderer, robotoF, "");
	msNameInputText.dstR = msNameR;
	msNameInputText.autoAdjustW = true;
	SDL_FRect msSurnameR = msNameR;
	msSurnameR.x = msNameR.x + msNameR.w;
	Text msSurnameInputText;
	msSurnameInputText.setText(renderer, robotoF, "");
	msSurnameInputText.dstR = msSurnameR;
	msSurnameInputText.autoAdjustW = true;
	SDL_FRect msTopicR = msSurnameR;
	msTopicR.x = msSurnameR.x + msSurnameR.w;
	Text msTopicInputText;
	msTopicInputText.setText(renderer, robotoF, "");
	msTopicInputText.dstR = msTopicR;
	msTopicInputText.autoAdjustW = true;
	SDL_FRect msContentR;
	msContentR.w = windowWidth;
	msContentR.h = windowHeight - closeBtnR.h;
	msContentR.x = 0;
	msContentR.y = closeBtnR.y + closeBtnR.h;
	Text msContentInputText;
	msContentInputText.setText(renderer, robotoF, "");
	msContentInputText.dstR = closeBtnR;
	msContentInputText.dstR.y = closeBtnR.y + closeBtnR.h;
	msContentInputText.autoAdjustW = true;
	SDL_FRect msSendBtnR;
	msSendBtnR.w = 200;
	msSendBtnR.h = 100;
	msSendBtnR.x = 0;
	msSendBtnR.y = windowHeight - msSendBtnR.h;
	// TODO: Content input text which will accept \t \n
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
						if (SDL_PointInFRect(&mousePos, &messages[i].r)) {
							state = State::MessageContent;
							messageIndexToShow = i;
							break;
						}
					}
					if (SDL_PointInFRect(&mousePos, &sendBtnR)) {
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
			}
			// TODO: Use more secure HTTP(s) -> curl
			// TODO: Do this from time to time + do so that it won't block UI
			messages.clear();
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
					std::string message;
					{
						int i = 0;
						while (std::getline(ss, message, '\036')) {
							std::stringstream ss(message);
							messages.push_back(Message());
							std::getline(ss, messages.back().topicText.text, '\037');
							std::getline(ss, messages.back().senderNameText.text, '\037');
							std::getline(ss, messages.back().senderSurnameText.text, '\037');
							std::getline(ss, messages.back().dateText.text, '\037');
							std::getline(ss, messages.back().contentText.text, '\037');
							messages.back().r.w = windowWidth;
							messages.back().r.h = 20;
							messages.back().r.x = 0;
							messages.back().r.y = messages.back().r.h * i;
							messages.back().topicText.dstR = messages.back().r;
							messages.back().topicText.autoAdjustW = true;
							messages.back().topicText.setText(renderer, robotoF, messages.back().topicText.text);
							messages.back().topicText.dstR.w *= 0.3;
							messages.back().senderNameText.dstR = messages.back().r;
							messages.back().senderNameText.dstR.x = messages.back().topicText.dstR.x + messages.back().topicText.dstR.w + 10;
							messages.back().senderNameText.autoAdjustW = true;
							messages.back().senderNameText.setText(renderer, robotoF, messages.back().senderNameText.text);
							messages.back().senderNameText.dstR.w *= 0.3;
							messages.back().senderSurnameText.dstR = messages.back().r;
							messages.back().senderSurnameText.dstR.x = messages.back().senderNameText.dstR.x + messages.back().senderNameText.dstR.w + 10;
							messages.back().senderSurnameText.autoAdjustW = true;
							messages.back().senderSurnameText.setText(renderer, robotoF, messages.back().senderSurnameText.text);
							messages.back().senderSurnameText.dstR.w *= 0.3;
							messages.back().dateText.dstR = messages.back().r;
							messages.back().dateText.dstR.x = messages.back().senderSurnameText.dstR.x + messages.back().senderSurnameText.dstR.w + 10;
							messages.back().dateText.autoAdjustW = true;
							messages.back().dateText.setText(renderer, robotoF, messages.back().dateText.text);
							messages.back().dateText.dstR.w *= 0.3;
							messages.back().contentText.dstR = messages.back().topicText.dstR;
							messages.back().contentText.dstR.x = closeBtnR.x + closeBtnR.w + 2;
							messages.back().contentText.autoAdjustW = true;
							messages.back().contentText.setText(renderer, robotoF, messages.back().contentText.text);
							messages.back().contentText.dstR.w *= 0.3;
							++i;
						}
					}
				}
			}
			// TODO: Handle errors? E.g. no internet connection.
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			SDL_RenderClear(renderer);
			// TODO: Add scroll when messages enter send button?
			for (int i = 0; i < messages.size(); ++i) {
				SDL_SetRenderDrawColor(renderer, 37, 37, 68, 0);
				SDL_RenderFillRectF(renderer, &messages[i].r);
				SDL_SetRenderDrawColor(renderer, 25, 25, 25, 0);
				SDL_RenderDrawRectF(renderer, &messages[i].r);
				messages[i].topicText.draw(renderer);
				messages[i].senderNameText.draw(renderer);
				messages[i].senderSurnameText.draw(renderer);
				messages[i].dateText.draw(renderer);
			}
			SDL_RenderCopyF(renderer, sendT, 0, &sendBtnR);
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
			}
			topicText.setText(renderer, robotoF, messages[messageIndexToShow].topicText.text);
			topicText.dstR.x = closeBtnR.x + closeBtnR.w + (windowWidth - closeBtnR.w) / 2 - topicText.dstR.w / 2;
			topicText.dstR.y = closeBtnR.h / 2 - topicText.dstR.h / 2;
			contentText.setText(renderer, robotoF, messages[messageIndexToShow].contentText.text);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			SDL_RenderClear(renderer);
			SDL_RenderCopyF(renderer, closeT, 0, &closeBtnR);
			topicText.draw(renderer);
			contentText.draw(renderer);
			SDL_RenderPresent(renderer);
		}
		else if (state == State::MessageSend) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				if (msSelectedWidget == MsSelectedWidget::Name) {
					inputHandleEvent(event, msNameInputText, renderer, robotoF);
				}
				else if (msSelectedWidget == MsSelectedWidget::Surname) {
					inputHandleEvent(event, msSurnameInputText, renderer, robotoF);
				}
				else if (msSelectedWidget == MsSelectedWidget::Topic) {
					inputHandleEvent(event, msTopicInputText, renderer, robotoF);
				}
				else if (msSelectedWidget == MsSelectedWidget::Content) {
					inputHandleEvent(event, msContentInputText, renderer, robotoF);
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
						msNameInputText.setText(renderer, robotoF, "");
						msSurnameInputText.setText(renderer, robotoF, "");
						msTopicInputText.setText(renderer, robotoF, "");
						msContentInputText.setText(renderer, robotoF, "");
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
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderFillRectF(renderer, &msNameR);
			SDL_RenderFillRectF(renderer, &msSurnameR);
			SDL_RenderFillRectF(renderer, &msTopicR);
			SDL_RenderFillRectF(renderer, &msContentR);
			SDL_RenderCopyF(renderer, closeT, 0, &closeBtnR);
			msNameInputText.draw(renderer);
			msSurnameInputText.draw(renderer);
			msTopicInputText.draw(renderer);
			msContentInputText.draw(renderer);
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