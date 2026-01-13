#include "mpd_parser.h"
#include "curl_helper.h"
#include <tinyxml2.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <curl/curl.h>

using namespace tinyxml2;

MPD MPDParser::parse(const std::string& mpdPath,
                     const std::string& serverBaseURL) {
    MPD mpd;
    std::string mpdContent;

    bool isURL = mpdPath.rfind("http", 0) == 0;

    /* ---------- Load MPD ---------- */
    if (isURL) {
        CURL* curl = curl_easy_init();
        if (!curl) return mpd;

        if (!CurlHelper::downloadToString(curl, mpdPath, mpdContent)) {
            curl_easy_cleanup(curl);
            return mpd;
        }
        curl_easy_cleanup(curl);
    } else {
        std::ifstream file(mpdPath);
        if (!file.is_open()) return mpd;
        std::stringstream buffer;
        buffer << file.rdbuf();
        mpdContent = buffer.str();
    }

    /* ---------- Parse XML ---------- */
    XMLDocument doc;
    if (doc.Parse(mpdContent.c_str()) != XML_SUCCESS) {
        std::cerr << "[MPD] XML parse failed\n";
        return mpd;
    }

    /* ---------- Root (ignore namespace) ---------- */
    XMLElement* mpdElem = doc.RootElement();
    if (!mpdElem) return mpd;

    XMLElement* period = mpdElem->FirstChildElement();
    if (!period) return mpd;

    XMLElement* adaptationSet = period->FirstChildElement();
    if (!adaptationSet) return mpd;

    XMLElement* representation =
        adaptationSet->FirstChildElement();
    if (!representation) return mpd;

    /* ---------- BaseURL ---------- */
    XMLElement* baseURLElem = representation->FirstChildElement("BaseURL");
    if (!baseURLElem || !baseURLElem->GetText()) return mpd;

    std::string baseURL = baseURLElem->GetText();

    /* ---------- SegmentTemplate ---------- */
    XMLElement* segTemplate =
        representation->FirstChildElement("SegmentTemplate");
    if (!segTemplate) return mpd;

    const char* mediaTemplate = segTemplate->Attribute("media");
    int timescale = segTemplate->IntAttribute("timescale", 1);

    XMLElement* timeline =
        segTemplate->FirstChildElement("SegmentTimeline");
    if (!timeline) return mpd;

    /* ---------- Parse segments ---------- */
    int currentNumber = -1;

    for (XMLElement* s = timeline->FirstChildElement("S");
         s != nullptr;
         s = s->NextSiblingElement("S")) {

        if (s->Attribute("t"))
            currentNumber = s->IntAttribute("t");
        else
            currentNumber++;

        int d = s->IntAttribute("d");
        double duration = (double)d / timescale;

        std::string filename = mediaTemplate;
        size_t pos = filename.find("$Number$");
        if (pos != std::string::npos)
            filename.replace(pos, 8, std::to_string(currentNumber));

        Segment seg;
        seg.number = currentNumber;
        seg.duration = duration;
        seg.url = serverBaseURL + baseURL + filename;

        mpd.segments.push_back(seg);
    }

    std::cout << "[MPD] Parsed " << mpd.segments.size() << " segments\n";
    std::cout << "[MPD] First URL: " << mpd.segments[0].url << "\n";

    return mpd;
}
