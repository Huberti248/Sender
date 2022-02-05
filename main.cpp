/*
TODO:
- Try to test polish character path + polish filename + polish characters in file send as attachment from one PC to another + think about possible bugs (on server at least filename doesn't contain polish characters)
- When adding attachments with big size the program doesn't respond. Display some info about it?
- Think what might happen if .txt file will be send from one OS to another (binary mode file read and write)
- When std::wstring is send to the server, std::wstring should be received from client on the server
- Prevent: DoS and DDoS attacks, TCP SYN flood attack, Teardrop attack, Smurf attack, Ping of death attack?, Botnets?, Eavesdropping attack (just use encryption), Birthday attack (what's that)???
- Chekout getSize() and popBack() functions for exotic characters as well as remember to use them for every place where polish character might be placed
- Before final release remember to delete not used code from the server (in case of security)
- Some mechanism to automatically disconnect some clients which haven't responded for a very long time?
- Checkout have many clients could be connected to the server at the same time + handle situation when server is full
- Date time may depend on server Windows settings. It might be important when moving server to a different PC. Make it the same for all PC. Also think what date time should be dipslayed on the client (use this Windows settings?)
- It might be a good idea to implement encryption algorithms from stretch in order to understand them and their limitations (like how much time would it be necessary to break them e.g. brute force)
- Replace own inputs with imgui ones
- Think what will happen when user exceeds e.g. character limit (more characters than there could be in int data type)
*/
#ifdef __linux__
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_ttf.h>
#else
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_net.h>
#include <SDL_syswm.h>
#include <SDL_ttf.h>
#endif
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
//#include <SDL_gpu.h>
//#include <SFML/Audio.hpp>
//#include <SFML/Network.hpp>
//#include <SFML/Graphics.hpp>
#include <algorithm>
#include <assert.h>
#include <atomic>
#include <codecvt>
//#include <cryptlib.h>
#include <cstdlib>
#include <ctime>
//#include <dh.h>
//#include <dh2.h>
//#include <elgamal.h>
//#include <fhmqv.h>
//#include <filters.h>
#include <fstream>
#include <functional>
//#include <gcm.h>
//#include <hex.h>
//#include <hmqv.h>
#include <iostream>
#include <locale>
//#include <modes.h>
//#include <mqv.h>
#include <mutex>
//#include <nbtheory.h>
#include "vendor/NATIVEFILEDIALOGEXTENDED/src/include/nfd.h"
//#include <nfd.h>
//#include <osrng.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <vector>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>
#include <misc/cpp/imgui_stdlib.h>
#include "vendor/PUGIXML/src/pugixml.hpp"
#ifdef __ANDROID__
#include <android/log.h> //__android_log_print(ANDROID_LOG_VERBOSE, "Sender", "Example number log: %d", number);
#include <jni.h>
namespace fs = std::__fs::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif
#ifdef _WIN32
#include <Shlobj.h>
#include <shellapi.h>
#include <windows.h>
#endif

//using namespace std::chrono_literals;

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
std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
//std::string narrow = converter.to_bytes(wide_utf16_source_string);
//std::wstring wide = converter.from_bytes(narrow_utf8_source_string);

#ifndef RELEASE
#define SERVER
#endif
#define BG_COLOR 0, 0, 0, 0
#define TEXT_COLOR 0, 0, 0, 0
#define SERVER_PORT 5123
#define MAX_SOCKETS 65534

#define i8 int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

const int TAG_SIZE = 12;

#undef SendMessage

enum PacketType : i32 {
    TestMessage,
    Dh,
    Login,
    LoginSuccess,
    LoginFailure,
    SendMessage,
    SendMessageToClass,
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

std::wstring toWstring(std::string str)
{
    return converter.from_bytes(str);
}

std::string toStdString(std::wstring str)
{
    return converter.to_bytes(str);
}

void logOutputCallback(void* userdata, int category, SDL_LogPriority priority, const char* message)
{
    printf("%s\n", message);
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
    return ((p->x >= r->x) && (p->x < (r->x + r->w)) && (p->y >= r->y) && (p->y < (r->y + r->h))) ? SDL_TRUE : SDL_FALSE;
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

std::wstring s2ws(const std::string& str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
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

    void setText(SDL_Renderer* renderer, TTF_Font* font, std::string text, SDL_Color c = { 255, 255, 255 })
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

    void setText(SDL_Renderer* renderer, TTF_Font* font, int value, SDL_Color c = { 255, 255, 255 })
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
    time_t now = std::time(0);
    struct tm tstruct;
    char buf[80];
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
    SDL_FRect bgR{};
    SDL_Rect deleteBtnR{};
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

std::string ucs4ToUtf8(const std::u32string& in)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(in);
}

std::u32string utf8ToUcs4(const std::string& in)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.from_bytes(in);
}

std::string utf8Substr(const std::string& str, unsigned int start, unsigned int leng)
{
    std::u32string s = utf8ToUcs4(str);
    return ucs4ToUtf8(s.substr(start, leng));
}

void utf8Erase(std::string& buffer, int index)
{
    buffer = utf8Substr(buffer, 0, index) + utf8Substr(buffer, index + 1, std::string::npos);
}

void utf8PopBack(std::string& buffer)
{
    std::u32string str = utf8ToUcs4(buffer);
    str.pop_back();
    buffer = ucs4ToUtf8(str);
}

std::size_t utf8GetSize(std::string buffer)
{
    std::u32string str = utf8ToUcs4(buffer);
    return str.size();
}

void utf8Insert(std::string& buffer, int index, std::string text)
{
    std::u32string u32Buffer = utf8ToUcs4(buffer);
    if (index > u32Buffer.size()) {
        index = u32Buffer.size();
    }
    u32Buffer.insert(index, utf8ToUcs4(text));
    buffer = ucs4ToUtf8(u32Buffer);
}

void drawInBorders(Text& text, SDL_FRect r, SDL_Renderer* renderer, TTF_Font* font)
{
    std::string currentText = text.text;
    while (text.dstR.x + text.dstR.w > r.x + r.w) {
        if (text.text.empty()) {
            break;
        }
        else {
            utf8Erase(text.text, 0);
            text.setText(renderer, font, text.text, { TEXT_COLOR });
        }
    }
    text.draw(renderer);
    text.setText(renderer, font, currentText, { TEXT_COLOR });
}

struct Input {
    SDL_Rect r{};
    Text text;
    SDL_Rect cursorR{};
    int currentLetter = 0;
    std::u32string left;
    std::u32string right;
    bool isPassword = false;
    bool isSelected = false;

    void handleEvent(SDL_Event event, TTF_Font* font)
    {
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) {
                if (currentLetter) {
                    utf8Erase(text.text, currentLetter - 1);
                    if (!right.empty()) {
                        std::u32string s = utf8ToUcs4(text.text);
                        s.push_back(right.front());
                        right.erase(0, 1);
                        text.text = ucs4ToUtf8(s);
                    }
                    else if (!left.empty()) {
                        std::u32string s = utf8ToUcs4(text.text);
                        s.insert(0, 1, left.back());
                        left.pop_back();
                        text.text = ucs4ToUtf8(s);
                    }
                    else {
                        --currentLetter;
                    }
                    text.setText(renderer, font, text.text, {});
                }
            }
            else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
                if (currentLetter) {
                    --currentLetter;
                }
                else {
                    if (!left.empty()) {
                        std::u32string s = utf8ToUcs4(text.text);
                        s.insert(s.begin(), left.back());
                        left.pop_back();
                        right.insert(right.begin(), s.back());
                        s.pop_back();
                        text.setText(renderer, font, ucs4ToUtf8(s), {});
                    }
                }
            }
            else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
                if (currentLetter != utf8ToUcs4(text.text).size()) {
                    ++currentLetter;
                }
                else {
                    if (!right.empty()) {
                        std::u32string s = utf8ToUcs4(text.text);
                        s.push_back(right.front());
                        right.erase(0, 1);
                        left.push_back(s.front());
                        s.erase(0, 1);
                        text.setText(renderer, font, ucs4ToUtf8(s), {});
                    }
                }
            }
        }
        else if (event.type == SDL_TEXTINPUT) {
            utf8Insert(text.text, currentLetter, event.text.text);
            text.setText(renderer, font, text.text, {});
            ++currentLetter;
            while (text.dstR.x + text.dstR.w > r.x + r.w) {
                --currentLetter;
                left.push_back(utf8ToUcs4(text.text)[0]);
                utf8Erase(text.text, 0);
                text.setText(renderer, font, text.text, {});
            }
        }
    }

    void draw(SDL_Renderer* renderer, TTF_Font* font)
    {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
        SDL_RenderFillRect(renderer, &r);
        if (isPassword) {
            std::string currPassword = text.text;
            text.setText(renderer, font, std::string(utf8GetSize(currPassword), '*'), { TEXT_COLOR });
            text.draw(renderer);
            text.setText(renderer, font, currPassword, { TEXT_COLOR });
        }
        else {
            text.draw(renderer);
        }
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 0);
        Text t = text;
        t.text = utf8Substr(t.text, 0, currentLetter);
        t.setText(renderer, font, t.text, {});
        if (currentLetter) {
            cursorR.x = t.dstR.x + t.dstR.w;
        }
        else {
            cursorR.x = t.dstR.x;
        }
        if (isSelected) {
            SDL_RenderFillRect(renderer, &cursorR);
        }
    }
};

struct TextEditInput {
    SDL_Rect r{};
    SDL_Rect cursorR{};
    std::vector<Text> texts;
    int currentCursorLetterIndex = 0;
    int currentCursorTextIndex = 0;
    int scrollUpCount = 0;
    bool isSelected = false;

    std::string getText()
    {
        std::string text;
        for (int i = 0; i < texts.size(); ++i) {
            text += texts[i].text + "\n";
        }
        return text;
    }

    void clear(SDL_Renderer* renderer, TTF_Font* font)
    {
        if (texts.size() > 1) {
            texts.erase(texts.begin() + 1, texts.end());
        }
        if (!texts.empty()) {
            texts.front().setText(renderer, font, "", {});
        }
        currentCursorLetterIndex = 0;
        currentCursorTextIndex = 0;
        scrollUpCount = 0;
    }

    void putTypedTextIntoInput(SDL_Event event, SDL_Renderer* renderer, TTF_Font* font)
    {
        std::u32string str = utf8ToUcs4(texts[currentCursorTextIndex].text);
        str.insert(currentCursorLetterIndex, utf8ToUcs4(std::string(event.text.text)));
        texts[currentCursorTextIndex].setText(renderer, font, ucs4ToUtf8(str), {});
    }

    void moveCursorOneCharacterRight(SDL_Renderer* renderer, TTF_Font* font)
    {
        ++currentCursorLetterIndex;
        Text text = texts[currentCursorTextIndex];
        std::u32string str = utf8ToUcs4(texts[currentCursorTextIndex].text);
        text.setText(renderer, font, ucs4ToUtf8(str.substr(0, currentCursorLetterIndex)), {});
        cursorR.x = text.dstR.x + text.dstR.w;
    }

    int getMaxVisiableCount()
    {
        return r.h / texts.front().dstR.h;
    }

    void handleEvent(SDL_Event event, SDL_Renderer* renderer, TTF_Font* font)
    {
        if (event.type == SDL_TEXTINPUT) {
            // TODO: When the user have text excedeed down border and it enters it on the top it shouldn't always scroll
            putTypedTextIntoInput(event, renderer, font);
            moveCursorOneCharacterRight(renderer, font);
            int currentCursorTextIndexBackup = currentCursorTextIndex;
            bool isFirstIteration = true;
            while (texts[currentCursorTextIndex].dstR.x + texts[currentCursorTextIndex].dstR.w > r.x + r.w) {
                // NOTE: Put last character onto next line. If it will exceed second line repeat it until it won't.
                bool backupTextIndexChanged = false;
                if (isFirstIteration && currentCursorLetterIndex == texts[currentCursorTextIndex].text.size()) {
                    ++currentCursorTextIndexBackup;
                    currentCursorLetterIndex = 1;
                    backupTextIndexChanged = true;
                }
                if (currentCursorTextIndex + 1 == texts.size()) {
                    texts.insert(texts.begin() + currentCursorTextIndex + 1, 1, Text());
                    texts[currentCursorTextIndex + 1] = texts[currentCursorTextIndex];
                    texts[currentCursorTextIndex + 1].setText(renderer, font, "", {});
                    texts[currentCursorTextIndex + 1].dstR.y += texts[currentCursorTextIndex].dstR.h;
                }
                if (backupTextIndexChanged) {
                    cursorR.x = texts[currentCursorTextIndexBackup].dstR.x + texts[currentCursorTextIndexBackup].dstR.w;
                    cursorR.y = texts[currentCursorTextIndexBackup].dstR.y + texts[currentCursorTextIndexBackup].dstR.h / 2 - cursorR.h / 2;
                }
                std::u32string s1 = utf8ToUcs4(texts[currentCursorTextIndex].text);
                std::u32string s2 = utf8ToUcs4(texts[currentCursorTextIndex + 1].text);
                s2.insert(0, 1, s1.back());
                s1.pop_back();
                texts[currentCursorTextIndex].setText(renderer, font, ucs4ToUtf8(s1), {});
                texts[currentCursorTextIndex + 1].setText(renderer, font, ucs4ToUtf8(s2), {});
                ++currentCursorTextIndex;
                isFirstIteration = false;
            }

            currentCursorTextIndex = currentCursorTextIndexBackup;
            if (getMaxVisiableCount() - 1 + scrollUpCount == currentCursorTextIndex) // NOTE: If cursor is currently on the last visiable line
            {
                for (int i = 0; i < texts.size(); ++i) {
                    texts[i].dstR.y -= texts[i].dstR.h;
                    cursorR.y = texts[currentCursorTextIndex].dstR.y + texts[currentCursorTextIndex].dstR.h / 2 - cursorR.h / 2;
                }
                ++scrollUpCount;
            }
// TODO: It should scroll down when the user presses enter in the last line.
// NOTE: If it got out of down border move up texts and cursor
#if 0
            if (texts.back().dstR.y + texts.back().dstR.h > r.y + r.h) {
                for (int i = 0; i < texts.size(); ++i) {
                    texts[i].dstR.y -= texts[i].dstR.h;
                    cursorR.y = texts[currentCursorTextIndex].dstR.y + texts[currentCursorTextIndex].dstR.h / 2 - cursorR.h / 2;
                }
                ++scrollUpCount;
            }
#endif
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) {
                std::u32string str = utf8ToUcs4(texts[currentCursorTextIndex].text);
                if (!str.empty()) {
                    if (currentCursorLetterIndex) {
                        --currentCursorLetterIndex;
                        str.erase(currentCursorLetterIndex, 1);
                        texts[currentCursorTextIndex].setText(renderer, font, ucs4ToUtf8(str), {});
                        Text text = texts[currentCursorTextIndex];
                        text.setText(renderer, font, ucs4ToUtf8(str.substr(0, currentCursorLetterIndex)), {});
                        cursorR.x = text.dstR.x + text.dstR.w;
                        // TODO: It should be possible to remove "enter"
                        if (str.empty() && currentCursorLetterIndex == 0 && currentCursorTextIndex != 0) {
                            texts.erase(texts.begin() + currentCursorTextIndex);
                            --currentCursorTextIndex;
                            currentCursorLetterIndex = texts[currentCursorTextIndex].text.size();
                            cursorR.x = texts[currentCursorTextIndex].dstR.x + texts[currentCursorTextIndex].dstR.w;
                            cursorR.y = texts[currentCursorTextIndex].dstR.y + texts[currentCursorTextIndex].dstR.h / 2 - cursorR.h / 2;
                            if (scrollUpCount > 0) {
                                for (int i = 0; i < texts.size(); ++i) {
                                    texts[i].dstR.y += texts[i].dstR.h;
                                    cursorR.y = texts[currentCursorTextIndex].dstR.y + texts[currentCursorTextIndex].dstR.h / 2 - cursorR.h / 2;
                                }
                                --scrollUpCount;
                            }
                        }
                        else if (!str.empty() && currentCursorLetterIndex == 0 && currentCursorTextIndex != 0) {
                            --currentCursorTextIndex;
                            currentCursorLetterIndex = texts[currentCursorTextIndex].text.size();
                            cursorR.x = texts[currentCursorTextIndex].dstR.x + texts[currentCursorTextIndex].dstR.w;
                            cursorR.y = texts[currentCursorTextIndex].dstR.y + texts[currentCursorTextIndex].dstR.h / 2 - cursorR.h / 2;
                            if (scrollUpCount > 0) {
                                for (int i = 0; i < texts.size(); ++i) {
                                    texts[i].dstR.y += texts[i].dstR.h;
                                    cursorR.y = texts[currentCursorTextIndex].dstR.y + texts[currentCursorTextIndex].dstR.h / 2 - cursorR.h / 2;
                                }
                                --scrollUpCount;
                            }
                        }
                    }
                    else {
                        if (currentCursorTextIndex) {
                            --currentCursorTextIndex;
                            str = utf8ToUcs4(texts[currentCursorTextIndex].text);
                            if (!str.empty()) {
                                str.pop_back();
                            }
                            texts[currentCursorTextIndex].setText(renderer, font, ucs4ToUtf8(str), {});
                            currentCursorLetterIndex = texts[currentCursorTextIndex].text.size();
                            cursorR.x = texts[currentCursorTextIndex].dstR.x + texts[currentCursorTextIndex].dstR.w;
                            cursorR.y = texts[currentCursorTextIndex].dstR.y + texts[currentCursorTextIndex].dstR.h / 2 - cursorR.h / 2;
                        }
                    }
                }
#if 0
                std::string s;
                for (int i = 0; i < texts.size(); ++i) {
                    s += texts[i].text;
                }
                Text text = texts.front();
                texts.clear();
                do {
                    text.setText(renderer, font, s, {});
                    while (text.dstR.x + text.dstR.w > r.x + r.w) {
                        text.text.pop_back();
                        text.setText(renderer, font, text.text, {});
                    }
                    s = s.substr(text.text.size());
                    texts.push_back(text);
                    if (texts.size() > 1) {
                        texts.back().dstR.y += texts.back().dstR.h;
                    }
                    text = texts.back();
                } while (!s.empty());
#endif
            }
            else if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
                if (getMaxVisiableCount() - 1 + scrollUpCount == currentCursorTextIndex) // NOTE: If cursor is currently on the last visiable line
                {
                    for (int i = 0; i < texts.size(); ++i) {
                        texts[i].dstR.y -= texts[i].dstR.h;
                        cursorR.y = texts[currentCursorTextIndex].dstR.y + texts[currentCursorTextIndex].dstR.h / 2 - cursorR.h / 2;
                    }
                    ++scrollUpCount;
                }
                texts.insert(texts.begin() + currentCursorTextIndex + 1, 1, texts[currentCursorTextIndex]);
                texts[currentCursorTextIndex + 1].dstR.y += texts[currentCursorTextIndex + 1].dstR.h;
                // TODO: Is substr with currentCursorLetterIndex correct?
                std::string left = texts[currentCursorTextIndex].text.substr(0, currentCursorLetterIndex);
                std::string right = texts[currentCursorTextIndex].text.substr(currentCursorLetterIndex);
                texts[currentCursorTextIndex].setText(renderer, font, left, {});
                texts[currentCursorTextIndex + 1].setText(renderer, font, right, {});
                ++currentCursorTextIndex;
                currentCursorLetterIndex = 0;
                cursorR.x = texts[currentCursorTextIndex].dstR.x + texts[currentCursorTextIndex].dstR.w;
                cursorR.y = texts[currentCursorTextIndex].dstR.y + texts[currentCursorTextIndex].dstR.h / 2 - cursorR.h / 2;

#if 0
                texts.insert(texts.begin() + currentCursorTextIndex + 1, 1, texts[currentCursorTextIndex]);
                // TODO: Implement
                // TODO: Create new Text() and put everything after currentCursorLetterIndex on the next line
                texts.insert(texts.begin() + currentCursorTextIndex + 1, 1, texts[currentCursorTextIndex]);
                texts[currentCursorTextIndex + 1].dstR.y += texts[currentCursorTextIndex + 1].dstR.h;
                // TODO: Is substr range correct?
                std::string left = texts[currentCursorTextIndex].text.substr(0, currentCursorLetterIndex);
                std::string right = texts[currentCursorTextIndex].text.substr(currentCursorLetterIndex);
                texts[currentCursorTextIndex].setText(renderer, font, left, {});
                texts[currentCursorTextIndex + 1].setText(renderer, font, right, {});
#endif
            }
            else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
                // TODO: When moving at the beginning of text created by enter it should allow to move at the beginning of text
                --currentCursorLetterIndex;
                if (currentCursorLetterIndex == 0 || currentCursorLetterIndex == -1) {
                    if (currentCursorTextIndex) {
                        if (currentCursorTextIndex - scrollUpCount == 0) // NOTE: If currentCursorTextIndex is on first visible line
                        {
                            if (scrollUpCount > 0) {
                                for (int i = 0; i < texts.size(); ++i) {
                                    texts[i].dstR.y += texts[i].dstR.h;
                                    cursorR.y = texts[currentCursorTextIndex].dstR.y + texts[currentCursorTextIndex].dstR.h / 2 - cursorR.h / 2;
                                }
                                --scrollUpCount;
                            }
                        }
                        cursorR.y -= texts[currentCursorTextIndex].dstR.h;
                        --currentCursorTextIndex;
                        currentCursorLetterIndex = texts[currentCursorTextIndex].text.size();
                        Text text = texts[currentCursorTextIndex];
                        std::u32string str = utf8ToUcs4(text.text);
                        text.setText(renderer, font, ucs4ToUtf8(str.substr(0, currentCursorLetterIndex)), {});
                        cursorR.x = text.dstR.x + text.dstR.w;
                    }
                    else {
                        if (currentCursorLetterIndex == -1) {
                            ++currentCursorLetterIndex;
                        }
                        Text text = texts[currentCursorTextIndex];
                        std::u32string str = utf8ToUcs4(text.text);
                        text.setText(renderer, font, ucs4ToUtf8(str.substr(0, currentCursorLetterIndex)), {});
                        cursorR.x = text.dstR.x + text.dstR.w;
                    }
                }
                else {
                    Text text = texts[currentCursorTextIndex];
                    std::u32string str = utf8ToUcs4(text.text);
                    text.setText(renderer, font, ucs4ToUtf8(str.substr(0, currentCursorLetterIndex)), {});
                    cursorR.x = text.dstR.x + text.dstR.w;
                }
            }
            else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
                ++currentCursorLetterIndex;
                Text text = texts[currentCursorTextIndex];
                std::u32string str = utf8ToUcs4(text.text);
                text.setText(renderer, font, ucs4ToUtf8(str.substr(0, currentCursorLetterIndex)), {});
                cursorR.x = text.dstR.x + text.dstR.w;
                if (currentCursorLetterIndex > texts[currentCursorTextIndex].text.size()) {
                    if (currentCursorTextIndex != texts.size() - 1) {
                        if (getMaxVisiableCount() - 1 + scrollUpCount == currentCursorTextIndex) // NOTE: If cursor is currently on the last visiable line
                        {
                            if (currentCursorTextIndex != texts.size() - 1) {
                                for (int i = 0; i < texts.size(); ++i) {
                                    texts[i].dstR.y -= texts[i].dstR.h;
                                    cursorR.y = texts[currentCursorTextIndex].dstR.y + texts[currentCursorTextIndex].dstR.h / 2 - cursorR.h / 2;
                                }
                                ++scrollUpCount;
                            }
                        }
                        cursorR.y += texts[currentCursorTextIndex].dstR.h;
                        ++currentCursorTextIndex;
                        currentCursorLetterIndex = 0;
                        ++currentCursorLetterIndex;
                        Text text = texts[currentCursorTextIndex];
                        std::u32string str = utf8ToUcs4(text.text);
                        text.setText(renderer, font, ucs4ToUtf8(str.substr(0, currentCursorLetterIndex)), {});
                        cursorR.x = text.dstR.x + text.dstR.w;
                    }
                    else {
                        --currentCursorLetterIndex;
                    }
                }
            }
            else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
                if (getMaxVisiableCount() - 1 + scrollUpCount == currentCursorTextIndex) // NOTE: If cursor is currently on the last visiable line
                {
                    if (currentCursorTextIndex != texts.size() - 1) {
                        for (int i = 0; i < texts.size(); ++i) {
                            texts[i].dstR.y -= texts[i].dstR.h;
                            cursorR.y = texts[currentCursorTextIndex].dstR.y + texts[currentCursorTextIndex].dstR.h / 2 - cursorR.h / 2;
                        }
                        ++scrollUpCount;
                    }
                }
                if (currentCursorTextIndex != texts.size() - 1) {
                    ++currentCursorTextIndex;
                    if (currentCursorLetterIndex > texts[currentCursorTextIndex].text.size() - 1) {
                        currentCursorLetterIndex = texts[currentCursorTextIndex].text.size() - 1;
                        cursorR.x = texts[currentCursorTextIndex].dstR.x + texts[currentCursorTextIndex].dstR.w;
                    }
                    else {
                        std::u32string str = utf8ToUcs4(texts[currentCursorTextIndex].text);
                        Text text = texts[currentCursorTextIndex];
                        text.setText(renderer, font, ucs4ToUtf8(str.substr(0, currentCursorLetterIndex)), {});
                        cursorR.x = text.dstR.x + text.dstR.w;
                    }
                    cursorR.y = texts[currentCursorTextIndex].dstR.y + texts[currentCursorTextIndex].dstR.h / 2 - cursorR.h / 2;
                }
            }
            else if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
                if (currentCursorTextIndex - scrollUpCount == 0) // NOTE: If currentCursorTextIndex is on first visible line
                {
                    if (scrollUpCount > 0) {
                        for (int i = 0; i < texts.size(); ++i) {
                            texts[i].dstR.y += texts[i].dstR.h;
                            cursorR.y = texts[currentCursorTextIndex].dstR.y + texts[currentCursorTextIndex].dstR.h / 2 - cursorR.h / 2;
                        }
                        --scrollUpCount;
                    }
                }
                if (currentCursorTextIndex > scrollUpCount) {
                    --currentCursorTextIndex;
                    if (currentCursorLetterIndex > texts[currentCursorTextIndex].text.size() - 1) {
                        currentCursorLetterIndex = texts[currentCursorTextIndex].text.size() - 1;
                        cursorR.x = texts[currentCursorTextIndex].dstR.x + texts[currentCursorTextIndex].dstR.w;
                    }
                    else {
                        std::u32string str = utf8ToUcs4(texts[currentCursorTextIndex].text);
                        Text text = texts[currentCursorTextIndex];
                        text.setText(renderer, font, ucs4ToUtf8(str.substr(0, currentCursorLetterIndex)), {});
                        cursorR.x = text.dstR.x + text.dstR.w;
                    }
                    cursorR.y = texts[currentCursorTextIndex].dstR.y + texts[currentCursorTextIndex].dstR.h / 2 - cursorR.h / 2;
                }
            }
        }
    }

    void draw(SDL_Renderer* renderer)
    {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
        SDL_RenderFillRect(renderer, &r);
        for (int i = 0; i < texts.size(); ++i) {
            if (!(texts[i].dstR.y < r.y)) {
                texts[i].draw(renderer);
            }
        }
        if (isSelected) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 0);
            SDL_RenderFillRect(renderer, &cursorR);
        }
    }
};

std::string readWholeFileInBinary(std::string path)
{
    std::stringstream ss;
    std::ifstream ifs(path, std::ifstream::in | std::ifstream::binary);
    ss << ifs.rdbuf();
    return ss.str();
}

#define MAX_LENGTH 1024 * 100

int send(TCPsocket socket, std::string msg)
{
    return SDLNet_TCP_Send(socket, msg.c_str(), msg.size() + 1);
}

int receive(TCPsocket socket, std::string& msg)
{
    char message[MAX_LENGTH]{};
    int result = SDLNet_TCP_Recv(socket, message, MAX_LENGTH);
    msg = message;
    return result;
}

void sendMessage(TCPsocket socket, Text msNameText, std::string& msReceiver, std::string& msTopic, std::string& msMessage, SDL_Renderer* renderer, TTF_Font* font, std::vector<Attachment>& attachments)
{
    pugi::xml_document doc;
    pugi::xml_node rootNode = doc.append_child("root");
    if (msNameText.text == "Klasa") {
        rootNode.append_child("packetType").append_child(pugi::node_pcdata).set_value(std::to_string(PacketType::SendMessageToClass).c_str());
    }
    else {
        rootNode.append_child("packetType").append_child(pugi::node_pcdata).set_value(std::to_string(PacketType::SendMessage).c_str());
    }
    pugi::xml_node msNameInputNode = rootNode.append_child("msNameInput");
    msNameInputNode.append_child(pugi::node_pcdata).set_value(msReceiver.c_str());
    pugi::xml_node msTopicInputNode = rootNode.append_child("msTopicInput");
    msTopicInputNode.append_child(pugi::node_pcdata).set_value(msTopic.c_str());
    pugi::xml_node msContentInputNode = rootNode.append_child("msContentInput");
    msContentInputNode.append_child(pugi::node_pcdata).set_value(msMessage.c_str());
    for (int i = 0; i < attachments.size(); ++i) {
        pugi::xml_node attachmentNode = rootNode.append_child("attachment");
        pugi::xml_node pathNode = attachmentNode.append_child("path");
        pathNode.append_child(pugi::node_pcdata).set_value(attachments[i].path.c_str());
        pugi::xml_node fileContentNode = attachmentNode.append_child("fileContent");
        fileContentNode.append_child(pugi::node_pcdata).set_value(attachments[i].fileContent.c_str());
    }
    std::stringstream ss;
    doc.save(ss);
    send(socket, ss.str()); // TODO: Do something on fail + put it on separate thread?
    msReceiver.clear();
    msTopic.clear();
    msMessage.clear();
    attachments.clear();
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

struct Client {
    TCPsocket socket = 0;
    std::string name;
    std::string className;
    bool isLogged = false;
};

std::vector<Client> clients;

bool exists(std::string path)
{
#ifdef __ANDROID__
    std::ifstream ifs(path);
    return ifs.operator bool();
#else
    return fs::exists(path);
#endif
}

bool exists(std::wstring path)
{
#ifdef __ANDROID__
    std::ifstream ifs(ws2s(path));
    return ifs.operator bool();
#else
    return fs::exists(path);
#endif
}

void runServer()
{
    IPaddress ip;
    SDLNet_ResolveHost(&ip, 0, SERVER_PORT);
    TCPsocket serverSocket = SDLNet_TCP_Open(&ip);
    SDLNet_SocketSet socketSet = SDLNet_AllocSocketSet(MAX_SOCKETS + 1); // TODO: Do something when the server is full
    SDLNet_TCP_AddSocket(socketSet, serverSocket);
    bool running = true;
    while (running) {
        if (SDLNet_CheckSockets(socketSet, 0) > 0) {
            if (SDLNet_SocketReady(serverSocket)) {
                clients.push_back(Client());
                clients.back().socket = SDLNet_TCP_Accept(serverSocket);
                SDLNet_TCP_AddSocket(socketSet, clients.back().socket);
                printf("Client joined!\n");
            }
            for (int i = 0; i < clients.size(); ++i) {
                if (SDLNet_SocketReady(clients[i].socket)) {
                    if (!exists(prefPath + "data.xml")) // NOTE: If the data.xml doesn't exist, create one
                    {
                        pugi::xml_document doc;
                        pugi::xml_node rootNode = doc.append_child("root");
                        pugi::xml_node usersNode = rootNode.append_child("users");
                        pugi::xml_node messagesNode = rootNode.append_child("messages");
                        doc.save_file((prefPath + "data.xml").c_str());
                    }
                    std::string msg;
                    int received = receive(clients[i].socket, msg);
                    if (received > 0) {
                        pugi::xml_document doc;
                        doc.load_string(msg.c_str());
                        PacketType packetType = (PacketType)doc.child("root").child("packetType").text().as_int();
                        if (packetType == PacketType::Login) {
                            std::string name = doc.child("root").child("name").text().as_string(), password = doc.child("root").child("password").text().as_string();
                            pugi::xml_document doc;
                            doc.load_file((prefPath + "data.xml").c_str());
                            auto userNodes = doc.child("root").child("users").children("user");
                            bool found = false;
                            for (auto& userNode : userNodes) {
                                if (userNode.child("name").text().as_string() == name && userNode.child("password").text().as_string() == password) {
                                    found = true;
                                    clients[i].className = userNode.child("class").text().as_string();
                                    break;
                                }
                            }
                            {
                                pugi::xml_document doc;
                                pugi::xml_node rootNode = doc.append_child("root");
                                pugi::xml_node packetTypeNode = rootNode.append_child("packetType");
                                if (found) {
                                    // TODO: What will happen when someone will change it's IP address and sent some packet
                                    packetTypeNode.append_child(pugi::node_pcdata).set_value(std::to_string(PacketType::LoginSuccess).c_str());
                                    pugi::xml_node classNameNode = rootNode.append_child("className");
                                    classNameNode.append_child(pugi::node_pcdata).set_value(clients[i].className.c_str());
                                    clients[i].isLogged = true;
                                    clients[i].name = name;
                                }
                                else {
                                    packetTypeNode.append_child(pugi::node_pcdata).set_value(std::to_string(PacketType::LoginFailure).c_str());
                                }
                                std::stringstream ss;
                                doc.save(ss);
                                send(clients[i].socket, ss.str());
                            }
                        }
                        else {
                            if (clients[i].isLogged) {
                                if (packetType == PacketType::ReceiveMessages) {
                                    pugi::xml_document doc;
                                    doc.load_file((prefPath + "data.xml").c_str());
                                    auto messageNodes = doc.child("root").child("messages").children("message");
                                    pugi::xml_document d;
                                    pugi::xml_node rootNode = d.append_child("root");
                                    for (pugi::xml_node& messageNode : messageNodes) {
                                        if (messageNode.child("receiverName").text().as_string() == clients[i].name || messageNode.child("classReceiverName").text().as_string() == clients[i].className) {
                                            pugi::xml_node currMessageNode = rootNode.append_child("message");

                                            pugi::xml_node topicNode = currMessageNode.append_child("topic");
                                            topicNode.append_child(pugi::node_pcdata).set_value(messageNode.child("topic").text().as_string());
                                            pugi::xml_node senderNameNode = currMessageNode.append_child("senderName");
                                            senderNameNode.append_child(pugi::node_pcdata).set_value(messageNode.child("senderName").text().as_string());
                                            pugi::xml_node contentNode = currMessageNode.append_child("content");
                                            contentNode.append_child(pugi::node_pcdata).set_value(messageNode.child("content").text().as_string());
                                            pugi::xml_node dateTimeNode = currMessageNode.append_child("dateTime");
                                            dateTimeNode.append_child(pugi::node_pcdata).set_value(messageNode.child("dateTime").text().as_string());
                                            auto attachmentNodes = messageNode.child("attachments").children("attachment");
                                            int attachmentNodesCount = 0;
                                            {
                                                for (pugi::xml_node& attachmentNode : attachmentNodes) {
                                                    ++attachmentNodesCount;
                                                }
                                            }
                                            pugi::xml_node attachmentNodesCountNode = currMessageNode.append_child("attachmentNodesCount");
                                            attachmentNodesCountNode.append_child(pugi::node_pcdata).set_value(std::to_string(attachmentNodesCount).c_str());
                                            for (pugi::xml_node& attachmentNode : attachmentNodes) {
                                                pugi::xml_node attachmentNode2 = currMessageNode.append_child("attachment");
                                                pugi::xml_node userPathNode = attachmentNode2.append_child("path");
                                                userPathNode.append_child(pugi::node_pcdata).set_value(attachmentNode.child("userPath").text().as_string());
                                                pugi::xml_node fileNode = attachmentNode2.append_child("file");
                                                fileNode.append_child(pugi::node_pcdata).set_value(readWholeFileInBinary(attachmentNode.child("serverPath").text().as_string()).c_str());
                                            }
                                        }
                                    }
                                    std::stringstream ss;
                                    d.save(ss);
                                    send(clients[i].socket, ss.str());
                                }
                                else if (packetType == PacketType::SendMessage) {
                                    std::string msNameInputText = doc.child("root").child("msNameInput").text().as_string();
                                    std::string msTopicInputText = doc.child("root").child("msTopicInput").text().as_string();
                                    std::string msContentInputText = doc.child("root").child("msContentInput").text().as_string();
                                    pugi::xml_document d;
                                    d.load_file((prefPath + "data.xml").c_str());
                                    pugi::xml_node rootNode = d.child("root");
                                    int maxId = 0;
#if 1 // NOTE: Find max id
                                    {
                                        auto messageNodes = rootNode.child("messages").children("message");
                                        for (auto messageNode : messageNodes) {
                                            int id = messageNode.child("id").text().as_int();
                                            if (id > maxId) {
                                                maxId = id;
                                            }
                                        }
                                    }
#endif

                                    pugi::xml_node messageNode = rootNode.child("messages").append_child("message");
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
                                    senderNameNode.append_child(pugi::node_pcdata).set_value(clients[i].name.c_str());
                                    dateTimeNode.append_child(pugi::node_pcdata).set_value(currentDateTime().c_str());
                                    pugi::xml_node attachmentsNode = messageNode.append_child("attachments");
                                    {
                                        auto attachmentNodes = doc.child("root").children("attachment");
                                        std::string path, fileContent;
                                        for (auto it : attachmentNodes) {
                                            path = it.child("path").text().as_string();
                                            fileContent = it.child("fileContent").text().as_string();
                                            std::string storePathWithoutFilename = prefPath + "Attachments\\" + idNode.text().as_string() + "\\" + deleteFilename(deleteFirstColon(path));
                                            std::string storePathWithFilename = prefPath + "Attachments\\" + idNode.text().as_string() + "\\" + deleteFirstColon(path);
                                            if (!exists(storePathWithoutFilename)) {
                                                fs::create_directories(storePathWithoutFilename);
                                            }
                                            std::string oldStorePathWithFilename = storePathWithFilename;
                                            for (int i = 0; exists(storePathWithFilename); ++i) {
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
                                    d.save_file((prefPath + "data.xml").c_str());
                                }
                                else if (packetType == PacketType::SendMessageToClass) {
                                    std::string msNameInputText = doc.child("root").child("msNameInput").text().as_string();
                                    std::string msTopicInputText = doc.child("root").child("msTopicInput").text().as_string();
                                    std::string msContentInputText = doc.child("root").child("msContentInput").text().as_string();
                                    pugi::xml_document d;
                                    d.load_file((prefPath + "data.xml").c_str());
                                    pugi::xml_node rootNode = d.child("root");
                                    int maxId = 0;
#if 1 // NOTE: Find max id
                                    {
                                        auto messageNodes = rootNode.child("messages").children("message");
                                        for (auto messageNode : messageNodes) {
                                            int id = messageNode.child("id").text().as_int();
                                            if (id > maxId) {
                                                maxId = id;
                                            }
                                        }
                                    }
#endif
                                    pugi::xml_node messageNode = rootNode.child("messages").append_child("message");
                                    pugi::xml_node idNode = messageNode.append_child("id");
                                    pugi::xml_node classNameNode = messageNode.append_child("classReceiverName");
                                    pugi::xml_node topicNode = messageNode.append_child("topic");
                                    pugi::xml_node contentNode = messageNode.append_child("content");
                                    pugi::xml_node senderNameNode = messageNode.append_child("senderName");
                                    pugi::xml_node dateTimeNode = messageNode.append_child("dateTime");

                                    idNode.append_child(pugi::node_pcdata).set_value(std::to_string(maxId + 1).c_str());
                                    classNameNode.append_child(pugi::node_pcdata).set_value(msNameInputText.c_str());
                                    topicNode.append_child(pugi::node_pcdata).set_value(msTopicInputText.c_str());
                                    contentNode.append_child(pugi::node_pcdata).set_value(msContentInputText.c_str());
                                    senderNameNode.append_child(pugi::node_pcdata).set_value(clients[i].name.c_str());
                                    dateTimeNode.append_child(pugi::node_pcdata).set_value(currentDateTime().c_str());
                                    pugi::xml_node attachmentsNode = messageNode.append_child("attachments");
                                    {
                                        auto attachmentNodes = doc.child("root").children("attachment");
                                        for (auto it : attachmentNodes) {
                                            std::string path = it.child("path").text().as_string(), fileContent = it.child("fileContent").text().as_string();
                                            std::string storePathWithoutFilename = prefPath + "Attachments\\" + idNode.text().as_string() + "\\" + deleteFilename(deleteFirstColon(path));
                                            std::string storePathWithFilename = prefPath + "Attachments\\" + idNode.text().as_string() + "\\" + deleteFirstColon(path);
                                            if (!exists(storePathWithoutFilename)) {
                                                fs::create_directories(storePathWithoutFilename);
                                            }
                                            std::string oldStorePathWithFilename = storePathWithFilename;
                                            for (int i = 0; exists(storePathWithFilename); ++i) {
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
                                    d.save_file((prefPath + "data.xml").c_str());
                                }
                            }
                            else {
                                // TODO: Possibly cheat
                            }
                        }
                    }
                    else {
                        /*
						NOTE:
							An error may have occured, but sometimes you can just ignore it
							It may be good to disconnect sock because it is likely invalid now.
						*/
                    }
                }
            }
        }
    }
}

enum CallState {
    Non,
    CallerPending,
    ReceiverPending,
};

struct MessageList {
    std::vector<Message> messages;
    SDL_Texture* writeMessageT = 0;
    SDL_FRect writeMessageBtnR{};
    SDL_FRect callBtnR{};
    Text nameText;
    SDL_FRect nameR{};
    Text nameInputText;
    bool isNameSelected = true;
    SDL_FRect pickUpBtnR{};
    Text callerNameText;
    CallState callState = CallState::Non;
    Text userInfoText;
};

std::string getFilename(std::string s)
{
#ifdef __ANDROID__
    return s.substr(s.find_last_of("/\\") + 1);
#else
    return fs::path(s).filename().string();
#endif
}

std::string removeExtension(const std::string& filename)
{
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos)
        return filename;
    return filename.substr(0, lastdot);
}

enum class Scroll {
    Non,
    Up,
    Down,
};

void sendLoginAndPasswordToServer(std::string login, std::string password, TTF_Font* robotoF, TCPsocket socket, State& state, Text& infoText, MessageList& ml)
{

    /*
						TODO:
							(DONE)

							Someone might modify program behaviour and get into later states. Protect againts that.
							Note that event if server will keep current user state, someone might change his username in State::messageList and get someone else messages.

							One way to fix it might be to add user name to struct Client on server side + add bool isLoggedIn + check both before sending all user messages from db to client on
							PacketType::ReceiveMessages (then there is no need to send user name from user to the server anymore I guess - it might be got from clients[i].[...])
						*/

    // TODO: Encrypt the data on the client and decrypt it on the server
    // TODO: Add Crypto++ library to preamke + learn how to use AES
    // TODO: Make sure that it will be safe (no one would be able to steal login and password)
    // TODO: Prevent brute force - connection: (slowdown? timeout? capatche?)
    // TODO: Checkout possible security leaks in DH and possibly fix them (e.g man-in-the-middle)
    // TODO: Remember that other file types like images should be unreadable after the encryption
    {
        pugi::xml_document doc;
        pugi::xml_node rootNode = doc.append_child("root");
        pugi::xml_node packetTypeNode = rootNode.append_child("packetType");
        packetTypeNode.append_child(pugi::node_pcdata).set_value(std::to_string(PacketType::Login).c_str());
        pugi::xml_node nameNode = rootNode.append_child("name");
        nameNode.append_child(pugi::node_pcdata).set_value(login.c_str());
        pugi::xml_node passwordNode = rootNode.append_child("password");
        passwordNode.append_child(pugi::node_pcdata).set_value(password.c_str());
        std::stringstream ss;
        doc.save(ss);
        send(socket, ss.str()); // TODO: Do something on fail + put it on separate thread?
    }
    {
        std::string receiveMsg;
        receive(socket, receiveMsg);
        pugi::xml_document doc;
        doc.load_string(receiveMsg.c_str());
        PacketType packetType = (PacketType)doc.child("root").child("packetType").text().as_int();
        if (packetType == PacketType::LoginSuccess) {
            state = State::MessageList;
            std::string className = doc.child("root").child("className").text().as_string();
            ml.userInfoText.setText(renderer, robotoF, login + " " + className, { 255, 255, 255 });
        }
        else if (packetType == PacketType::LoginFailure) {
            infoText.setText(renderer, robotoF, u8"Nieprawidłowe dane logowania. Spróbuj ponownie.");
        }
    }
}

void textCentered(std::string text)
{
    auto windowWidth = ImGui::GetWindowSize().x;
    auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::Text(text.c_str());
}

int main(int argc, char* argv[])
{
    //TODO: How AES and DH works
#if 0
	try {
		// RFC 5114, 1024-bit MODP Group with 160-bit Prime Order Subgroup
		// http://tools.ietf.org/html/rfc5114#section-2.1
		CryptoPP::Integer p("0xB10B8F96A080E01DDE92DE5EAE5D54EC52C99FBCFB06A3C6"
			"9A6A9DCA52D23B616073E28675A23D189838EF1E2EE652C0"
			"13ECB4AEA906112324975C3CD49B83BFACCBDD7D90C4BD70"
			"98488E9C219A73724EFFD6FAE5644738FAA31A4FF55BCCC0"
			"A151AF5F0DC8B4BD45BF37DF365C1A65E68CFDA76D4DA708"
			"DF1FB2BC2E4A4371");

		CryptoPP::Integer g("0xA4D1CBD5C3FD34126765A442EFB99905F8104DD258AC507F"
			"D6406CFF14266D31266FEA1E5C41564B777E690F5504F213"
			"160217B4B01B886A5E91547F9E2749F4D7FBD7D3B9A92EE1"
			"909D0D2263F80A76A6A24C087A091F531DBF0A0169B6A28A"
			"D662A4D18E73AFA32D779D5918D08BC8858F4DCEF97C2A24"
			"855E6EEB22B3B2E5");

		CryptoPP::Integer q("0xF518AA8781A8DF278ABA4E7D64B7CB9D49462353");

		// Schnorr Group primes are of the form p = rq + 1, p and q prime. They
		// provide a subgroup order. In the case of 1024-bit MODP Group, the
		// security level is 80 bits (based on the 160-bit prime order subgroup).		

		// For a compare/contrast of using the maximum security level, see
		// dh-unified.zip. Also see http://www.cryptopp.com/wiki/Diffie-Hellman
		// and http://www.cryptopp.com/wiki/Security_level .

		CryptoPP::DH dh;
		CryptoPP::AutoSeededRandomPool rnd;

		dh.AccessGroupParameters().Initialize(p, q, g);

		if (!dh.GetGroupParameters().ValidateGroup(rnd, 3))
			throw std::runtime_error("Failed to validate prime and generator");

		size_t count = 0;

		p = dh.GetGroupParameters().GetModulus();
		q = dh.GetGroupParameters().GetSubgroupOrder();
		g = dh.GetGroupParameters().GetGenerator();

		// http://groups.google.com/group/sci.crypt/browse_thread/thread/7dc7eeb04a09f0ce
		CryptoPP::Integer v = ModularExponentiation(g, q, p);
		if (v != CryptoPP::Integer::One())
			throw std::runtime_error("Failed to verify order of the subgroup");

		//////////////////////////////////////////////////////////////

		CryptoPP::DH2 dhA(dh), dhB(dh);

		// NOTE: Empheral - classical key exchange

		CryptoPP::SecByteBlock sprivA(dhA.StaticPrivateKeyLength()), spubA(dhA.StaticPublicKeyLength());
		CryptoPP::SecByteBlock eprivA(dhA.EphemeralPrivateKeyLength()), epubA(dhA.EphemeralPublicKeyLength());

		CryptoPP::SecByteBlock sprivB(dhB.StaticPrivateKeyLength()), spubB(dhB.StaticPublicKeyLength());
		CryptoPP::SecByteBlock eprivB(dhB.EphemeralPrivateKeyLength()), epubB(dhB.EphemeralPublicKeyLength());

		dhA.GenerateStaticKeyPair(rnd, sprivA, spubA);
		dhA.GenerateEphemeralKeyPair(rnd, eprivA, epubA);

		dhB.GenerateStaticKeyPair(rnd, sprivB, spubB);
		dhB.GenerateEphemeralKeyPair(rnd, eprivB, epubB);

		//////////////////////////////////////////////////////////////

		if (dhA.AgreedValueLength() != dhB.AgreedValueLength())
			throw std::runtime_error("Shared secret size mismatch");

		CryptoPP::SecByteBlock sharedA(dhA.AgreedValueLength()), sharedB(dhB.AgreedValueLength());

		if (!dhA.Agree(sharedA, sprivA, eprivA, spubB, epubB))
			throw std::runtime_error("Failed to reach shared secret (A)");

		if (!dhB.Agree(sharedB, sprivB, eprivB, spubA, epubA))
			throw std::runtime_error("Failed to reach shared secret (B)");

		count = std::min(dhA.AgreedValueLength(), dhB.AgreedValueLength());
		if (!count || 0 != memcmp(sharedA.BytePtr(), sharedB.BytePtr(), count))
			throw std::runtime_error("Failed to reach shared secret");

		//////////////////////////////////////////////////////////////

		CryptoPP::Integer a, b;

		a.Decode(sharedA.BytePtr(), sharedA.SizeInBytes());
		std::cout << "Shared secret (A): " << std::hex << a << std::endl;

		b.Decode(sharedB.BytePtr(), sharedB.SizeInBytes());
		std::cout << "Shared secret (B): " << std::hex << b << std::endl;
	}
	catch (const CryptoPP::Exception& e) {
		std::cerr << e.what() << std::endl;
		return -2;
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}
	return 0;
#endif
#if 0
	{
		try {
			// RFC 5114, 1024-bit MODP Group with 160-bit Prime Order Subgroup
			// http://tools.ietf.org/html/rfc5114#section-2.1
			CryptoPP::Integer p("0xB10B8F96A080E01DDE92DE5EAE5D54EC52C99FBCFB06A3C6"
				"9A6A9DCA52D23B616073E28675A23D189838EF1E2EE652C0"
				"13ECB4AEA906112324975C3CD49B83BFACCBDD7D90C4BD70"
				"98488E9C219A73724EFFD6FAE5644738FAA31A4FF55BCCC0"
				"A151AF5F0DC8B4BD45BF37DF365C1A65E68CFDA76D4DA708"
				"DF1FB2BC2E4A4371");

			CryptoPP::Integer g("0xA4D1CBD5C3FD34126765A442EFB99905F8104DD258AC507F"
				"D6406CFF14266D31266FEA1E5C41564B777E690F5504F213"
				"160217B4B01B886A5E91547F9E2749F4D7FBD7D3B9A92EE1"
				"909D0D2263F80A76A6A24C087A091F531DBF0A0169B6A28A"
				"D662A4D18E73AFA32D779D5918D08BC8858F4DCEF97C2A24"
				"855E6EEB22B3B2E5");

			CryptoPP::Integer q("0xF518AA8781A8DF278ABA4E7D64B7CB9D49462353");

			// Schnorr Group primes are of the form p = rq + 1, p and q prime. They
			// provide a subgroup order. In the case of 1024-bit MODP Group, the
			// security level is 80 bits (based on the 160-bit prime order subgroup).		

			// For a compare/contrast of using the maximum security level, see
			// dh-unified.zip. Also see http://www.cryptopp.com/wiki/Diffie-Hellman
			// and http://www.cryptopp.com/wiki/Security_level .

			CryptoPP::DH dh;
			CryptoPP::AutoSeededRandomPool rnd;

			dh.AccessGroupParameters().Initialize(p, q, g);

			if (!dh.GetGroupParameters().ValidateGroup(rnd, 3))
				throw std::runtime_error("Failed to validate prime and generator");

			size_t count = 0;

			p = dh.GetGroupParameters().GetModulus();
			q = dh.GetGroupParameters().GetSubgroupOrder();
			g = dh.GetGroupParameters().GetGenerator();

			// http://groups.google.com/group/sci.crypt/browse_thread/thread/7dc7eeb04a09f0ce
			CryptoPP::Integer v = ModularExponentiation(g, q, p);
			if (v != CryptoPP::Integer::One())
				throw std::runtime_error("Failed to verify order of the subgroup");

			//////////////////////////////////////////////////////////////

			CryptoPP::DH2 dhA(dh), dhB(dh);

			// TODO: What is the usage of 8 below keys?

			// In the unified model, each party holds two key pairs : one key pair is used to perform classical(unautheticated) Diffie - Hellman, and the second pair is used for signing to ensure authenticity.

			// TODO: Put below keys on the client site
			CryptoPP::SecByteBlock sprivA(dhA.StaticPrivateKeyLength()), spubA(dhA.StaticPublicKeyLength()); // NOTE: This is used to perform classical(unautheticated) Diffie - Hellman
			CryptoPP::SecByteBlock eprivA(dhA.EphemeralPrivateKeyLength()), epubA(dhA.EphemeralPublicKeyLength()); // NOTE: This is used to sign to ensure authenticity

			// TODO: Put below keys on the server site
			CryptoPP::SecByteBlock sprivB(dhB.StaticPrivateKeyLength()), spubB(dhB.StaticPublicKeyLength());
			CryptoPP::SecByteBlock eprivB(dhB.EphemeralPrivateKeyLength()), epubB(dhB.EphemeralPublicKeyLength());

			/*
			TODO:
			Client should containt 2 key pair (one key pair is used to perform classical(unautheticated) Diffie - Hellman), (the second pair is used for signing to ensure authenticity).
			Server should containt 2 key pair
			*/

			dhA.GenerateStaticKeyPair(rnd, sprivA, spubA);
			dhA.GenerateEphemeralKeyPair(rnd, eprivA, epubA);

			dhB.GenerateStaticKeyPair(rnd, sprivB, spubB);
			dhB.GenerateEphemeralKeyPair(rnd, eprivB, epubB);

			//////////////////////////////////////////////////////////////

			std::cout << "Here:" << dhA.AgreedValueLength() << " " << dhB.AgreedValueLength() << std::endl;

			if (dhA.AgreedValueLength() != dhB.AgreedValueLength())  // NOTE: 256 == 256
				throw std::runtime_error("Shared secret size mismatch");

			CryptoPP::SecByteBlock sharedA(dhA.AgreedValueLength()), sharedB(dhB.AgreedValueLength());

			if (!dhA.Agree(sharedA, sprivA, eprivA, spubB, epubB))
				throw std::runtime_error("Failed to reach shared secret (A)");

			if (!dhB.Agree(sharedB, sprivB, eprivB, spubA, epubA))
				throw std::runtime_error("Failed to reach shared secret (B)");

			count = std::min(dhA.AgreedValueLength(), dhB.AgreedValueLength());
			if (!count || 0 != memcmp(sharedA.BytePtr(), sharedB.BytePtr(), count))
				throw std::runtime_error("Failed to reach shared secret");

			//////////////////////////////////////////////////////////////

			CryptoPP::Integer a, b;

			a.Decode(sharedA.BytePtr(), sharedA.SizeInBytes());
			std::cout << "Shared secret (A): " << std::hex << a << std::endl;

			b.Decode(sharedB.BytePtr(), sharedB.SizeInBytes());
			std::cout << "Shared secret (B): " << std::hex << b << std::endl;
		}

		catch (const CryptoPP::Exception& e) {
			std::cerr << e.what() << std::endl;
			return -2;
		}

		catch (const std::exception& e) {
			std::cerr << e.what() << std::endl;
			return -1;
		}

		//return 0;
	}
#endif
    prefPath = SDL_GetPrefPath("Huberti", "Sender"); // NOTE: On android /data/user/0/org.libsdl.app/files/
    std::srand(std::time(0));
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
    SDL_LogSetOutputFunction(logOutputCallback, 0);
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    SDLNet_Init();
#ifdef _WIN32
    NFD_Init();
#endif
    SDL_GetMouseState(&mousePos.x, &mousePos.y);
#ifdef SERVER
    std::thread t([&] {
        runServer();
    });
    t.detach();
#endif
    IPaddress ip;
    SDLNet_ResolveHost(&ip, "192.168.1.10", SERVER_PORT); // TODO: Change Ip address to public one (domain name)
    TCPsocket socket = SDLNet_TCP_Open(&ip); // TODO: Error handling
#if 0
	{
		// TODO: Make sure to provide four desirable features of secure communications(confidentiality, integrity, authentication, and non - repudiation).

		// TODO: DH doesn't protect against against man-in-the-middle. DH2 includes authentication which hardens the exchange protocol against many man-in-the-middle attacks. But does it eliminate the problem? The documentation says that it makes the algorithm authenticated (because orginally it's not)

		// TODO: Use DH2 ??? And if that's true, would it be secure?

			// TODO: Is public/private key generation secure?

		// TODO: Why only 32b key is said to be max allowed? Use keys with more b?

			// NOTE: Conversely, most block cipher modes of operation require an IV which is randomand unpredictable, or at least unique for each message encrypted with a given key.

				//Note: if your project is using encryption alone to secure your data, encryption alone is usually not enough.Please take a moment to read Authenticated Encryptionand consider using an algorithm or mode like CCM, GCM, EAX or ChaCha20Poly1305.


			// TODO: Remember to make all data authenticated (if you choose to generate a random IV and append it to the message, be sure to authenticate the {IV,Ciphertext} pair.)


		CryptoPP::AutoSeededRandomPool prng;
		CryptoPP::Integer p, q, g;
		CryptoPP::PrimeAndGenerator pg;

		pg.Generate(1, prng, 512, 511);
		p = pg.Prime();
		q = pg.SubPrime();
		g = pg.Generator();

		CryptoPP::DH dh(p, q, g);
		CryptoPP::SecByteBlock t1(dh.PrivateKeyLength()), t2(dh.PublicKeyLength());
		dh.GenerateKeyPair(prng, t1, t2);
		CryptoPP::Integer privateKey(t1, t1.size()), publicKey(t2, t2.size());

		sf::Packet packet;
		packet << PacketType::Dh << toStdString(publicKey);
		socket.send(packet); // TODO: Do something on fail + put it on separate thread?


		sf::Packet answerPacket;
		socket.receive(answerPacket); // TODO: Do something on fail + put it on separate thread?
		byte key[CryptoPP::AES::DEFAULT_KEYLENGTH];
		for (int i = 0; i < CryptoPP::AES::DEFAULT_KEYLENGTH; ++i) {
			answerPacket >> key[i];
		}

		// TODO: Below code: It does not perform authentication over additional authenticated data (aad). - Is it necessary ???


// The test vectors use both ADATA and PDATA. However,
//  as a drop in replacement for older modes such as
//  CBC, we only exercise (and need) plain text.


		byte iv[CryptoPP::AES::BLOCKSIZE];
		prng.GenerateBlock(iv, sizeof(iv));

		const int TAG_SIZE = 12;

		// Plain text
		std::string pdata = "Authenticated Encryption";

		// Encrypted, with Tag
		std::string cipher, encoded;

		// Recovered plain text
		std::string rpdata;

		/*********************************\
		\*********************************/

		// Pretty print
		encoded.clear();
		CryptoPP::StringSource(key, sizeof(key), true,
			new CryptoPP::HexEncoder(
				new CryptoPP::StringSink(encoded)
			) // HexEncoder
		); // StringSource
		std::cout << "key: " << encoded << std::endl;

		// Pretty print
		encoded.clear();
		CryptoPP::StringSource(iv, sizeof(iv), true,
			new CryptoPP::HexEncoder(
				new CryptoPP::StringSink(encoded)
			) // HexEncoder
		); // StringSource
		std::cout << " iv: " << encoded << std::endl;

		std::cout << std::endl;

		/*********************************\
		\*********************************/



		try {
			std::cout << "plain text: " << pdata << std::endl;

			CryptoPP::GCM< CryptoPP::AES >::Encryption e;
			e.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
			// e.SpecifyDataLengths( 0, pdata.size(), 0 );

			CryptoPP::StringSource(pdata, true,
				new CryptoPP::AuthenticatedEncryptionFilter(e,
					new CryptoPP::StringSink(cipher), false, TAG_SIZE
				) // AuthenticatedEncryptionFilter
			); // StringSource




		}
		catch (CryptoPP::InvalidArgument& e) {
			std::cerr << "Caught InvalidArgument..." << std::endl;
			std::cerr << e.what() << std::endl;
			std::cerr << std::endl;
		}
		catch (CryptoPP::Exception& e) {
			std::cerr << "Caught Exception..." << std::endl;
			std::cerr << e.what() << std::endl;
			std::cerr << std::endl;
		}

		/*********************************\
		\*********************************/

		// Pretty print
		encoded.clear();
		CryptoPP::StringSource(cipher, true,
			new CryptoPP::HexEncoder(
				new CryptoPP::StringSink(encoded)
			) // HexEncoder
		); // StringSource
		std::cout << "cipher text: " << encoded << std::endl;

		// Attack the first and last byte
		//if( cipher.size() > 1 )
		//{
		// cipher[ 0 ] |= 0x0F;
		// cipher[ cipher.size()-1 ] |= 0x0F;
		//}

		/*********************************\
		\*********************************/

		/*********************************\
		\*********************************/



		{
			sf::Packet packet;
			packet << PacketType::TestMessage << cipher;
			for (int i = 0; i < CryptoPP::AES::BLOCKSIZE; ++i) {
				packet << iv[i];
			}
			socket.send(packet); // TODO: Do something on fail + put it on separate thread?
			std::this_thread::sleep_for(3s);
		}


	}
#endif
    std::string programName = removeExtension(getFilename(argv[0]));
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
    Text infoText;
    infoText.setText(renderer, robotoF, "");
    infoText.dstR.w = 300;
    infoText.dstR.h = 20;
    infoText.dstR.x = 15;
    infoText.dstR.y = windowHeight - infoText.dstR.h - 15;
    std::string login;
    std::string password;
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
    ml.callBtnR.w = 256;
    ml.callBtnR.h = 60;
    ml.callBtnR.x = ml.nameR.x + ml.nameR.w;
    ml.callBtnR.y = windowHeight - ml.callBtnR.h;
    ml.pickUpBtnR.w = 256;
    ml.pickUpBtnR.h = 60;
    ml.pickUpBtnR.x = ml.callBtnR.x + ml.callBtnR.w + 5;
    ml.pickUpBtnR.y = windowHeight - ml.pickUpBtnR.h;
    ml.callerNameText.autoAdjustW = true;
    ml.callerNameText.wMultiplier = 0.3;
    ml.callerNameText.dstR.h = 20;
    ml.callerNameText.dstR.x = ml.pickUpBtnR.x + ml.pickUpBtnR.w;
    ml.callerNameText.dstR.y = ml.pickUpBtnR.y + ml.pickUpBtnR.h / 2 - ml.callerNameText.dstR.h / 2;
    ml.userInfoText.setText(renderer, robotoF, "");
    ml.userInfoText.dstR.w = 40;
    ml.userInfoText.dstR.h = 20;
    ml.userInfoText.dstR.x = windowWidth - ml.userInfoText.dstR.w;
    ml.userInfoText.dstR.y = windowHeight - ml.userInfoText.dstR.h;
    ml.userInfoText.dstR.x += 3;
    ml.userInfoText.autoAdjustW = true;
    ml.userInfoText.wMultiplier = 0.5;
    std::string callerName;
    std::string receiverName;
#endif
#if 1 // NOTE: MessageContent
    SDL_Texture* closeT = IMG_LoadTexture(renderer, "res/close.png");
    SDL_Texture* deleteT = IMG_LoadTexture(renderer, "res/delete.png");
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
    Scroll scroll = Scroll::Non;
    std::vector<Text> texts;
    Text attachmentsText;
    attachmentsText.setText(renderer, robotoF, u8"Załączniki", { 255, 255, 255 });
    attachmentsText.dstR.w = 70;
    attachmentsText.dstR.h = 32;
    attachmentsText.dstR.x = windowWidth / 2 - attachmentsText.dstR.w / 2;
    attachmentsText.dstR.y = contentR.y + contentR.h;
    attachmentsText.autoAdjustW = true;
    SDL_FRect attachmentR;
    attachmentR.x = 0;
    attachmentR.y = attachmentsText.dstR.y + attachmentsText.dstR.h;
    attachmentR.w = windowWidth / 2.f;
    attachmentR.h = windowHeight - attachmentR.y;
    SDL_Texture* downloadT = IMG_LoadTexture(renderer, "res/download.png");
    SDL_FRect downloadBtnR;
    downloadBtnR.w = 128;
    downloadBtnR.h = 32;
    downloadBtnR.x = 0;
    downloadBtnR.y = attachmentsText.dstR.y;
    SDL_FRect attachmentsScrollR;
    attachmentsScrollR.w = 20;
    attachmentsScrollR.h = attachmentR.h;
    attachmentsScrollR.x = windowWidth - attachmentsScrollR.w;
    attachmentsScrollR.y = attachmentR.y;
    SDL_FRect attachmentsScrollBtnR;
    attachmentsScrollBtnR.w = attachmentsScrollR.w;
    attachmentsScrollBtnR.h = 30;
    attachmentsScrollBtnR.x = attachmentsScrollR.x;
    attachmentsScrollBtnR.y = attachmentsScrollR.y;
#endif
#if 1 // NOTE: MessageSend
    SDL_Texture* sendT = IMG_LoadTexture(renderer, "res/send.png");
    SDL_Texture* swapT = IMG_LoadTexture(renderer, "res/swap.png");
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
    SDL_FRect msSwapBtnR;
    msSwapBtnR.w = closeBtnR.w;
    msSwapBtnR.h = closeBtnR.h;
    msSwapBtnR.x = closeBtnR.x + closeBtnR.w;
    msSwapBtnR.y = closeBtnR.y;
    Text msNameText;
    msNameText.setText(renderer, robotoF, u8"Nazwa", { TEXT_COLOR });
    msNameText.dstR.w = 60;
    msNameText.dstR.h = 20;
    msNameText.dstR.x = msSwapBtnR.x + msSwapBtnR.w + getValueFromValueAndPercent(windowWidth - closeBtnR.w, 50) / 2 - msNameText.dstR.w / 2;
    msNameText.dstR.y = 0;
    Text msTopicText;
    msTopicText.setText(renderer, robotoF, "Temat", { TEXT_COLOR });
    msTopicText.dstR.w = 60;
    msTopicText.dstR.h = 20;
    msTopicText.dstR.x = msSwapBtnR.x + msSwapBtnR.w + getValueFromValueAndPercent(windowWidth - closeBtnR.w, 50) + getValueFromValueAndPercent(windowWidth - closeBtnR.w, 50) / 2 - msTopicText.dstR.w / 2;
    msTopicText.dstR.y = msNameText.dstR.y;
    msTopicText.autoAdjustW = true;
    Text msContentText;
    msContentText.setText(renderer, robotoF, u8"Treść", { TEXT_COLOR });
    msContentText.dstR.w = 60;
    msContentText.dstR.h = 20;
    msContentText.dstR.x = windowWidth / 2 - msContentText.dstR.w / 2;
    msContentText.dstR.y = closeBtnR.y + closeBtnR.h;
    Text msAttachmentsText;
    msAttachmentsText.setText(renderer, robotoF, u8"Załączniki", { TEXT_COLOR });
    msAttachmentsText.dstR.w = 70;
    msAttachmentsText.dstR.h = 20;
    msAttachmentsText.dstR.x = windowWidth / 2 - msAttachmentsText.dstR.w / 2;
    msAttachmentsText.dstR.y = msContentText.dstR.y + msContentText.dstR.h + windowHeight - closeBtnR.h - msSendBtnR.h - 260;
    msAttachmentsText.autoAdjustW = true;
    std::vector<Attachment> msAttachments;
    SDL_FRect msAttachmentR;
    msAttachmentR.x = 0;
    msAttachmentR.y = msAttachmentsText.dstR.y + msAttachmentsText.dstR.h;
    msAttachmentR.w = windowWidth / 2.f;
    msAttachmentR.h = msSendBtnR.y - msAttachmentR.y;
    SDL_FRect msAttachmentsScrollR;
    msAttachmentsScrollR.w = 20;
    msAttachmentsScrollR.h = msAttachmentR.h;
    msAttachmentsScrollR.x = windowWidth - attachmentsScrollR.w;
    msAttachmentsScrollR.y = msAttachmentR.y;
    SDL_FRect msAttachmentsScrollBtnR;
    msAttachmentsScrollBtnR.w = msAttachmentsScrollR.w;
    msAttachmentsScrollBtnR.h = 30;
    msAttachmentsScrollBtnR.x = msAttachmentsScrollR.x;
    msAttachmentsScrollBtnR.y = msAttachmentsScrollR.y;
    std::string msReceiver;
    std::string msTopic;
    std::string msMessage;
    bool shouldUseFirstWidgets = true;
#endif
#if 0 // NOTE: Call
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
    SDL_FRect disconnectBtnR;
    disconnectBtnR.w = 256;
    disconnectBtnR.h = 64;
    disconnectBtnR.x = windowWidth / 2 - disconnectBtnR.w / 2;
    disconnectBtnR.y = windowHeight - disconnectBtnR.h;
#endif
    int messageIndexToShow = -1;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window);
    ImGui_ImplSDLRenderer_Init(renderer);
    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddText(u8"Zażółć gęślą jaźń");
    builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
    builder.BuildRanges(&ranges);
    io.Fonts->AddFontFromFileTTF("res/roboto.ttf", 16.0f, 0, ranges.Data);
    io.Fonts->Build();
    bool showDemoWindow = true;
    while (running) {
        if (state == State::LoginAndRegister) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    running = false;
                    // TODO: On mobile remember to use eventWatch function (it doesn't reach this code when terminating)
                }
                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    SDL_RenderSetScale(renderer, event.window.data1 / (float)windowWidth, event.window.data2 / (float)windowHeight);
                }
                if (event.type == SDL_KEYDOWN) {
                    keys[event.key.keysym.scancode] = true;
                    if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
                        sendLoginAndPasswordToServer(login, password, robotoF, socket, state, infoText, ml);
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
            }
            ImGui_ImplSDLRenderer_NewFrame();
            ImGui_ImplSDL2_NewFrame(window);
            io.MousePos.x = mousePos.x;
            io.MousePos.y = mousePos.y;
            ImGui::NewFrame();
            if (showDemoWindow) {
                ImGui::ShowDemoWindow(&showDemoWindow);
            }
            {
                ImGui::Begin("Login", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
                ImGui::SetWindowPos("Login", { windowWidth / 2.f - ImGui::GetWindowSize().x / 2, windowHeight / 2.f - ImGui::GetWindowSize().y / 2 });
                ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 400) * 0.5f);
                ImGui::SetCursorPosY((ImGui::GetWindowSize().y) * 0.5f);
                ImGui::InputTextWithHint("##Nazwa", "Nazwa", &login);
                ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 400) * 0.5f);
                ImGui::InputTextWithHint(u8"##Hasło", u8"Hasło", &password, ImGuiInputTextFlags_Password); // TODO: Report a bug or fix it that when you move cursor using left arrow cursor somethimes disappears
                ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 100) * 0.5f);
                if (ImGui::Button("Login", { 200, 50 })) {
                    sendLoginAndPasswordToServer(login, password, robotoF, socket, state, infoText, ml);
                }
                ImGui::End();
            }
            ImGui::Render();
            SDL_SetRenderDrawColor(renderer, BG_COLOR);
            SDL_RenderClear(renderer);
            ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
            SDL_RenderSetViewport(renderer, 0);
            infoText.draw(renderer);
            SDL_RenderPresent(renderer);
        }
        else if (state == State::MessageList) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);
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
                                        scroll = Scroll::Non;
                                    }
                                }
                                scrollBtnR.y = scrollR.y;
                                attachmentsScrollBtnR.y = attachmentsScrollR.y;
                                break;
                            }
                        }
                    }
                    if (SDL_PointInFRect(&mousePos, &ml.writeMessageBtnR)) {
                        state = State::MessageSend;
                        msAttachmentsScrollBtnR.y = msAttachmentsScrollR.y;
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
            {
                pugi::xml_document doc;
                pugi::xml_node rootNode = doc.append_child("root");
                rootNode.append_child("packetType").append_child(pugi::node_pcdata).set_value(std::to_string(PacketType::ReceiveMessages).c_str());
                std::stringstream ss;
                doc.save(ss);
                send(socket, ss.str());
            }
            std::string receiveMsg;
            receive(socket, receiveMsg); // TODO: Do something on fail + put it on separate thread?

            std::vector<Message> newMessages;
            newMessages.push_back(Message());
            {
                pugi::xml_document doc;
                doc.load_string(receiveMsg.c_str());
                auto messageNodes = doc.child("root").children("message");
                int i = 0;
                for (auto it = messageNodes.begin(); it != messageNodes.end(); ++it) {
                    newMessages.back().topicText.text = it->child("topic").text().as_string();
                    newMessages.back().senderNameText.text = it->child("senderName").text().as_string();
                    newMessages.back().contentText.text = it->child("content").text().as_string();
                    newMessages.back().dateText.text = it->child("date").text().as_string();
                    int attachmentNodesCount = it->child("attachmentNodesCount").text().as_int();
                    auto attachmentNodes = it->children("attachment");
                    for (pugi::xml_node& attachmentNode : attachmentNodes) {
                        newMessages.back().attachments.push_back(Attachment());
                        newMessages.back().attachments.back().path = attachmentNode.child("path").text().as_string();
                        newMessages.back().attachments.back().fileContent = attachmentNode.child("fileContent").text().as_string();
                        newMessages.back().attachments.back().text.autoAdjustW = true;
                        newMessages.back().attachments.back().text.wMultiplier = 0.2;
                        newMessages.back().attachments.back().text.setText(renderer, robotoF, getFilename(newMessages.back().attachments.back().path), { 255, 255, 255 });
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
                    if (ml.messages[i].topicText.text != newMessages[i].topicText.text || ml.messages[i].senderNameText.text != newMessages[i].senderNameText.text || ml.messages[i].dateText.text != newMessages[i].dateText.text || ml.messages[i].contentText.text != newMessages[i].contentText.text) {
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
                        ml.callerNameAndSurnameText.setText(renderer, robotoF, callerName + " " + callerSurname, { 255, 255, 255 });
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
            ml.userInfoText.dstR.x = windowWidth - ml.userInfoText.dstR.w;
            ml.userInfoText.dstR.y = windowHeight - ml.userInfoText.dstR.h;
            ml.userInfoText.draw(renderer);
            SDL_RenderPresent(renderer);
        }
        else if (state == State::MessageContent) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);
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
                        for (int i = 0; i < ml.messages[messageIndexToShow].attachments.size(); ++i) {
                            if (i == 0) {
                                ml.messages[messageIndexToShow].attachments[i].text.dstR.y = attachmentsText.dstR.y + attachmentsText.dstR.h;
                            }
                            else {
                                ml.messages[messageIndexToShow].attachments[i].text.dstR.y = ml.messages[messageIndexToShow].attachments[i - 1].text.dstR.y + ml.messages[messageIndexToShow].attachments[i - 1].text.dstR.h;
                            }
                        }
                    }
                    for (int i = 0; i < ml.messages[messageIndexToShow].attachments.size(); ++i) {
                        if (SDL_PointInFRect(&mousePos, &ml.messages[messageIndexToShow].attachments[i].text.dstR) && ml.messages[messageIndexToShow].attachments[i].text.dstR.y >= attachmentsScrollR.y) {
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
                                std::string filename = getFilename(ml.messages[messageIndexToShow].attachments[i].path);
                                std::wstring finalPath = path + L"\\" + converter.from_bytes(filename);
                                bool shouldSave = true;
                                if (exists(finalPath)) {
                                    SDL_SysWMinfo wmInfo;
                                    SDL_VERSION(&wmInfo.version);
                                    SDL_GetWindowWMInfo(window, &wmInfo);
                                    HWND hwnd = wmInfo.info.win.window;
                                    std::wstring caption = std::wstring(L"Czy chcesz nadpisać ten plik? ") + converter.from_bytes(filename);
                                    int result = MessageBox(hwnd, caption.c_str(), L"Ten plik istnieje", MB_YESNO);
                                    if (result == IDYES) {
                                        ;
                                    }
                                    else if (result == IDNO) {
                                        shouldSave = false;
                                    }
                                }
                                if (shouldSave) {
                                    std::ofstream ofs(ws2s(finalPath), std::ofstream::out | std::ofstream::binary);
                                    ofs << ml.messages[messageIndexToShow].attachments[i].fileContent;
                                    ShellExecute(0, L"open", path.c_str(), 0, 0, SW_SHOWDEFAULT);
                                }
                            }
                        }
                    }
                    if (SDL_PointInFRect(&mousePos, &downloadBtnR)) {
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
                            for (int i = 0; i < ml.messages[messageIndexToShow].attachments.size(); ++i) {
                                std::string filename = getFilename(ml.messages[messageIndexToShow].attachments[i].path);
                                std::wstring finalPath = path + L"\\" + converter.from_bytes(filename);
                                bool shouldSave = true;
                                if (exists(finalPath)) {
                                    SDL_SysWMinfo wmInfo;
                                    SDL_VERSION(&wmInfo.version);
                                    SDL_GetWindowWMInfo(window, &wmInfo);
                                    HWND hwnd = wmInfo.info.win.window;
                                    std::wstring caption = std::wstring(L"Czy chcesz nadpisać ten plik? ") + converter.from_bytes(filename);
                                    int result = MessageBox(hwnd, caption.c_str(), L"Ten plik istnieje", MB_YESNO);
                                    if (result == IDYES) {
                                        ;
                                    }
                                    else if (result == IDNO) {
                                        shouldSave = false;
                                    }
                                }
                                if (shouldSave) {
                                    std::ofstream ofs(ws2s(finalPath), std::ofstream::out | std::ofstream::binary);
                                    ofs << ml.messages[messageIndexToShow].attachments[i].fileContent;
                                }
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
                    else if (SDL_PointInFRect(&mousePos, &attachmentR) || SDL_PointInFRect(&mousePos, &attachmentsScrollR)) {
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
                                    float maxVisible = attachmentR.h / ml.messages[messageIndexToShow].attachments.front().text.dstR.h; // NOTE: Assume that items have same height
                                    float percent = 100 / (ml.messages[messageIndexToShow].attachments.size() - maxVisible);
                                    float max = 0 + attachmentsScrollR.h - attachmentsScrollBtnR.h;
                                    float min = 0;
                                    attachmentsScrollBtnR.y -= (percent * (max - min) / 100) + min;
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
                                    float maxVisible = attachmentR.h / ml.messages[messageIndexToShow].attachments.front().text.dstR.h; // NOTE: Assume that items have same height
                                    float percent = 100 / (ml.messages[messageIndexToShow].attachments.size() - maxVisible);
                                    float max = 0 + attachmentsScrollR.h - attachmentsScrollBtnR.h;
                                    float min = 0;
                                    attachmentsScrollBtnR.y += (percent * (max - min) / 100) + min;
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
                scroll = Scroll::Non;
                for (Text& text : texts) {
                    if (text.dstR.y >= contentR.y && text.dstR.y + text.dstR.h <= contentR.y + contentR.h) {
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
                if (ml.messages[messageIndexToShow].attachments[i].text.dstR.y + ml.messages[messageIndexToShow].attachments[i].text.dstR.h <= attachmentR.y + attachmentR.h && ml.messages[messageIndexToShow].attachments[i].text.dstR.y >= attachmentR.y) {
                    SDL_SetRenderDrawColor(renderer, 125, 55, 34, 0);
                    ml.messages[messageIndexToShow].attachments[i].bgR = ml.messages[messageIndexToShow].attachments[i].text.dstR;
                    ml.messages[messageIndexToShow].attachments[i].bgR.w += 50;
                    SDL_RenderFillRectF(renderer, &ml.messages[messageIndexToShow].attachments[i].bgR);
                    ml.messages[messageIndexToShow].attachments[i].text.draw(renderer);
                }
            }
            SDL_RenderCopyF(renderer, downloadT, 0, &downloadBtnR);
            SDL_SetRenderDrawColor(renderer, 23, 23, 23, 255);
            SDL_RenderFillRectF(renderer, &attachmentsScrollR);
            SDL_SetRenderDrawColor(renderer, 77, 77, 77, 255);
            SDL_RenderFillRectF(renderer, &attachmentsScrollBtnR);
            SDL_RenderPresent(renderer);
        }
        else if (state == State::MessageSend) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);
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
                        msAttachments.clear();
                    }
                    if (SDL_PointInFRect(&mousePos, &msSwapBtnR)) {
                        if (msNameText.text == "Klasa") {
                            msNameText.setText(renderer, robotoF, "Nazwa", {});
                        }
                        else {
                            msNameText.setText(renderer, robotoF, "Klasa", {});
                        }
                    }
                    if (SDL_PointInFRect(&mousePos, &msSendBtnR)) {
                        sendMessage(socket, msNameText, msReceiver, msTopic, msMessage, renderer, robotoF, msAttachments);
                        shouldUseFirstWidgets = !shouldUseFirstWidgets;
                    }
                    if (SDL_PointInFRect(&mousePos, &msAddAttachmentBtnR)) {
                        const nfdpathset_t* outPaths;
                        nfdresult_t result = NFD_OpenDialogMultipleN(&outPaths, 0, 0, 0);
                        if (result == NFD_OKAY) {
                            nfdpathsetsize_t count;
                            nfdresult_t result = NFD_PathSet_GetCount(outPaths, &count);
                            if (result == NFD_OKAY) {
                                for (int i = 0; i < count; ++i) {
                                    nfdnchar_t* outPath;
                                    nfdresult_t result = NFD_PathSet_GetPathN(outPaths, i, &outPath);
                                    if (result == NFD_OKAY) {
                                        std::ifstream ifs(outPath, std::ifstream::in | std::ifstream::binary);
                                        std::stringstream ss;
                                        ss << ifs.rdbuf();
                                        msAttachments.push_back(Attachment());
                                        msAttachments.back().fileContent = ss.str();
                                        std::wstring outPathStr(outPath);
                                        msAttachments.back().path = converter.to_bytes(outPathStr);
                                        msAttachments.back().text.autoAdjustW = true;
                                        msAttachments.back().text.wMultiplier = 0.2;
                                        msAttachments.back().text.setText(renderer, robotoF, getFilename(msAttachments.back().path), { TEXT_COLOR });
                                        msAttachments.back().text.dstR.h = 20;
                                        msAttachments.back().text.dstR.x = 0;
                                        if (msAttachments.size() == 1) {
                                            msAttachments.back().text.dstR.y = msAttachmentsText.dstR.y + msAttachmentsText.dstR.h;
                                        }
                                        else {
                                            msAttachments.back().text.dstR.y = msAttachments[msAttachments.size() - 2].text.dstR.y + msAttachments[msAttachments.size() - 2].text.dstR.h;
                                        }
                                        msAttachments.back().bgR = msAttachments.back().text.dstR;
                                        msAttachments.back().bgR.w += 50;
                                        msAttachments.back().deleteBtnR.w = msAttachments.back().bgR.h;
                                        msAttachments.back().deleteBtnR.h = msAttachments.back().bgR.h;
                                        msAttachments.back().deleteBtnR.x = msAttachments.back().bgR.x + msAttachments.back().bgR.w;
                                        msAttachments.back().deleteBtnR.y = msAttachments.back().bgR.y;
                                        NFD_PathSet_FreePathN(outPath);
                                    }
                                }
                            }
                            NFD_PathSet_Free(outPaths);
                        }
                    }
                    {
                        int i = 0;
                        for (; i < msAttachments.size(); ++i) {
                            if (SDL_PointInRect(&mousePos, &msAttachments[i].deleteBtnR)) {
                                msAttachments.erase(msAttachments.begin() + i--);
                                break;
                            }
                        }
                        ++i;
                        for (; i < msAttachments.size(); ++i) {
                            msAttachments[i].text.dstR.y -= msAttachments[i].text.dstR.h;
                            msAttachments[i].deleteBtnR.y -= msAttachments[i].deleteBtnR.h;
                            msAttachments[i].bgR.y -= msAttachments[i].bgR.h;
                        }
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
                                    attachment.bgR.y += attachment.bgR.h;
                                    attachment.deleteBtnR.y += attachment.deleteBtnR.h;
                                }
                                float maxVisible = msAttachmentR.h / msAttachments.front().text.dstR.h; // NOTE: Assume that items have same height
                                float percent = 100 / (msAttachments.size() - maxVisible);
                                float max = 0 + msAttachmentsScrollR.h - msAttachmentsScrollBtnR.h;
                                float min = 0;
                                msAttachmentsScrollBtnR.y -= (percent * (max - min) / 100) + min;
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
                                    attachment.bgR.y -= attachment.bgR.h;
                                    attachment.deleteBtnR.y -= attachment.deleteBtnR.h;
                                }
                                float maxVisible = msAttachmentR.h / msAttachments.front().text.dstR.h; // NOTE: Assume that items have same height
                                float percent = 100 / (msAttachments.size() - maxVisible);
                                float max = 0 + msAttachmentsScrollR.h - msAttachmentsScrollBtnR.h;
                                float min = 0;
                                msAttachmentsScrollBtnR.y += (percent * (max - min) / 100) + min;
                            }
                        }
                    }
                }
            }
            ImGui_ImplSDLRenderer_NewFrame();
            ImGui_ImplSDL2_NewFrame(window);
            io.MousePos.x = mousePos.x;
            io.MousePos.y = mousePos.y;
            ImGui::NewFrame();
            {
                ImGui::Begin("MessageSend", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
                ImGui::Text("");
                ImGui::Text("");
                ImGui::PushItemWidth(ImGui::GetWindowSize().x / 2.1);
                if (shouldUseFirstWidgets) {
                    ImGui::InputText("##receiver", &msReceiver);
                }
                else {
                    ImGui::InputText("##receiver2", &msReceiver);
                }
                ImGui::SameLine(0, 5);
                ImGui::PushItemWidth(ImGui::GetWindowSize().x / 2.1);
                if (shouldUseFirstWidgets) {
                    ImGui::InputText("##topic", &msTopic);
                }
                else {
                    ImGui::InputText("##topic2", &msTopic);
                }
                ImGui::Text("");
                ImGui::Text("");
                if (shouldUseFirstWidgets) {
                    ImGui::InputTextMultiline("##message", &msMessage, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));
                }
                else {
                    ImGui::InputTextMultiline("##message2", &msMessage, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));
                }
                ImGui::End();
            }
            ImGui::Render();
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
            SDL_RenderClear(renderer);
            ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
            SDL_RenderSetViewport(renderer, 0);
            SDL_RenderCopyF(renderer, closeT, 0, &closeBtnR);
            msNameText.draw(renderer);
            msTopicText.draw(renderer);
            msContentText.draw(renderer);
            SDL_SetRenderDrawColor(renderer, 52, 131, 235, 255);
            SDL_RenderCopyF(renderer, sendT, 0, &msSendBtnR);
            SDL_RenderCopyF(renderer, addAttachmentT, 0, &msAddAttachmentBtnR);
            msAttachmentsText.draw(renderer);
            for (int i = 0; i < msAttachments.size(); ++i) {
                if (msAttachments[i].text.dstR.y + msAttachments[i].text.dstR.h <= msAttachmentR.y + msAttachmentR.h && msAttachments[i].text.dstR.y >= msAttachmentR.y) {
                    SDL_SetRenderDrawColor(renderer, 125, 55, 34, 0);
                    SDL_RenderFillRectF(renderer, &msAttachments[i].bgR);
                    msAttachments[i].text.draw(renderer);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
                    SDL_RenderFillRect(renderer, &msAttachments[i].deleteBtnR);
                    SDL_RenderCopy(renderer, deleteT, 0, &msAttachments[i].deleteBtnR);
                }
            }
            SDL_SetRenderDrawColor(renderer, 23, 23, 23, 255);
            SDL_RenderFillRectF(renderer, &msAttachmentsScrollR);
            SDL_SetRenderDrawColor(renderer, 77, 77, 77, 255);
            SDL_RenderFillRectF(renderer, &msAttachmentsScrollBtnR);
            SDL_RenderCopyF(renderer, swapT, 0, &msSwapBtnR);
            SDL_RenderPresent(renderer);
        }
    }
    // TODO: On mobile remember to use eventWatch function (it doesn't reach this code when terminating)
    return 0;
}