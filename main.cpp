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
#include <SFML/Audio.hpp>
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
#include <thread>
#include <atomic>
#ifdef __ANDROID__
#include <android/log.h> //__android_log_print(ANDROID_LOG_VERBOSE, "Sender", "Example number log: %d", number);
#include <jni.h>
#include "vendor/PUGIXML/src/pugixml.hpp"
#else
#include <pugixml.hpp>
#include <filesystem>
namespace fs = std::filesystem;
#endif
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std::chrono_literals;

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
std::string prefPath;

#ifndef RELEASE
#define SERVER
#endif
#define BG_COLOR 0,0,0,0
#define TEXT_COLOR 0,0,0,0
#define SERVER_PORT 4800 // TODO: Set some other port?

#undef SendMessage

enum PacketType {
	SendMessage,
	ReceiveMessages,
	MakeCall,
	CheckCalls,
	PendingCall,
	NoPendingCall,
	AcceptCall,
	IsCallAccepted,
	CallIsAccepted,
	CallIsNotAccepted,
	SendCallerAudioData,
	SendReceiverAudioData,
	GetCallerAudioData,
	GetReceiverAudioData,
};

void logOutputCallback(void* userdata, int category, SDL_LogPriority priority, const char* message)
{
	std::printf("%s\n", message);
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

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
std::string currentDateTime()
{
	time_t     now = std::time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *std::localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	std::strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

	return buf;
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
	Call,
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

void sendMessage(sf::TcpSocket& socket, Text& msNameInputText, Text& msSurnameInputText, Text& msTopicInputText, Text& msContentInputText, Text& nameInputText, Text& surnameInputText,
	MsSelectedWidget& msSelectedWidget, SDL_Renderer* renderer, TTF_Font* font)
{
	sf::Packet packet;
	packet
		<< PacketType::SendMessage
		<< msNameInputText.text
		<< msSurnameInputText.text
		<< msTopicInputText.text
		<< msContentInputText.text
		<< nameInputText.text
		<< surnameInputText.text;
	socket.send(packet); // TODO: Do something on fail + put it on separate thread?
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

struct Client {
	std::shared_ptr<sf::TcpSocket> socket;

	Client()
	{
		socket = std::make_shared<sf::TcpSocket>();
	}
};

pugi::xml_document loadDataDoc()
{
	pugi::xml_document srcDoc;
	srcDoc.load_file((prefPath + "data.xml").c_str());
	auto currMessageNodes = srcDoc.child("root").children("message");
	auto currCallNodes = srcDoc.child("root").children("call");
	pugi::xml_document dstDoc;
	pugi::xml_node rootNode = dstDoc.append_child("root");
	for (pugi::xml_node& currMessageNode : currMessageNodes) {
		pugi::xml_node messageNode = rootNode.append_child("message");
		messageNode.append_child("receiverName").append_child(pugi::node_pcdata).set_value(currMessageNode.child("receiverName").text().as_string());
		messageNode.append_child("receiverSurname").append_child(pugi::node_pcdata).set_value(currMessageNode.child("receiverSurname").text().as_string());
		messageNode.append_child("topic").append_child(pugi::node_pcdata).set_value(currMessageNode.child("topic").text().as_string());
		messageNode.append_child("content").append_child(pugi::node_pcdata).set_value(currMessageNode.child("content").text().as_string());
		messageNode.append_child("senderName").append_child(pugi::node_pcdata).set_value(currMessageNode.child("senderName").text().as_string());
		messageNode.append_child("senderSurname").append_child(pugi::node_pcdata).set_value(currMessageNode.child("senderSurname").text().as_string());
	}

	for (pugi::xml_node& currCallNode : currCallNodes) {
		pugi::xml_node callNode = rootNode.append_child("call");
		callNode.append_child("callerName").append_child(pugi::node_pcdata).set_value(currCallNode.child("callerName").text().as_string());
		callNode.append_child("callerSurname").append_child(pugi::node_pcdata).set_value(currCallNode.child("callerSurname").text().as_string());
		callNode.append_child("receiverName").append_child(pugi::node_pcdata).set_value(currCallNode.child("receiverName").text().as_string());
		callNode.append_child("receiverSurname").append_child(pugi::node_pcdata).set_value(currCallNode.child("receiverSurname").text().as_string());
		callNode.append_child("status").append_child(pugi::node_pcdata).set_value(currCallNode.child("status").text().as_string());

		auto currCallerAudioNodes = currCallNode.children("callerAudio");
		for (pugi::xml_node& currCallerAudioNode : currCallerAudioNodes) {
			pugi::xml_node callerAudioNode = callNode.append_child("callerAudio");
			callerAudioNode.append_child("id").append_child(pugi::node_pcdata).set_value(currCallerAudioNode.child("id").text().as_string());
			callerAudioNode.append_child("sampleRate").append_child(pugi::node_pcdata).set_value(currCallerAudioNode.child("sampleRate").text().as_string());
			callerAudioNode.append_child("channelCount").append_child(pugi::node_pcdata).set_value(currCallerAudioNode.child("channelCount").text().as_string());
			callerAudioNode.append_child("sampleCount").append_child(pugi::node_pcdata).set_value(currCallerAudioNode.child("sampleCount").text().as_string());
			callerAudioNode.append_child("samples").append_child(pugi::node_pcdata).set_value(currCallerAudioNode.child("samples").text().as_string());
		}

		auto currReceiverAudioNodes = currCallNode.children("receiverAudio");
		for (pugi::xml_node& currReceiverAudioNode : currReceiverAudioNodes) {
			pugi::xml_node receiverAudioNode = callNode.append_child("receiverAudio");
			receiverAudioNode.append_child("id").append_child(pugi::node_pcdata).set_value(currReceiverAudioNode.child("id").text().as_string());
			receiverAudioNode.append_child("sampleRate").append_child(pugi::node_pcdata).set_value(currReceiverAudioNode.child("sampleRate").text().as_string());
			receiverAudioNode.append_child("channelCount").append_child(pugi::node_pcdata).set_value(currReceiverAudioNode.child("channelCount").text().as_string());
			receiverAudioNode.append_child("sampleCount").append_child(pugi::node_pcdata).set_value(currReceiverAudioNode.child("sampleCount").text().as_string());
			receiverAudioNode.append_child("samples").append_child(pugi::node_pcdata).set_value(currReceiverAudioNode.child("samples").text().as_string());
		}
	}
	return dstDoc;
}

std::vector<pugi::xml_node> pugiXmlObjectRangeToStdVector(pugi::xml_object_range<pugi::xml_named_node_iterator> xmlObjectRange)
{
	std::vector<pugi::xml_node> vec;
	{
		for (pugi::xml_node& node : xmlObjectRange) {
			vec.push_back(node);
		}
	}
	return vec;
}

void runServer()
{
	sf::TcpListener listener;
	listener.listen(SERVER_PORT);
	std::vector<Client> clients;
	sf::SocketSelector selector;
	selector.add(listener);
	bool running = true;
	while (running) {
		if (selector.wait()) {
			if (selector.isReady(listener)) {
				clients.push_back(Client());
				if (listener.accept(*(clients.back().socket)) == sf::Socket::Done) {
					selector.add(*(clients.back().socket));
				}
				else {
					clients.pop_back();
				}
			}
			else {
				for (int i = 0; i < clients.size(); ++i) {
					if (selector.isReady(*(clients[i].socket))) {
						sf::Packet packet;
						if (clients[i].socket->receive(packet) == sf::Socket::Done) {
							PacketType packetType;
							{
								int tmp;
								packet >> tmp;
								packetType = (PacketType)tmp;
							}
							if (packetType == PacketType::SendMessage) {
								std::string msNameInputText;
								std::string msSurnameInputText;
								std::string msTopicInputText;
								std::string msContentInputText;
								std::string nameInputText;
								std::string surnameInputText;
								packet >> msNameInputText >> msSurnameInputText >> msTopicInputText >> msContentInputText >> nameInputText >> surnameInputText;
								pugi::xml_document doc = loadDataDoc();
								pugi::xml_node rootNode = doc.child("root");
								pugi::xml_node messageNode = rootNode.append_child("message");
								pugi::xml_node receiverNameNode = messageNode.append_child("receiverName");
								pugi::xml_node receiverSurnameNode = messageNode.append_child("receiverSurname");
								pugi::xml_node topicNode = messageNode.append_child("topic");
								pugi::xml_node contentNode = messageNode.append_child("content");
								pugi::xml_node senderNameNode = messageNode.append_child("senderName");
								pugi::xml_node senderSurnameNode = messageNode.append_child("senderSurname");
								receiverNameNode.append_child(pugi::node_pcdata).set_value(msNameInputText.c_str());
								receiverSurnameNode.append_child(pugi::node_pcdata).set_value(msSurnameInputText.c_str());
								topicNode.append_child(pugi::node_pcdata).set_value(msTopicInputText.c_str());
								contentNode.append_child(pugi::node_pcdata).set_value(msContentInputText.c_str());
								senderNameNode.append_child(pugi::node_pcdata).set_value(nameInputText.c_str());
								senderSurnameNode.append_child(pugi::node_pcdata).set_value(surnameInputText.c_str());
								doc.save_file((prefPath + "data.xml").c_str());
							}
							else if (packetType == PacketType::ReceiveMessages) {
								std::string name, surname;
								packet >> name >> surname;
								pugi::xml_document doc = loadDataDoc();
								auto messageNodes = doc.child("root").children("message");
								sf::Packet answerPacket;
								for (pugi::xml_node messageNode : messageNodes) {
									if (messageNode.child("receiverName").text().as_string() == name && messageNode.child("receiverSurname").text().as_string() == surname) {
										answerPacket
											<< messageNode.child("topic").text().as_string()
											<< messageNode.child("senderName").text().as_string()
											<< messageNode.child("senderSurname").text().as_string()
											<< messageNode.child("date").text().as_string()
											<< messageNode.child("content").text().as_string();
									}
								}
								clients[i].socket->send(answerPacket);
							}
							else if (packetType == PacketType::MakeCall) {
								std::string callerName, callerSurname, receiverName, receiverSurname;
								packet >> callerName >> callerSurname >> receiverName >> receiverSurname;
								pugi::xml_document doc = loadDataDoc();
								pugi::xml_node rootNode = doc.child("root");
								pugi::xml_node callNode = rootNode.append_child("call");
								pugi::xml_node callerNameNode = callNode.append_child("callerName");
								pugi::xml_node callerSurnameNode = callNode.append_child("callerSurname");
								pugi::xml_node receiverNameNode = callNode.append_child("receiverName");
								pugi::xml_node receiverSurnameNode = callNode.append_child("receiverSurname");
								pugi::xml_node statusNode = callNode.append_child("status");
								callerNameNode.append_child(pugi::node_pcdata).set_value(callerName.c_str());
								callerSurnameNode.append_child(pugi::node_pcdata).set_value(callerSurname.c_str());
								receiverNameNode.append_child(pugi::node_pcdata).set_value(receiverName.c_str());
								receiverSurnameNode.append_child(pugi::node_pcdata).set_value(receiverSurname.c_str());
								statusNode.append_child(pugi::node_pcdata).set_value("pending");
								doc.save_file((prefPath + "data.xml").c_str());
							}
							else if (packetType == PacketType::CheckCalls) {
								std::string receiverName, receiverSurname;
								packet >> receiverName >> receiverSurname;
								pugi::xml_document doc = loadDataDoc();
								pugi::xml_node rootNode = doc.child("root");
								auto callNodes = rootNode.children("call");
								sf::Packet answerPacket;
								bool found = false;
								for (pugi::xml_node& callNode : callNodes) {
									if (receiverName == callNode.child("receiverName").text().as_string() &&
										receiverSurname == callNode.child("receiverSurname").text().as_string()) {
										answerPacket << PacketType::PendingCall << callNode.child("callerName").text().as_string() << callNode.child("callerSurname").text().as_string();
										found = true;
										break;
									}
								}
								if (!found) {
									answerPacket << PacketType::NoPendingCall;
								}
								clients[i].socket->send(answerPacket);
								std::printf("Checked calls\n");
							}
							else if (packetType == PacketType::AcceptCall) {
								std::string receiverName, receiverSurname;
								packet >> receiverName >> receiverSurname;
								pugi::xml_document doc = loadDataDoc();
								pugi::xml_node rootNode = doc.child("root");
								auto callNodes = rootNode.children("call");
								for (pugi::xml_node& callNode : callNodes) {
									if (callNode.child("receiverName").text().as_string() == receiverName &&
										callNode.child("receiverSurname").text().as_string() == receiverSurname) {
										callNode.remove_child("status");
										pugi::xml_node statusNode = callNode.append_child("status");
										statusNode.append_child(pugi::node_pcdata).set_value("accepted");
										break;
									}
								}
								doc.save_file((prefPath + "data.xml").c_str());
							}
							else if (packetType == PacketType::IsCallAccepted) {
								std::string callerName, callerSurname;
								packet >> callerName >> callerSurname;
								pugi::xml_document doc = loadDataDoc();
								pugi::xml_node rootNode = doc.child("root");
								auto callNodes = rootNode.children("call");
								sf::Packet answerPacket;
								bool found = false;
								for (pugi::xml_node& callNode : callNodes) {
									if (callNode.child("callerName").text().as_string() == callerName &&
										callNode.child("callerSurname").text().as_string() == callerSurname) {
										found = true;
										std::string callerName = callNode.child("callerName").text().as_string();
										std::string callerSurname = callNode.child("callerSurname").text().as_string();
										callNode.print(std::cout);
										std::string status = callNode.child("status").text().as_string();
										if (status == "accepted") {
											answerPacket << PacketType::CallIsAccepted << callNode.child("receiverName").text().as_string() << callNode.child("receiverSurname").text().as_string();
										}
										else {
											answerPacket << PacketType::CallIsNotAccepted;
										}
										break;
									}
								}
								if (!found) {
									answerPacket << PacketType::CallIsNotAccepted;
								}
								clients[i].socket->send(answerPacket);
							}
							else if (packetType == PacketType::SendCallerAudioData) {
								std::string callerName, callerSurname;
								packet >> callerName >> callerSurname;
								pugi::xml_document doc = loadDataDoc();
								pugi::xml_node rootNode = doc.child("root");
								auto callNodes = rootNode.children("call");
								sf::Packet answerPacket;
								// TODO: Handle situations where there are more than one call to the same person and things like that (remember to do the same in below if statements)
								for (pugi::xml_node& callNode : callNodes) {
									if (callNode.child("callerName").text().as_string() == callerName &&
										callNode.child("callerSurname").text().as_string() == callerSurname) {
										// TODO: Think about infinite number of id's or reuse old ones (data type size)?
										int lastCallerAudioNodeId = 0;
										auto callerAudioNodes = callNode.children("callerAudio");
										for (pugi::xml_node& callerAudioNode : callerAudioNodes) {
											int id = callerAudioNode.child("id").text().as_int();
											if (id > lastCallerAudioNodeId) {
												lastCallerAudioNodeId = id;
											}
										}
										int id = lastCallerAudioNodeId + 1;

										pugi::xml_node callerAudioNode = callNode.append_child("callerAudio");

										unsigned sampleRate;
										unsigned channelCount;
										sf::Uint64 sampleCount;
										std::string samplesStr;
										packet >> sampleRate >> channelCount >> sampleCount >> samplesStr;

										callerAudioNode.append_child("id").append_child(pugi::node_pcdata).set_value(std::to_string(id).c_str());
										callerAudioNode.append_child("sampleRate").append_child(pugi::node_pcdata).set_value(std::to_string(sampleRate).c_str());
										callerAudioNode.append_child("channelCount").append_child(pugi::node_pcdata).set_value(std::to_string(channelCount).c_str());
										callerAudioNode.append_child("sampleCount").append_child(pugi::node_pcdata).set_value(std::to_string(sampleCount).c_str());
										callerAudioNode.append_child("samples").append_child(pugi::node_pcdata).set_value(samplesStr.c_str());
										// WARNING: Date time may depend on server Windows settings. It might be important when moving server to a different PC.
										doc.save_file((prefPath + "data.xml").c_str());
										break;
									}
								}
							}
							else if (packetType == PacketType::SendReceiverAudioData) {
								std::string receiverName, receiverSurname;
								packet >> receiverName >> receiverSurname;
								pugi::xml_document doc = loadDataDoc();
								pugi::xml_node rootNode = doc.child("root");
								auto callNodes = rootNode.children("call");
								sf::Packet answerPacket;
								for (pugi::xml_node& callNode : callNodes) {
									if (callNode.child("receiverName").text().as_string() == receiverName &&
										callNode.child("receiverSurname").text().as_string() == receiverSurname) {
										// TODO: Think about infinite number of id's or reuse old ones (data type size)?
										int lastReceiverAudioNodeId = 0;
										auto callerAudioNodes = callNode.children("receiverAudio");
										for (pugi::xml_node& callerAudioNode : callerAudioNodes) {
											int id = callerAudioNode.child("id").text().as_int();
											if (id > lastReceiverAudioNodeId) {
												lastReceiverAudioNodeId = id;
											}
										}
										int id = lastReceiverAudioNodeId + 1;

										pugi::xml_node receiverAudioNode = callNode.append_child("receiverAudio");

										unsigned sampleRate;
										unsigned channelCount;
										sf::Uint64 sampleCount;
										std::string samplesStr;
										packet >> sampleRate >> channelCount >> sampleCount >> samplesStr;

										receiverAudioNode.append_child("id").append_child(pugi::node_pcdata).set_value(std::to_string(id).c_str());
										receiverAudioNode.append_child("sampleRate").append_child(pugi::node_pcdata).set_value(std::to_string(sampleRate).c_str());
										receiverAudioNode.append_child("channelCount").append_child(pugi::node_pcdata).set_value(std::to_string(channelCount).c_str());
										receiverAudioNode.append_child("sampleCount").append_child(pugi::node_pcdata).set_value(std::to_string(sampleCount).c_str());
										receiverAudioNode.append_child("samples").append_child(pugi::node_pcdata).set_value(samplesStr.c_str());
										doc.save_file((prefPath + "data.xml").c_str());
										break;
									}
								}
							}
							else if (packetType == PacketType::GetCallerAudioData) {
								std::string callerName, callerSurname;
								packet >> callerName >> callerSurname;
								pugi::xml_document doc = loadDataDoc();
								// TODO: Is it possible that second user won't support channelCount? If yes what to do about it?
								pugi::xml_node rootNode = doc.child("root");
								auto callNodes = rootNode.children("call");
								sf::Packet answerPacket;
								for (pugi::xml_node& callNode : callNodes) {
									if (callNode.child("callerName").text().as_string() == callerName &&
										callNode.child("callerSurname").text().as_string() == callerSurname) {
										std::vector<pugi::xml_node> callerAudioNodes = pugiXmlObjectRangeToStdVector(callNode.children("callerAudio"));
										if (!callerAudioNodes.empty()) {
											pugi::xml_node callerAudioNodeWithMaxId = callerAudioNodes.front();
											for (int i = 1; i < callerAudioNodes.size(); ++i) {
												if (callerAudioNodes[i].child("id").text().as_int() > callerAudioNodeWithMaxId.child("id").text().as_int()) {
													callerAudioNodeWithMaxId = callerAudioNodes[i];
												}
											}

											unsigned sampleRate = callerAudioNodeWithMaxId.child("sampleRate").text().as_uint();
											unsigned channelCount = callerAudioNodeWithMaxId.child("channelCount").text().as_uint();
											sf::Uint64 sampleCount = callerAudioNodeWithMaxId.child("sampleCount").text().as_ullong();
											std::string samplesStr = callerAudioNodeWithMaxId.child("samples").text().as_string();
											answerPacket << sampleRate << channelCount << sampleCount << samplesStr;
											clients[i].socket->send(answerPacket);
										}
										else {
											unsigned sampleRate = 0;
											unsigned channelCount = 0;
											sf::Uint64 sampleCount = 0;
											std::string samplesStr;
											answerPacket << sampleRate << channelCount << sampleCount << samplesStr;
											clients[i].socket->send(answerPacket);
										}
									}
								}
							}
							else if (packetType == PacketType::GetReceiverAudioData) {
								std::string receiverName, receiverSurname;
								packet >> receiverName >> receiverSurname;
								pugi::xml_document doc = loadDataDoc();
								pugi::xml_node rootNode = doc.child("root");
								auto callNodes = rootNode.children("call");
								sf::Packet answerPacket;
								for (pugi::xml_node& callNode : callNodes) {
									if (callNode.child("receiverName").text().as_string() == receiverName &&
										callNode.child("receiverSurname").text().as_string() == receiverSurname) {
										std::vector<pugi::xml_node> receiverAudioNodes = pugiXmlObjectRangeToStdVector(callNode.children("receiverAudio"));
										if (!receiverAudioNodes.empty()) {
											pugi::xml_node receiverAudioNodeWithMaxId = receiverAudioNodes.front();
											for (int i = 1; i < receiverAudioNodes.size(); ++i) {
												if (receiverAudioNodes[i].child("id").text().as_int() > receiverAudioNodeWithMaxId.child("id").text().as_int()) {
													receiverAudioNodeWithMaxId = receiverAudioNodes[i];
												}
											}

											unsigned sampleRate = receiverAudioNodeWithMaxId.child("sampleRate").text().as_uint();
											unsigned channelCount = receiverAudioNodeWithMaxId.child("channelCount").text().as_uint();
											sf::Uint64 sampleCount = receiverAudioNodeWithMaxId.child("sampleCount").text().as_ullong();
											std::string samplesStr = receiverAudioNodeWithMaxId.child("samples").text().as_string();
											answerPacket << sampleRate << channelCount << sampleCount << samplesStr;
											clients[i].socket->send(answerPacket);
										}
										else {
											unsigned sampleRate = 0;
											unsigned channelCount = 0;
											sf::Uint64 sampleCount = 0;
											std::string samplesStr;
											answerPacket << sampleRate << channelCount << sampleCount << samplesStr;
											clients[i].socket->send(answerPacket);
										}
									}
								}
							}
						}
						else if (clients[i].socket->receive(packet) == sf::Socket::Disconnected) {
							selector.remove(*clients[i].socket);
							clients.erase(clients.begin() + i--);
						}
					}
				}
			}
		}
	}
}

enum CallState {
	None,
	CallerPending,
	ReceiverPending,
};

struct MessageList {
	std::vector<Message> messages;
	SDL_Texture* writeMessageT = 0;
	SDL_Texture* callT = 0;
	SDL_FRect writeMessageBtnR{};
	SDL_FRect callBtnR{};
	Text nameText;
	SDL_FRect nameR{};
	Text nameInputText;
	Text surnameText;
	SDL_FRect surnameR{};
	Text surnameInputText;
	bool isNameSelected = true;
	SDL_Texture* pickUpT = 0;
	SDL_FRect pickUpBtnR{};
	Text callerNameAndSurnameText;
	CallState callState = CallState::None;
};

int main(int argc, char* argv[])
{
	prefPath = SDL_GetPrefPath("Huberti", "Sender");
	std::srand(std::time(0));
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
	SDL_LogSetOutputFunction(logOutputCallback, 0);
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	SDL_GetMouseState(&mousePos.x, &mousePos.y);
#ifdef SERVER
	std::thread t([&] {
		runServer();
		});
	t.detach();
#endif
	sf::TcpSocket socket;
	socket.connect("192.168.1.10", SERVER_PORT); // TODO: Put it on seperate thread + do something with timeout variable(when it can't connect to remote server) + change Ip address to public one?
	std::string programName = fs::path(argv[0]).stem().string();
#if 1 && !RELEASE // TODO: Remember to turn it off on reelase
	SDL_Window* window = SDL_CreateWindow(programName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);
	windowWidth = dm.w;
	windowHeight = dm.h;
#else
	SDL_Window* window = SDL_CreateWindow(programName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP);
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
	MessageList ml;
	ml.writeMessageT = IMG_LoadTexture(renderer, "res/writeMessage.png");
	ml.writeMessageBtnR.w = 256;
	ml.writeMessageBtnR.h = 60;
	ml.writeMessageBtnR.x = 0;
	ml.writeMessageBtnR.y = windowHeight - ml.writeMessageBtnR.h;
	ml.nameR.w = 200;
	ml.nameR.h = 30;
	ml.nameR.x = ml.writeMessageBtnR.x + ml.writeMessageBtnR.w;
	ml.nameR.y = ml.writeMessageBtnR.y + ml.writeMessageBtnR.h / 2;
	ml.nameInputText.setText(renderer, robotoF, "");
	ml.nameInputText.dstR = ml.nameR;
	ml.nameInputText.dstR.x += 3;
	ml.nameInputText.autoAdjustW = true;
	ml.nameInputText.wMultiplier = 0.5;
	ml.nameText.setText(renderer, robotoF, u8"Imię");
	ml.nameText.dstR.w = 40;
	ml.nameText.dstR.h = 20;
	ml.nameText.dstR.x = ml.nameR.x + ml.nameR.w / 2 - ml.nameText.dstR.w / 2;
	ml.nameText.dstR.y = ml.writeMessageBtnR.y;
	ml.surnameR.w = 200;
	ml.surnameR.h = 30;
	ml.surnameR.x = ml.nameR.x + ml.nameR.w + 5;
	ml.surnameR.y = ml.writeMessageBtnR.y + ml.writeMessageBtnR.h / 2;
	ml.surnameInputText.setText(renderer, robotoF, "");
	ml.surnameInputText.dstR = ml.surnameR;
	ml.surnameInputText.dstR.x += 3;
	ml.surnameInputText.autoAdjustW = true;
	ml.surnameInputText.wMultiplier = 0.5;
	ml.surnameText.setText(renderer, robotoF, u8"Nazwisko");
	ml.surnameText.dstR.w = 80;
	ml.surnameText.dstR.h = 20;
	ml.surnameText.dstR.x = ml.surnameR.x + ml.surnameR.w / 2 - ml.surnameText.dstR.w / 2;
	ml.surnameText.dstR.y = ml.writeMessageBtnR.y;
	ml.callT = IMG_LoadTexture(renderer, "res/call.png");
	ml.callBtnR.w = 256;
	ml.callBtnR.h = 60;
	ml.callBtnR.x = ml.surnameR.x + ml.surnameR.w;
	ml.callBtnR.y = windowHeight - ml.callBtnR.h;
	ml.pickUpT = IMG_LoadTexture(renderer, "res/pickUp.png");
	ml.pickUpBtnR.w = 256;
	ml.pickUpBtnR.h = 60;
	ml.pickUpBtnR.x = ml.callBtnR.x + ml.callBtnR.w + 5;
	ml.pickUpBtnR.y = windowHeight - ml.pickUpBtnR.h;
	ml.callerNameAndSurnameText.autoAdjustW = true;
	ml.callerNameAndSurnameText.wMultiplier = 0.3;
	ml.callerNameAndSurnameText.dstR.h = 20;
	ml.callerNameAndSurnameText.dstR.x = ml.pickUpBtnR.x + ml.pickUpBtnR.w;
	ml.callerNameAndSurnameText.dstR.y = ml.pickUpBtnR.y + ml.pickUpBtnR.h / 2 - ml.callerNameAndSurnameText.dstR.h / 2;
#endif
#if 1 // NOTE: MessageContent
	SDL_Texture* closeT = IMG_LoadTexture(renderer, "res/close.png");
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
	SDL_Texture* sendT = IMG_LoadTexture(renderer, "res/send.png");
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
#if 1 // NOTE: Call
	std::string receiverName, receiverSurname;
	std::string callerName, callerSurname;
	bool isCaller = false;
	SDL_Rect r;
	r.w = 32;
	r.h = 32;
	r.x = 0;
	r.y = windowHeight / 2 - r.h / 2;
	float dx = 1;
	std::atomic<bool> shouldRunRecordingThread = true;
	std::atomic<bool> shouldRunPlayingThread = true;
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
					if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) {
						if (ml.isNameSelected) {
							if (!ml.nameInputText.text.empty()) {
								ml.nameInputText.text.pop_back();
								ml.nameInputText.setText(renderer, robotoF, ml.nameInputText.text, { TEXT_COLOR });
							}
						}
						else {
							if (!ml.surnameInputText.text.empty()) {
								ml.surnameInputText.text.pop_back();
								ml.surnameInputText.setText(renderer, robotoF, ml.surnameInputText.text, { TEXT_COLOR });
							}
						}
					}
					if (event.key.keysym.scancode == SDL_SCANCODE_TAB) {
						ml.isNameSelected = !ml.isNameSelected;
					}
				}
				if (event.type == SDL_KEYUP) {
					keys[event.key.keysym.scancode] = false;
				}
				if (event.type == SDL_MOUSEBUTTONDOWN) {
					buttons[event.button.button] = true;
					for (int i = 0; i < ml.messages.size(); ++i) {
						if (ml.messages[i].r.y + ml.messages[i].r.h < ml.writeMessageBtnR.y) {
							if (SDL_PointInFRect(&mousePos, &ml.messages[i].r)) {
								state = State::MessageContent;
								messageIndexToShow = i;
								texts.clear();
								{
									std::stringstream ss(ml.messages[messageIndexToShow].contentText.text);
									texts.push_back(Text());
									texts.back() = ml.messages[messageIndexToShow].contentText;
#if 1 // NOTE: Purpose - delete new line characters which shouldn't be displayed + split into new lines
									while (std::getline(ss, texts.back().text, '\n')) {
										texts.back().autoAdjustW = true;
										texts.back().wMultiplier = 0.3;
										texts.back().dstR.h = ml.writeMessageBtnR.h / 3;
										texts.back().dstR.x = 0;
										texts.back().dstR.y = closeBtnR.y + closeBtnR.h;
										texts.back().setText(renderer, robotoF, texts.back().text);
										texts.push_back(Text());
										texts.back() = ml.messages[messageIndexToShow].contentText;
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
					if (SDL_PointInFRect(&mousePos, &ml.writeMessageBtnR)) {
						state = State::MessageSend;
					}
					else if (SDL_PointInFRect(&mousePos, &ml.nameR)) {
						ml.isNameSelected = true;
					}
					else if (SDL_PointInFRect(&mousePos, &ml.surnameR)) {
						ml.isNameSelected = false;
					}
					else if (SDL_PointInFRect(&mousePos, &ml.callBtnR)) {
						sf::Packet packet;
						packet
							<< PacketType::MakeCall
							<< nameInputText.text
							<< surnameInputText.text
							<< ml.nameInputText.text
							<< ml.surnameInputText.text;
						ml.nameInputText.setText(renderer, robotoF, "", { TEXT_COLOR });
						ml.surnameInputText.setText(renderer, robotoF, "", { TEXT_COLOR });
						socket.send(packet); // TODO: Do something on fail + put it on separate thread?
						ml.callState = CallState::CallerPending;
						isCaller = true;
					}
					else if (ml.callState == CallState::ReceiverPending && SDL_PointInFRect(&mousePos, &ml.pickUpBtnR)) {
						sf::Packet packet;
						packet << PacketType::AcceptCall << nameInputText.text << surnameInputText.text;
						socket.send(packet); // TODO: Do something on fail + put it on separate thread?
						state = State::Call;
						isCaller = false;
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
						if (!ml.messages.empty()) {
							float minY = ml.messages.front().r.y;
							for (int i = 1; i < ml.messages.size(); ++i) {
								minY = std::min(minY, ml.messages[i].r.y);
							}
							if (minY < 0) {
								for (Message& message : ml.messages) {
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
						if (!ml.messages.empty()) {
							float maxY = ml.messages.front().r.y;
							for (int i = 1; i < ml.messages.size(); ++i) {
								maxY = std::max(maxY, ml.messages[i].r.y);
							}
							if (maxY >= windowHeight) {
								for (Message& message : ml.messages) {
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
				if (event.type == SDL_TEXTINPUT) {
					if (ml.isNameSelected) {
						ml.nameInputText.setText(renderer, robotoF, ml.nameInputText.text + event.text.text, { TEXT_COLOR });
					}
					else {
						ml.surnameInputText.setText(renderer, robotoF, ml.surnameInputText.text + event.text.text, { TEXT_COLOR });
					}
				}
			}
			if (isCaller) {
				sf::Packet packet;
				packet << PacketType::IsCallAccepted << nameInputText.text << surnameInputText.text;
				socket.send(packet); // TODO: Do something on fail + put it on separate thread?
				sf::Packet answerPacket;
				socket.receive(answerPacket); // TODO: Do something on fail + put it on separate thread?
				PacketType answerPacketType;
				{
					int tmp;
					answerPacket >> tmp;
					answerPacketType = (PacketType)(tmp);
					std::cout << tmp << std::endl;
				}
				if (answerPacketType == PacketType::CallIsAccepted) {
					answerPacket >> receiverName >> receiverSurname;
					state = State::Call;
				}
			}
			sf::Packet sentPacket;
			sentPacket << PacketType::ReceiveMessages << nameInputText.text << surnameInputText.text;
			socket.send(sentPacket);
			sf::Packet receivedPacket;
			socket.receive(receivedPacket); // TODO: Do something on fail + put it on separate thread?
			std::vector<Message> newMessages;
			newMessages.push_back(Message());
			{
				int i = 0;
				while (
					receivedPacket >> newMessages.back().topicText.text
					>> newMessages.back().senderNameText.text
					>> newMessages.back().senderSurnameText.text
					>> newMessages.back().dateText.text
					>> newMessages.back().contentText.text
					) {
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

					newMessages.push_back(Message());
					++i;
				}
			}
			newMessages.pop_back();
			if (ml.messages.size() != newMessages.size()) {
				ml.messages = newMessages;
			}
			else {
				for (int i = 0; i < ml.messages.size(); ++i) {
					if (ml.messages[i].topicText.text != newMessages[i].topicText.text ||
						ml.messages[i].senderNameText.text != newMessages[i].senderNameText.text ||
						ml.messages[i].senderSurnameText.text != newMessages[i].senderSurnameText.text ||
						ml.messages[i].dateText.text != newMessages[i].dateText.text ||
						ml.messages[i].contentText.text != newMessages[i].contentText.text) {
						ml.messages = newMessages;
						break;
					}
				}
			}
			if (ml.callState == CallState::None) {
				sf::Packet packet;
				packet
					<< PacketType::CheckCalls
					<< nameInputText.text
					<< surnameInputText.text;
				socket.send(packet); // TODO: Do something on fail + put it on separate thread?
				sf::Packet answerPacket;
				socket.receive(answerPacket); // TODO: Do something on fail + put it on separate thread?
				PacketType answerPacketType;
				{
					int tmp;
					answerPacket >> tmp;
					if (tmp == PacketType::PendingCall) {
						answerPacket >> callerName >> callerSurname;
						ml.callerNameAndSurnameText.setText(renderer, robotoF, callerName + " " + callerSurname, { 255,255,255 });
						ml.callState = CallState::ReceiverPending;
					}
				}
			}
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			SDL_RenderClear(renderer);
			for (int i = 0; i < ml.messages.size(); ++i) {
				if (ml.messages[i].r.y + ml.messages[i].r.h < ml.writeMessageBtnR.y) {
					SDL_SetRenderDrawColor(renderer, 37, 37, 68, 0);
					SDL_RenderFillRectF(renderer, &ml.messages[i].r);
					SDL_SetRenderDrawColor(renderer, 25, 25, 25, 0);
					SDL_RenderDrawRectF(renderer, &ml.messages[i].r);
					ml.messages[i].topicText.draw(renderer);
					ml.messages[i].senderNameText.draw(renderer);
					ml.messages[i].senderSurnameText.draw(renderer);
					ml.messages[i].dateText.draw(renderer);
				}
			}
			SDL_RenderCopyF(renderer, ml.writeMessageT, 0, &ml.writeMessageBtnR);
			SDL_RenderCopyF(renderer, ml.callT, 0, &ml.callBtnR);
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderFillRectF(renderer, &ml.nameR);
			SDL_RenderFillRectF(renderer, &ml.surnameR);
			SDL_SetRenderDrawColor(renderer, 52, 131, 235, 255);
			if (ml.isNameSelected) {
				SDL_RenderDrawRectF(renderer, &ml.nameR);
			}
			else {
				SDL_RenderDrawRectF(renderer, &ml.surnameR);
			}
			ml.nameText.draw(renderer);
			ml.surnameText.draw(renderer);
			drawInBorders(ml.nameInputText, ml.nameR, renderer, robotoF);
			drawInBorders(ml.surnameInputText, ml.surnameR, renderer, robotoF);
			if (ml.callState == CallState::ReceiverPending) {
				SDL_RenderCopyF(renderer, ml.pickUpT, 0, &ml.pickUpBtnR);
				ml.callerNameAndSurnameText.draw(renderer);
			}
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
			topicText.setText(renderer, robotoF, ml.messages[messageIndexToShow].topicText.text);
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
						sendMessage(socket, msNameInputText, msSurnameInputText, msTopicInputText, msContentInputText, nameInputText, surnameInputText, msSelectedWidget, renderer, robotoF);
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
		else if (state == State::Call) {
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

			// TODO: Take care about data types when doing Client-Server and Server-Client sends
			if (shouldRunRecordingThread) {
				shouldRunRecordingThread = false;
				std::thread t1([&] {
#if 0 // TODO: Use it ???
					if (!sf::SoundBufferRecorder::isAvailable()) {
					}
#endif
					sf::SoundBufferRecorder recorder;
					recorder.start();
					std::this_thread::sleep_for(3s);
					recorder.stop();
					const sf::SoundBuffer& buffer = recorder.getBuffer();
					sf::Packet p;
					if (isCaller) {
						p << PacketType::SendCallerAudioData;
					}
					else {
						p << PacketType::SendReceiverAudioData;
					}
					std::string samplesStr;
					for (sf::Uint64 i = 0; i < buffer.getSampleCount(); ++i) {
						samplesStr += std::to_string(buffer.getSamples()[i]) + " ";
					}
					p << nameInputText.text << surnameInputText.text << buffer.getSampleRate() << buffer.getChannelCount() << buffer.getSampleCount() << samplesStr;
					socket.send(p); // TODO: Do something on fail + put it on separate thread?
					shouldRunRecordingThread = true;
					});
				t1.detach();
			}
			if (shouldRunPlayingThread) {
				shouldRunPlayingThread = false;
				std::thread t2([&] {
					{
						sf::Packet p;
						if (isCaller) {
							p << PacketType::GetReceiverAudioData << receiverName << receiverSurname;
						}
						else {
							p << PacketType::GetCallerAudioData << callerName << callerSurname;
						}
						socket.send(p); // TODO: Do something on fail + put it on separate thread?
					}
					{
						sf::Packet p;
						socket.receive(p); // TODO: Do something on fail + put it on separate thread?
						unsigned sampleRate, channelCount;
						sf::Uint64 sampleCount;
						std::string samplesStr;
						p >> sampleRate >> channelCount >> sampleCount >> samplesStr;
						std::stringstream ss(samplesStr);
						std::string line;
						std::vector<sf::Int16> samples;
						while (std::getline(ss, line, ' ')) {
							samples.push_back(std::stoi(line));
						}
						sf::SoundBuffer buffer;
						if (!samples.empty()) {
							buffer.loadFromSamples(&samples[0], sampleCount, channelCount, sampleRate);
							sf::Sound sound(buffer);
							sound.play();
							while (sound.getStatus() == sf::Sound::Status::Playing) {
								;
							}
						}
					}
					shouldRunPlayingThread = true;
					});
				t2.detach();
			}

			r.x += dx;
			if (r.x + r.w > windowWidth || r.x < 0) {
				dx = -dx;
			}
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			SDL_RenderFillRect(renderer, &r);
			SDL_RenderPresent(renderer);
		}
	}
	// TODO: On mobile remember to use eventWatch function (it doesn't reach this code when terminating)
	return 0;
}