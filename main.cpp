﻿/*
TODO:
- Think what might happen if I'm reading/writing in binary mode files and sending them via at least 3 machines (user,server,user and user might login on a different PC and server db might be moved as well)
- Make download all button bigger in message content + rename it on the picture from "pobierz" to "pobierz wszystkie"
*/
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
#include <nfd.h>
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
#include <mutex>
#include <locale>
#include <codecvt>
#include <sys/types.h>
#include <sys/stat.h>
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
#include <Shlobj.h>
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
	DisconnectCall,
	DoesCallExits,
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

// Get current date/time, format is YYYY-MM-DD HH:mm:ss
std::string currentDateTime()
{
	time_t     now = std::time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *std::localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	std::strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

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

struct Attachment {
	std::string path;
	std::string fileContent;
	Text text;
};

struct Message {
	Text topicText;
	Text senderNameText;
	Text dateText;
	Text contentText;
	SDL_FRect r{};
	std::vector<Attachment> attachments;
};

float getValueFromValueAndPercent(float value, float percent)
{
	return value * percent / 100.f;
}

enum MsSelectedWidget {
	Name,
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

std::string readWholeFileInBinary(std::string path)
{
	std::stringstream ss;
	std::ifstream ifs(path, std::ifstream::in | std::ifstream::binary);
	ss << ifs.rdbuf();
	return ss.str();
}

void sendMessage(sf::TcpSocket& socket, Text& msNameInputText, Text& msTopicInputText, Text& msContentInputText, Text& nameInputText,
	MsSelectedWidget& msSelectedWidget, SDL_Renderer* renderer, TTF_Font* font, std::vector<Attachment>& attachments)
{
	sf::Packet packet;
	packet
		<< PacketType::SendMessage
		<< msNameInputText.text
		<< msTopicInputText.text
		<< msContentInputText.text
		<< nameInputText.text;
	for (int i = 0; i < attachments.size(); ++i) {
		packet
			<< attachments[i].path
			<< attachments[i].fileContent;
	}
	socket.send(packet); // TODO: Do something on fail + put it on separate thread?
	msNameInputText.setText(renderer, font, "");
	msTopicInputText.setText(renderer, font, "");
	msContentInputText.setText(renderer, font, "");
	msSelectedWidget = MsSelectedWidget::Name;
	attachments.clear();
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

std::string deleteFirstColon(std::string path)
{
	std::size_t index = path.find(":");
	if (index == std::string::npos) {
		return path;
	}
	else {
		return path.erase(index, 1);
	}
}

std::string deleteFilename(std::string path)
{
	std::size_t index = path.find_last_of("\\");
	if (index != std::string::npos) {
		return path.erase(index);
	}
	else {
		return "";
	}
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
						if (!fs::exists(prefPath + "data.xml")) // NOTE: If the data.xml doesn't exist, create one
						{
							pugi::xml_document doc;
							pugi::xml_node rootNode = doc.append_child("root");
							doc.save_file((prefPath + "data.xml").c_str());
						}
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
								std::string msTopicInputText;
								std::string msContentInputText;
								std::string nameInputText;
								packet >> msNameInputText >> msTopicInputText >> msContentInputText >> nameInputText;
								pugi::xml_document doc;
								doc.load_file((prefPath + "data.xml").c_str());
								pugi::xml_node rootNode = doc.child("root");
								int maxId = 0;
#if 1 // NOTE: Find max id
								{
									auto messageNodes = rootNode.children("message");
									for (auto messageNode : messageNodes) {
										int id = messageNode.child("id").text().as_int();
										if (id > maxId) {
											maxId = id;
										}
									}
								}
#endif
								pugi::xml_node messageNode = rootNode.append_child("message");
								pugi::xml_node idNode = messageNode.append_child("id");
								pugi::xml_node receiverNameNode = messageNode.append_child("receiverName");
								pugi::xml_node topicNode = messageNode.append_child("topic");
								pugi::xml_node contentNode = messageNode.append_child("content");
								pugi::xml_node senderNameNode = messageNode.append_child("senderName");
								pugi::xml_node dateTimeNode = messageNode.append_child("dateTime");

								idNode.append_child(pugi::node_pcdata).set_value(std::to_string(maxId + 1).c_str());
								receiverNameNode.append_child(pugi::node_pcdata).set_value(msNameInputText.c_str());
								topicNode.append_child(pugi::node_pcdata).set_value(msTopicInputText.c_str());
								contentNode.append_child(pugi::node_pcdata).set_value(msContentInputText.c_str());
								senderNameNode.append_child(pugi::node_pcdata).set_value(nameInputText.c_str());
								dateTimeNode.append_child(pugi::node_pcdata).set_value(currentDateTime().c_str());
								pugi::xml_node attachmentsNode = messageNode.append_child("attachments");
								{
									std::string path, fileContent;
									while (packet >> path >> fileContent) {
										std::string storePathWithoutFilename = prefPath + "Attachments\\" + idNode.text().as_string() + "\\" + deleteFilename(deleteFirstColon(path));
										std::string storePathWithFilename = prefPath + "Attachments\\" + idNode.text().as_string() + "\\" + deleteFirstColon(path);
										if (!fs::exists(storePathWithoutFilename)) {
											fs::create_directories(storePathWithoutFilename);
										}
										std::string oldStorePathWithFilename = storePathWithFilename;
										for (int i = 0; fs::exists(storePathWithFilename); ++i) {
											storePathWithFilename = oldStorePathWithFilename;
											storePathWithFilename += std::to_string(i);
										}
										std::ofstream ofs(storePathWithFilename, std::ofstream::out | std::ofstream::binary);
										ofs << fileContent;
										pugi::xml_node attachmentNode = attachmentsNode.append_child("attachment");
										attachmentNode.append_child("userPath").append_child(pugi::node_pcdata).set_value(path.c_str());
										attachmentNode.append_child("serverPath").append_child(pugi::node_pcdata).set_value(storePathWithFilename.c_str());
									}
								}
								doc.save_file((prefPath + "data.xml").c_str());
							}
							else if (packetType == PacketType::ReceiveMessages) {
								std::string name;
								packet >> name;
								pugi::xml_document doc;
								doc.load_file((prefPath + "data.xml").c_str());
								auto messageNodes = doc.child("root").children("message");
								sf::Packet answerPacket;
								for (pugi::xml_node& messageNode : messageNodes) {
									if (messageNode.child("receiverName").text().as_string() == name) {
										answerPacket
											<< messageNode.child("topic").text().as_string()
											<< messageNode.child("senderName").text().as_string()
											<< messageNode.child("content").text().as_string()
											<< messageNode.child("dateTime").text().as_string();
										auto attachmentNodes = messageNode.child("attachments").children("attachment");
										int attachmentNodesCount = 0;
										{
											for (pugi::xml_node& attachmentNode : attachmentNodes) {
												++attachmentNodesCount;
											}
										}
										answerPacket << attachmentNodesCount;
										for (pugi::xml_node& attachmentNode : attachmentNodes) {
											answerPacket
												<< attachmentNode.child("userPath").text().as_string()
												<< readWholeFileInBinary(attachmentNode.child("serverPath").text().as_string());
										}
									}
								}
								clients[i].socket->send(answerPacket);
							}
							else if (packetType == PacketType::MakeCall) {
								std::string callerName, receiverName;
								packet >> callerName >> receiverName;
								pugi::xml_document doc;
								doc.load_file((prefPath + "data.xml").c_str());
								pugi::xml_node rootNode = doc.child("root");
								pugi::xml_node callNode = rootNode.append_child("call");
								pugi::xml_node callerNameNode = callNode.append_child("callerName");
								pugi::xml_node receiverNameNode = callNode.append_child("receiverName");
								pugi::xml_node statusNode = callNode.append_child("status");
								callerNameNode.append_child(pugi::node_pcdata).set_value(callerName.c_str());
								receiverNameNode.append_child(pugi::node_pcdata).set_value(receiverName.c_str());
								statusNode.append_child(pugi::node_pcdata).set_value("pending");
								doc.save_file((prefPath + "data.xml").c_str());
							}
							else if (packetType == PacketType::CheckCalls) {
								std::string receiverName;
								packet >> receiverName;
								pugi::xml_document doc;
								doc.load_file((prefPath + "data.xml").c_str());
								pugi::xml_node rootNode = doc.child("root");
								auto callNodes = rootNode.children("call");
								sf::Packet answerPacket;
								bool found = false;
								for (pugi::xml_node& callNode : callNodes) {
									if (receiverName == callNode.child("receiverName").text().as_string()) {
										answerPacket << PacketType::PendingCall << callNode.child("callerName").text().as_string();
										found = true;
										break;
									}
								}
								if (!found) {
									answerPacket << PacketType::NoPendingCall;
								}
								clients[i].socket->send(answerPacket);
							}
							else if (packetType == PacketType::AcceptCall) {
								std::string receiverName;
								packet >> receiverName;
								pugi::xml_document doc;
								doc.load_file((prefPath + "data.xml").c_str());
								pugi::xml_node rootNode = doc.child("root");
								auto callNodes = rootNode.children("call");
								for (pugi::xml_node& callNode : callNodes) {
									if (callNode.child("receiverName").text().as_string() == receiverName) {
										callNode.remove_child("status");
										pugi::xml_node statusNode = callNode.append_child("status");
										statusNode.append_child(pugi::node_pcdata).set_value("accepted");
										break;
									}
								}
								doc.save_file((prefPath + "data.xml").c_str());
							}
							else if (packetType == PacketType::IsCallAccepted) {
								std::string callerName;
								packet >> callerName;
								pugi::xml_document doc;
								doc.load_file((prefPath + "data.xml").c_str());
								pugi::xml_node rootNode = doc.child("root");
								auto callNodes = rootNode.children("call");
								sf::Packet answerPacket;
								bool found = false;
								for (pugi::xml_node& callNode : callNodes) {
									if (callNode.child("callerName").text().as_string() == callerName) {
										found = true;
										std::string status = callNode.child("status").text().as_string();
										if (status == "accepted") {
											answerPacket << PacketType::CallIsAccepted << callNode.child("receiverName").text().as_string();
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
								std::string callerName;
								packet >> callerName;
								pugi::xml_document doc;
								doc.load_file((prefPath + "data.xml").c_str());
								pugi::xml_node rootNode = doc.child("root");
								auto callNodes = rootNode.children("call");
								sf::Packet answerPacket;
								// TODO: Handle situations where there are more than one call to the same person and things like that (remember to do the same in below if statements)
								for (pugi::xml_node& callNode : callNodes) {
									if (callNode.child("callerName").text().as_string() == callerName) {
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
								std::string receiverName;
								packet >> receiverName;
								pugi::xml_document doc;
								doc.load_file((prefPath + "data.xml").c_str());
								pugi::xml_node rootNode = doc.child("root");
								auto callNodes = rootNode.children("call");
								sf::Packet answerPacket;
								for (pugi::xml_node& callNode : callNodes) {
									if (callNode.child("receiverName").text().as_string() == receiverName) {
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
								std::string callerName;
								packet >> callerName;
								pugi::xml_document doc;
								doc.load_file((prefPath + "data.xml").c_str());
								// TODO: Is it possible that second user won't support channelCount? If yes what to do about it?
								pugi::xml_node rootNode = doc.child("root");
								auto callNodes = rootNode.children("call");
								sf::Packet answerPacket;
								bool found = false;
								for (pugi::xml_node& callNode : callNodes) {
									if (callNode.child("callerName").text().as_string() == callerName) {
										std::vector<pugi::xml_node> callerAudioNodes = pugiXmlObjectRangeToStdVector(callNode.children("callerAudio"));
										if (!callerAudioNodes.empty()) {
											found = true;
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
											break;
										}
									}
								}
								if (!found) {
									unsigned sampleRate = 0;
									unsigned channelCount = 0;
									sf::Uint64 sampleCount = 0;
									std::string samplesStr;
									answerPacket << sampleRate << channelCount << sampleCount << samplesStr;
									clients[i].socket->send(answerPacket);
								}
							}
							else if (packetType == PacketType::GetReceiverAudioData) {
								std::string receiverName;
								packet >> receiverName;
								pugi::xml_document doc;
								doc.load_file((prefPath + "data.xml").c_str());
								pugi::xml_node rootNode = doc.child("root");
								auto callNodes = rootNode.children("call");
								sf::Packet answerPacket;
								bool found = false;
								for (pugi::xml_node& callNode : callNodes) {
									if (callNode.child("receiverName").text().as_string() == receiverName) {
										std::vector<pugi::xml_node> receiverAudioNodes = pugiXmlObjectRangeToStdVector(callNode.children("receiverAudio"));
										if (!receiverAudioNodes.empty()) {
											found = true;
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
											break;
										}
									}
								}
								if (!found) {
									unsigned sampleRate = 0;
									unsigned channelCount = 0;
									sf::Uint64 sampleCount = 0;
									std::string samplesStr;
									answerPacket << sampleRate << channelCount << sampleCount << samplesStr;
									clients[i].socket->send(answerPacket);
								}
							}
							else if (packetType == PacketType::DisconnectCall) {
								std::string callerName;
								packet >> callerName;
								pugi::xml_document doc;
								doc.load_file((prefPath + "data.xml").c_str());
								pugi::xml_node rootNode = doc.child("root");
								auto callNodes = rootNode.children("call");
								for (pugi::xml_node& callNode : callNodes) {
									if (callNode.child("callerName").text().as_string() == callerName) {
										rootNode.remove_child(callNode);
										break;
									}
								}
								doc.save_file((prefPath + "data.xml").c_str());
							}
							else if (packetType == PacketType::DoesCallExits) {
								sf::Packet answerPacket;
								std::string callerName;
								packet >> callerName;
								pugi::xml_document doc;
								doc.load_file((prefPath + "data.xml").c_str());
								pugi::xml_node rootNode = doc.child("root");
								auto callNodes = rootNode.children("call");
								bool found = false;
								for (pugi::xml_node& callNode : callNodes) {
									if (callNode.child("callerName").text().as_string() == callerName) {
										found = true;
										break;
									}
								}
								answerPacket << found;
								clients[i].socket->send(answerPacket);
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
	bool isNameSelected = true;
	SDL_Texture* pickUpT = 0;
	SDL_FRect pickUpBtnR{};
	Text callerNameText;
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
	SDL_FRect nameR;
	nameR.w = 200;
	nameR.h = 30;
	nameR.x = windowWidth / 2 - nameR.w / 2;
	nameR.y = windowHeight / 2;
	Text nameText;
	nameText.setText(renderer, robotoF, u8"Nazwa");
	nameText.dstR.w = 60;
	nameText.dstR.h = 20;
	nameText.dstR.x = windowWidth / 2 - nameText.dstR.w / 2;
	nameText.dstR.y = nameR.y - nameText.dstR.h;
	Text nameInputText;
	nameInputText.setText(renderer, robotoF, "");
	nameInputText.dstR = nameR;
	nameInputText.dstR.x += 3;
	nameInputText.autoAdjustW = true;
	nameInputText.wMultiplier = 0.5;
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
	ml.nameText.setText(renderer, robotoF, u8"Nazwa");
	ml.nameText.dstR.w = 40;
	ml.nameText.dstR.h = 20;
	ml.nameText.dstR.x = ml.nameR.x + ml.nameR.w / 2 - ml.nameText.dstR.w / 2;
	ml.nameText.dstR.y = ml.writeMessageBtnR.y;
	ml.callT = IMG_LoadTexture(renderer, "res/call.png");
	ml.callBtnR.w = 256;
	ml.callBtnR.h = 60;
	ml.callBtnR.x = ml.nameR.x + ml.nameR.w;
	ml.callBtnR.y = windowHeight - ml.callBtnR.h;
	ml.pickUpT = IMG_LoadTexture(renderer, "res/pickUp.png");
	ml.pickUpBtnR.w = 256;
	ml.pickUpBtnR.h = 60;
	ml.pickUpBtnR.x = ml.callBtnR.x + ml.callBtnR.w + 5;
	ml.pickUpBtnR.y = windowHeight - ml.pickUpBtnR.h;
	ml.callerNameText.autoAdjustW = true;
	ml.callerNameText.wMultiplier = 0.3;
	ml.callerNameText.dstR.h = 20;
	ml.callerNameText.dstR.x = ml.pickUpBtnR.x + ml.pickUpBtnR.w;
	ml.callerNameText.dstR.y = ml.pickUpBtnR.y + ml.pickUpBtnR.h / 2 - ml.callerNameText.dstR.h / 2;
	std::string callerName;
	std::string receiverName;
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
	contentR.h = windowHeight - closeBtnR.h - 260;
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
	Text attachmentsText;
	attachmentsText.setText(renderer, robotoF, u8"Załączniki", { 255,255,255 });
	attachmentsText.dstR.w = 70;
	attachmentsText.dstR.h = 20;
	attachmentsText.dstR.x = windowWidth / 2 - attachmentsText.dstR.w / 2;
	attachmentsText.dstR.y = contentR.y + contentR.h;
	attachmentsText.autoAdjustW = true;
	SDL_FRect attachmentR;
	attachmentR.x = 0;
	attachmentR.y = attachmentsText.dstR.y + attachmentsText.dstR.h;
	attachmentR.w = windowWidth;
	attachmentR.h = windowHeight - attachmentR.y;
	SDL_Texture* downloadT = IMG_LoadTexture(renderer, "res/download.png");
	SDL_FRect downloadBtnR;
	downloadBtnR.w = 64;
	downloadBtnR.h = 20;
	downloadBtnR.x = 0;
	downloadBtnR.y = attachmentsText.dstR.y;
#endif
#if 1 // NOTE: MessageSend
	SDL_Texture* sendT = IMG_LoadTexture(renderer, "res/send.png");
	SDL_FRect msSendBtnR;
	msSendBtnR.w = 256;
	msSendBtnR.h = 64;
	msSendBtnR.x = 0;
	msSendBtnR.y = windowHeight - msSendBtnR.h;
	SDL_Texture* addAttachmentT = IMG_LoadTexture(renderer, "res/addAttachment.png");
	SDL_FRect msAddAttachmentBtnR;
	msAddAttachmentBtnR.w = 256;
	msAddAttachmentBtnR.h = 64;
	msAddAttachmentBtnR.x = msSendBtnR.x + msSendBtnR.w + 3;
	msAddAttachmentBtnR.y = msSendBtnR.y;
	MsSelectedWidget msSelectedWidget = MsSelectedWidget::Name;
	SDL_FRect msNameR;
	msNameR.w = getValueFromValueAndPercent(windowWidth - closeBtnR.w, 50);
	msNameR.h = getValueFromValueAndPercent(closeBtnR.h, 70);
	msNameR.x = closeBtnR.x + closeBtnR.w;
	msNameR.y = 0;
	Text msNameText;
	msNameText.setText(renderer, robotoF, u8"Nazwa", { TEXT_COLOR });
	msNameText.dstR.w = 60;
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
	SDL_FRect msTopicR = msNameR;
	msTopicR.x = msNameR.x + msNameR.w;
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
	Text msContentText;
	msContentText.setText(renderer, robotoF, u8"Treść", { TEXT_COLOR });
	msContentText.dstR.w = 60;
	msContentText.dstR.h = 20;
	msContentText.dstR.x = windowWidth / 2 - msContentText.dstR.w / 2;
	msContentText.dstR.y = closeBtnR.y + closeBtnR.h;
	SDL_FRect msContentR;
	msContentR.w = windowWidth;
	msContentR.h = windowHeight - closeBtnR.h - msSendBtnR.h - 260;
	msContentR.x = 0;
	msContentR.y = msContentText.dstR.y + msContentText.dstR.h;
	Text msContentInputText;
	msContentInputText.setText(renderer, robotoF, "", { TEXT_COLOR });
	msContentInputText.dstR = closeBtnR;
	msContentInputText.dstR.h = 20;
	msContentInputText.dstR.x += 3;
	msContentInputText.dstR.y = closeBtnR.y + closeBtnR.h;
	msContentInputText.autoAdjustW = true;
	msContentInputText.wMultiplier = 0.3;
	bool textInputEventInMsContentInputText = false;
	Text msAttachmentsText;
	msAttachmentsText.setText(renderer, robotoF, u8"Załączniki", { TEXT_COLOR });
	msAttachmentsText.dstR.w = 70;
	msAttachmentsText.dstR.h = 20;
	msAttachmentsText.dstR.x = windowWidth / 2 - msAttachmentsText.dstR.w / 2;
	msAttachmentsText.dstR.y = msContentR.y + msContentR.h;
	msAttachmentsText.autoAdjustW = true;
	std::vector<Attachment> msAttachments;
	SDL_FRect msAttachmentR;
	msAttachmentR.x = 0;
	msAttachmentR.y = msAttachmentsText.dstR.y + msAttachmentsText.dstR.h;
	msAttachmentR.w = windowWidth;
	msAttachmentR.h = msSendBtnR.y - msAttachmentR.y;
#endif
#if 1 // NOTE: Call
	bool isDisconnectedLocally = false;
	bool isDisconnectedRemotely = false;
	bool isCaller = false;
	SDL_Rect r;
	r.w = 32;
	r.h = 32;
	r.x = 0;
	r.y = windowHeight / 2 - r.h / 2;
	float dx = 1;
	std::atomic<bool> shouldRunRecordingThread = true;
	std::atomic<bool> shouldRunPlayingThread = true;
	std::mutex socketReceiveMutex;
	SDL_Texture* disconnectBtnT = IMG_LoadTexture(renderer, "res/disconnect.png");
	SDL_FRect disconnectBtnR;
	disconnectBtnR.w = 256;
	disconnectBtnR.h = 64;
	disconnectBtnR.x = windowWidth / 2 - disconnectBtnR.w / 2;
	disconnectBtnR.y = windowHeight - disconnectBtnR.h;
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
				}
			}
			SDL_SetRenderDrawColor(renderer, BG_COLOR);
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderFillRectF(renderer, &nameR);
			SDL_SetRenderDrawColor(renderer, 52, 131, 235, 255);
			if (isNameSelected) {
				SDL_RenderDrawRectF(renderer, &nameR);
			}
			nameText.draw(renderer);
			drawInBorders(nameInputText, nameR, renderer, robotoF);
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
#ifdef CALL
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
#endif
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
#ifdef CALL
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
						ml.isNameSelected = true;
						state = State::Call;
						isCaller = false;
						receiverName = nameInputText.text;
						receiverSurname = surnameInputText.text;
					}
#endif
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
									message.dateText.dstR.y -= windowHeight;
								}
							}
						}
					}
				}
				if (event.type == SDL_TEXTINPUT) {
#ifdef CALL
					if (ml.isNameSelected) {
						ml.nameInputText.setText(renderer, robotoF, ml.nameInputText.text + event.text.text, { TEXT_COLOR });
					}
					else {
						ml.surnameInputText.setText(renderer, robotoF, ml.surnameInputText.text + event.text.text, { TEXT_COLOR });
					}
#endif
				}
			}
#ifdef CALL
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
				}
				if (answerPacketType == PacketType::CallIsAccepted) {
					answerPacket >> receiverName >> receiverSurname;
					callerName = nameInputText.text;
					callerSurname = surnameInputText.text;
					ml.isNameSelected = true;
					state = State::Call;
				}
			}
#endif
			sf::Packet sentPacket;
			sentPacket << PacketType::ReceiveMessages << nameInputText.text;
			socket.send(sentPacket);
			sf::Packet receivedPacket;
			socket.receive(receivedPacket); // TODO: Do something on fail + put it on separate thread?
			std::vector<Message> newMessages;
			newMessages.push_back(Message());
			{
				int i = 0;
				while (
					receivedPacket
					>> newMessages.back().topicText.text
					>> newMessages.back().senderNameText.text
					>> newMessages.back().contentText.text
					>> newMessages.back().dateText.text
					) {
					int attachmentNodesCount = 0;
					receivedPacket >> attachmentNodesCount;
					for (int i = 0; i < attachmentNodesCount; ++i) {
						newMessages.back().attachments.push_back(Attachment());
						receivedPacket
							>> newMessages.back().attachments.back().path
							>> newMessages.back().attachments.back().fileContent;
						newMessages.back().attachments.back().text.autoAdjustW = true;
						newMessages.back().attachments.back().text.wMultiplier = 0.2;
						newMessages.back().attachments.back().text.setText(renderer, robotoF, fs::path(newMessages.back().attachments.back().path).filename().string(), { 255,255,255 });
						newMessages.back().attachments.back().text.dstR.h = 20;
						newMessages.back().attachments.back().text.dstR.x = 0;
						if (newMessages.back().attachments.size() == 1) {
							newMessages.back().attachments.back().text.dstR.y = attachmentsText.dstR.y + attachmentsText.dstR.h;
						}
						else {
							newMessages.back().attachments.back().text.dstR.y = newMessages.back().attachments[newMessages.back().attachments.size() - 2].text.dstR.y + newMessages.back().attachments[newMessages.back().attachments.size() - 2].text.dstR.h;
						}
					}

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

					newMessages.back().senderNameText.dstR = newMessages.back().r;
					newMessages.back().senderNameText.autoAdjustW = true;
					newMessages.back().senderNameText.setText(renderer, robotoF, newMessages.back().senderNameText.text);
					newMessages.back().senderNameText.dstR.w *= 0.3;
					newMessages.back().senderNameText.dstR.x = newMessages.back().dateText.dstR.x - newMessages.back().senderNameText.dstR.w - 10;

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
						ml.messages[i].dateText.text != newMessages[i].dateText.text ||
						ml.messages[i].contentText.text != newMessages[i].contentText.text) {
						ml.messages = newMessages;
						break;
					}
				}
			}
#ifdef CALL
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
#endif
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
					ml.messages[i].dateText.draw(renderer);
				}
			}
			SDL_RenderCopyF(renderer, ml.writeMessageT, 0, &ml.writeMessageBtnR);
#ifdef CALL
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
#endif
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
					for (int i = 0; i < ml.messages[messageIndexToShow].attachments.size(); ++i) {
						if (SDL_PointInFRect(&mousePos, &ml.messages[messageIndexToShow].attachments[i].text.dstR)) {
							// TODO: Don't allow to overwrite existing files
							std::wstring path;
							{
								wchar_t* p = 0;
								SHGetKnownFolderPath(FOLDERID_Downloads, 0, 0, &p);
								std::wstringstream ss;
								ss << p;
								path = ss.str();
								CoTaskMemFree(static_cast<void*>(p));
							}
							{
								std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
								// TODO: Are there any possible errors because of the fact that this file was send from one PC to another via TCP/IP using binary mode ???
								std::ofstream ofs(path + L"\\" + converter.from_bytes(fs::path(ml.messages[messageIndexToShow].attachments[i].path).filename().string()), std::ofstream::out | std::ofstream::binary);
								ofs << ml.messages[messageIndexToShow].attachments[i].fileContent;
							}
							ShellExecute(0, L"open", path.c_str(), 0, 0, SW_SHOWDEFAULT);
						}
					}
					if (SDL_PointInFRect(&mousePos, &downloadBtnR)) {
						// TODO: Don't allow to overwrite existing files
						std::wstring path;
						{
							wchar_t* p = 0;
							SHGetKnownFolderPath(FOLDERID_Downloads, 0, 0, &p);
							std::wstringstream ss;
							ss << p;
							path = ss.str();
							CoTaskMemFree(static_cast<void*>(p));
						}
						{
							std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
							for (int i = 0; i < ml.messages[messageIndexToShow].attachments.size(); ++i) {
								// TODO: Are there any possible errors because of the fact that this file was send from one PC to another via TCP/IP using binary mode ???
								std::ofstream ofs(path + L"\\" + converter.from_bytes(fs::path(ml.messages[messageIndexToShow].attachments[i].path).filename().string()), std::ofstream::out | std::ofstream::binary);
								ofs << ml.messages[messageIndexToShow].attachments[i].fileContent;
							}
						}
						ShellExecute(0, L"open", path.c_str(), 0, 0, SW_SHOWDEFAULT);
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
					if (SDL_PointInFRect(&mousePos, &contentR)) {
						if (event.wheel.y > 0) // scroll up
						{
							scroll = Scroll::Up;
						}
						else if (event.wheel.y < 0) // scroll down
						{
							scroll = Scroll::Down;
						}
					}
					else if (SDL_PointInFRect(&mousePos, &attachmentR)) {
						if (event.wheel.y > 0) // scroll up
						{
							if (!ml.messages[messageIndexToShow].attachments.empty()) {
								float minY = ml.messages[messageIndexToShow].attachments.front().text.dstR.y;
								for (int i = 1; i < ml.messages[messageIndexToShow].attachments.size(); ++i) {
									minY = std::min(minY, ml.messages[messageIndexToShow].attachments[i].text.dstR.y);
								}
								if (minY < attachmentR.y) {
									for (Attachment& attachment : ml.messages[messageIndexToShow].attachments) {
										attachment.text.dstR.y += attachment.text.dstR.h;
									}
								}
							}
						}
						else if (event.wheel.y < 0) // scroll down
						{
							if (!ml.messages[messageIndexToShow].attachments.empty()) {
								float maxY = ml.messages[messageIndexToShow].attachments.front().text.dstR.y;
								for (int i = 1; i < ml.messages[messageIndexToShow].attachments.size(); ++i) {
									maxY = std::max(maxY, ml.messages[messageIndexToShow].attachments[i].text.dstR.y);
								}
								if (maxY >= attachmentR.y + attachmentR.h) {
									for (Attachment& attachment : ml.messages[messageIndexToShow].attachments) {
										attachment.text.dstR.y -= attachment.text.dstR.h;
									}
								}
							}
						}
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
			attachmentsText.draw(renderer);
			for (int i = 0; i < ml.messages[messageIndexToShow].attachments.size(); ++i) {
				if (ml.messages[messageIndexToShow].attachments[i].text.dstR.y + ml.messages[messageIndexToShow].attachments[i].text.dstR.h <= attachmentR.y + attachmentR.h &&
					ml.messages[messageIndexToShow].attachments[i].text.dstR.y >= attachmentR.y) {
					ml.messages[messageIndexToShow].attachments[i].text.draw(renderer);
				}
			}
			SDL_RenderCopyF(renderer, downloadT, 0, &downloadBtnR);
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
						msAttachments.clear();
					}
					if (SDL_PointInFRect(&mousePos, &msSendBtnR)) {
						sendMessage(socket, msNameInputText, msTopicInputText, msContentInputText, nameInputText, msSelectedWidget, renderer, robotoF, msAttachments);
					}
					if (SDL_PointInFRect(&mousePos, &msAddAttachmentBtnR)) {
						nfdpathset_t outPaths;
						nfdresult_t result = NFD_OpenDialogMultiple(0, 0, &outPaths);
						if (result == NFD_OKAY) {
							for (int i = 0; i < NFD_PathSet_GetCount(&outPaths); ++i) {
								// TODO: Is it going to handle path from some exotic languages correctly??? - it requires u8 for Polish, what about others???
								msAttachments.push_back(Attachment());
								std::string path = NFD_PathSet_GetPath(&outPaths, i);
								msAttachments.back().fileContent = readWholeFileInBinary(path);
								msAttachments.back().path = path;
								msAttachments.back().text.autoAdjustW = true;
								msAttachments.back().text.wMultiplier = 0.2;
								msAttachments.back().text.setText(renderer, robotoF, fs::path(msAttachments.back().path).filename().string(), { TEXT_COLOR });
								msAttachments.back().text.dstR.h = 20;
								msAttachments.back().text.dstR.x = 0;
								if (msAttachments.size() == 1) {
									msAttachments.back().text.dstR.y = msAttachmentsText.dstR.y + msAttachmentsText.dstR.h;
								}
								else {
									msAttachments.back().text.dstR.y = msAttachments[msAttachments.size() - 2].text.dstR.y + msAttachments[msAttachments.size() - 2].text.dstR.h;
								}
							}
							NFD_PathSet_Free(&outPaths);
						}
					}
					else if (SDL_PointInFRect(&mousePos, &msNameR)) {
						msSelectedWidget = MsSelectedWidget::Name;
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
				if (event.type == SDL_MOUSEWHEEL) {
					if (event.wheel.y > 0) // scroll up
					{
						if (!msAttachments.empty()) {
							float minY = msAttachments.front().text.dstR.y;
							for (int i = 1; i < msAttachments.size(); ++i) {
								minY = std::min(minY, msAttachments[i].text.dstR.y);
							}
							if (minY < msAttachmentR.y) {
								for (Attachment& attachment : msAttachments) {
									attachment.text.dstR.y += attachment.text.dstR.h;
								}
							}
						}
					}
					else if (event.wheel.y < 0) // scroll down
					{
						if (!msAttachments.empty()) {
							float maxY = msAttachments.front().text.dstR.y;
							for (int i = 1; i < msAttachments.size(); ++i) {
								maxY = std::max(maxY, msAttachments[i].text.dstR.y);
							}
							if (maxY >= msAttachmentR.y + msAttachmentR.h) {
								for (Attachment& attachment : msAttachments) {
									attachment.text.dstR.y -= attachment.text.dstR.h;
								}
							}
						}
					}
				}
			}
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderFillRectF(renderer, &msNameR);
			SDL_RenderFillRectF(renderer, &msTopicR);
			SDL_RenderFillRectF(renderer, &msContentR);
			SDL_RenderCopyF(renderer, closeT, 0, &closeBtnR);
			msNameText.draw(renderer);
			msTopicText.draw(renderer);
			msContentText.draw(renderer);
			drawInBorders(msNameInputText, msNameR, renderer, robotoF);
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
			else if (msSelectedWidget == MsSelectedWidget::Topic) {
				SDL_RenderDrawRectF(renderer, &msTopicR);
			}
			else if (msSelectedWidget == MsSelectedWidget::Content) {
				SDL_RenderDrawRectF(renderer, &msContentR);
			}
			SDL_RenderCopyF(renderer, sendT, 0, &msSendBtnR);
			SDL_RenderCopyF(renderer, addAttachmentT, 0, &msAddAttachmentBtnR);
			msAttachmentsText.draw(renderer);
			for (int i = 0; i < msAttachments.size(); ++i) {
				if (msAttachments[i].text.dstR.y + msAttachments[i].text.dstR.h <= msAttachmentR.y + msAttachmentR.h &&
					msAttachments[i].text.dstR.y >= msAttachmentR.y) {
					msAttachments[i].text.draw(renderer);
				}
			}
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
					if (SDL_PointInFRect(&mousePos, &disconnectBtnR)) {
						isDisconnectedLocally = true;
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
			socketReceiveMutex.lock();
			{
				sf::Packet p;
				p << PacketType::DoesCallExits << callerName;
				socket.send(p); // TODO: Do something on fail + put it on separate thread?
			}
			{
				sf::Packet p;
				socket.receive(p); // TODO: Do something on fail + put it on separate thread?
				bool doesCallExists;
				p >> doesCallExists;
				if (!doesCallExists) {
					isDisconnectedRemotely = true;
				}
			}
			socketReceiveMutex.unlock();

			if (isDisconnectedRemotely || isDisconnectedLocally) {
#if 1 // NOTE: Wait for playingThread and recordingThread
				while (true) {
					if (shouldRunPlayingThread && shouldRunRecordingThread) {
						break;
					}
				}
#endif
				if (isDisconnectedLocally) {
					sf::Packet p;
					p << PacketType::DisconnectCall << callerName;
					socket.send(p); // TODO: Do something on fail + put it on separate thread?
				}
				state = State::MessageList;
				isDisconnectedLocally = false;
				isDisconnectedRemotely = false;
				ml.callState = CallState::None;
			}
			else {
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
						p << nameInputText.text << buffer.getSampleRate() << buffer.getChannelCount() << buffer.getSampleCount() << samplesStr;
						socket.send(p); // TODO: Do something on fail + put it on separate thread?
						shouldRunRecordingThread = true;
						});
					t1.detach();
				}
				if (shouldRunPlayingThread) {
					shouldRunPlayingThread = false;
					std::thread t2([&] {
						socketReceiveMutex.lock();
						{
							sf::Packet p;
							if (isCaller) {
								p << PacketType::GetReceiverAudioData << receiverName;
							}
							else {
								p << PacketType::GetCallerAudioData << callerName;
							}
							socket.send(p); // TODO: Do something on fail + put it on separate thread?
						}
						{
							sf::Packet p;
							socket.receive(p); // TODO: Do something on fail + put it on separate thread?
							socketReceiveMutex.unlock();
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
			}

			r.x += dx;
			if (r.x + r.w > windowWidth || r.x < 0) {
				dx = -dx;
			}
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			SDL_RenderFillRect(renderer, &r);
			SDL_RenderCopyF(renderer, disconnectBtnT, 0, &disconnectBtnR);
			SDL_RenderPresent(renderer);
		}
			}
	// TODO: On mobile remember to use eventWatch function (it doesn't reach this code when terminating)
	return 0;
			}
