// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <string>
namespace saveName {
inline std::string component(std::string text,const char* fallback)
{
    for(char& c:text) if(static_cast<unsigned char>(c)<32 || std::string("<>:\"/\\|?*").find(c)!=std::string::npos) c='-';
    if(text.size()>64) {
        size_t length=64;
        while((static_cast<unsigned char>(text[length])&0xc0)==0x80) --length;
        text.resize(length);
    }
    const auto first=text.find_first_not_of(" .-");
    if(first==std::string::npos) return fallback;
    text=text.substr(first,text.find_last_not_of(" .-")-first+1);
    return text;
}
inline std::string stem(const std::string& timestamp,const std::string& player,const std::string& map)
{ return timestamp+"-"+component(player,"Spieler")+"-"+component(map,"Karte"); }
}
