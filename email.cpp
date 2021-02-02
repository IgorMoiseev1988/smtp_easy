#include "email.h"

#include <algorithm>  //for_each()
#include <cstdlib>    //std::srand, std::rand
#include <ctime>      //std::time_t, std::time, std::tm, std::gmtime
#include <functional> //std::hash
#include <iomanip>    //std::setfill, std::setw, std::put_time
#include <ios>        //std::hex, std::dec
#include <sstream>    //std::stringstream

Email::Email() {}

Email::Email(const std::string& from,
          const std::string& to,
          const std::string& subject,
          const std::string& content)
    : from_(WrapByAngelBrackets(from))
    , subject_(subject)
    , content_(content)
{
    addTo(to);
    GenerateDate();
    GenerateMsgID();
}

void Email::setFrom(const std::string& from) {
    from_ = WrapByAngelBrackets(from);
}

void Email::addTo(const std::string &to) {
    to_.push_back(WrapByAngelBrackets(to));
}

void Email::clearTo() {
    to_.clear();
}

void Email::setSubject(const std::string &subject) {
    subject_ = subject;
}

void Email::setContent(const std::string &content) {
    content_ = content;
}

std::string Email::getFrom() const { return from_; }

std::vector<std::string> Email::getAllTo() const { return to_; }

std::string Email::getTo(int index) const {
    if (index < 0 || index >= static_cast<int>(to_.size())) return "";
    return to_[index];
}

size_t Email::getToCount() const {
    return to_.size();
}

std::string Email::WrapByAngelBrackets(const std::string &str) {
    return "<" + str + ">";
}

std::vector<std::string> Email::getDATA() const {
    const std::string RF = "\r\n";
    std::vector<std::string> DATA;
    DATA.push_back(HeaderTo());
    DATA.push_back(HeaderFrom());
    DATA.push_back(HeaderMessageID());
    DATA.push_back(HeaderDate());
    DATA.push_back(HeaderSubject());
    DATA.push_back(RF);
    DATA.push_back(content_ + RF);
    DATA.push_back("." + RF);    
    return DATA;
}

std::string Email::getDomain() const {
    std::string domain = from_.substr(from_.find('@') + 1);
    domain.pop_back();
    return domain;
}

std::string Email::HeaderTo() const {
    std::stringstream header;
    header << "To: ";
    bool first = true;
    for (const auto& address : to_) {
        if (!first) header << ", ";
        header << address;
        first = false;
    }
    header << "\r\n";
    return header.str();
}

std::string Email::HeaderFrom() const {
    std::stringstream header;
    header << "From: " << from_ << "\r\n";
    return header.str();
}

std::string Email::HeaderMessageID() const {
    std::stringstream header;
    header << "Message-ID: " << ID_ << "\r\n";
    return header.str();
}

std::string Email::HeaderDate() const {
    std::stringstream header;
    header << "Date: " << date_ << "\r\n";
    return header.str();
}

std::string Email::HeaderSubject() const {
    std::stringstream header;
    header << "Subject: " << subject_ << "\r\n";
    return header.str();
}

std::string Email::GetHexFromString(const std::string& number) const {
    std::stringstream ss;
    size_t hash_ = std::hash<std::string>{}(number);
    ss << std::hex << hash_;
    return ss.str();
}

void Email::GenerateDate() {
    std::stringstream ss;
    std::time_t sec = std::time(nullptr);
    std::tm* tm_ = std::gmtime(&sec);
    ss << std::put_time(tm_, "%a, %d %b %Y %T %z");
    date_ = ss.str();
}

void Email::GenerateMsgID() {
    std::stringstream ss;
    std::stringstream part_date[4];
    for (size_t p = 0; p < date_.size(); ++p) {
        part_date[p % 4] << date_[p];
    }
    ss << "<";
    bool first = true;
    for (int i = 0; i < 4; ++i) {
        if (!first) ss << ".";
        ss << GetHexFromString(part_date[i].str());
        first = false;
    }
    std::srand(std::time(nullptr));
    ss << "@mail" << std::dec << std::rand() << "-" << std::rand() << "." << getDomain() << ">";
    ID_ = ss.str();
}


/* For test code, delete in future

#include <iostream>
void Email::Print() {
    std::cout << "Email created with members: \n"
              << "from   : " << from_ << "|\n"
              << "to     : " << to_[0] << "|\n"
              << "subject: " << subject_ << "|\n"
              << "content: " << content_ << "|\n"
              << "date   : " << date_ << "|\n"
              << "ID     : " << ID_ << "|\n";
    std::cout << "\nGet DATA return next line: \n";
    for (const auto& s : getDATA()) {
        std::cout << s << "|\n";
    }
}
*/
