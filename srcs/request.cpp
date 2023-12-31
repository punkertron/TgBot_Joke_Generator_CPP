#include "request.hpp"

#include <curl/curl.h>

#include <iostream>
#include <utility>

#include "response.hpp"

request::request()
    : m_language("en - English"), m_type("Any"), m_safe_mode("No") {
    m_joke_category.insert("Any");
    m_blacklist.insert("None");
}

std::string request::argument_to_query() const {
    std::string arg;

    for (auto s : m_joke_category) arg = arg + s + ',';
    arg.erase(--arg.end());  // erase last ','

    arg += "?format=json";

    if (m_safe_mode == "Yes") arg += "&safe-mode";
    if (m_language != "en - English")
        arg = arg + "&lang=" + m_language.substr(0, 2);
    if (m_type != "Any") arg = arg + "&type=" + m_type;

    if (m_blacklist.find("None") == m_blacklist.end()) {
        arg += "&blacklistFlags=";
        for (auto s : m_blacklist) arg = arg + s + ',';
        arg.erase(--arg.end());  // erase last ','
    }
    std::cerr << "Query: " << arg << std::endl;
    return std::move(arg);
}

const std::string request::show_filters() const {
    std::string res;

    res = "Categories: ";
    for (auto s : m_joke_category) res += s;
    res += "\nLanguage: ";
    res += m_language;
    res += "\nBlacklist: ";
    for (auto s : m_blacklist) res = res + s + ',';
    res.erase(--res.end());
    res += "\nType (single or twopart): ";
    res += m_type;
    res += "\nSafe-mode: ";
    res += m_safe_mode;
    return (std::move(res));
}

const std::string request::setSafeMode() {
    std::string res("Safe-mode is set to ");
    if (m_safe_mode == "No") {
        m_safe_mode = "Yes";
        res += "Yes";
    } else {
        m_safe_mode = "No";
        res += "No";
    }
    return std::move(res);
}

const std::string request::setDefault() {
    m_language = "en - English";
    m_type = "Any";
    m_safe_mode = "No";
    m_joke_category.clear();
    m_joke_category.insert("Any");
    m_blacklist.clear();
    m_blacklist.insert("None");
    return std::move(std::string("All settings are set to default!"));
}

const std::string request::setLang(const char *s) {
    m_language = s;
    std::string res("Language is set to ");
    res += s;
    return std::move(res);
}

const std::string request::setBlackList(const std::string s) {
    std::size_t pos = s.find(' ');
    if (pos == std::string::npos) pos = s.size();
    std::string temp(s.substr(0, pos));
    if (temp == "None") {
        m_blacklist.clear();
        m_blacklist.insert(std::move(temp));
        return std::move(std::string("Reset to None"));
    }
    m_blacklist.erase("None");
    m_blacklist.insert(temp);
    return std::move(temp + std::string(" added to blacklist"));
}

const std::string request::setCategory(const std::string s) {
    std::size_t pos = s.find(' ');
    if (pos == std::string::npos) pos = s.size();
    std::string temp(s.substr(0, pos));
    if (temp == "Any") {
        m_joke_category.clear();
        m_joke_category.insert(std::move(temp));
        return std::move(std::string("Reset to Any"));
    }
    m_joke_category.erase("Any");
    m_joke_category.insert(temp);
    return std::move(temp + std::string(" added to category list"));
}

const std::string request::setType(const std::string s) {
    std::size_t pos = s.find(' ');
    if (pos == std::string::npos) pos = s.size();
    std::string temp(s.substr(0, pos));
    m_type = temp;
    return std::move(std::string("Type set to ") + m_type);
}

void request::request_joke(struct response &resp) {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        fp = fopen("response.json", "wb");
        if (!fp) {
            std::cerr << "Can't open response.json" << std::endl;
            resp.setError(true);
            return;
        }
        std::string query("https://v2.jokeapi.dev/joke/");
        query += argument_to_query();
        curl_easy_setopt(curl, CURLOPT_URL, query.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: "
                      << curl_easy_strerror(res) << std::endl;
            resp.setError(true);
            curl_easy_cleanup(curl);
            fclose(fp);
            return;
        }
        curl_easy_cleanup(curl);
        fclose(fp);
        resp.update();
    }

    return;
}
