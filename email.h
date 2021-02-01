#ifndef EMAIL_H
#define EMAIL_H

#include <string>
#include <vector>

class Email {
public:
    Email();
    Email(const std::string& from,
          const std::string& to,
          const std::string& subject,
          const std::string& content);
    void setFrom(const std::string& from);
    void addTo(const std::string& to);
    void clearTo();
    void setSubject(const std::string& subject);
    void setContent(const std::string& content);

    std::string getFrom() const;
    std::vector<std::string> getAllTo() const;
    std::string getTo(int index) const;
    size_t getToCount() const;
    std::vector<std::string> getDATA() const;
    std::string getDomain() const;
private:
    std::string from_;
    std::vector<std::string> to_;
    std::string subject_;
    std::string content_;
    std::string date_;
    std::string ID_;

    std::string WrapByAngelBrackets(const std::string& str);
    std::string HeaderTo() const;
    std::string HeaderFrom() const;
    std::string HeaderMessageID() const;
    std::string HeaderDate() const;
    std::string HeaderSubject() const;
    std::string GetHexFromString(const std::string& str) const;

    void GenerateDate();
    void GenerateMsgID();

    void Print();
};

#endif // EMAIL_H
