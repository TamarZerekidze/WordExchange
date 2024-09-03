#include <utility>
#include "User.h"
using namespace std;

User::User() = default;
User::User(std::string name, std::string password)
    : username(std::move(name)), password(std::move(password)), dateAdded(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) {}

User::User(string name, string password, const time_t dateAdded) : username(std::move(name)), password(std::move(password)), dateAdded(dateAdded) {
} /* TODO: might use & for strings and remove move() */

[[nodiscard]] long long User::getId() const{
    return this->id;
}

[[nodiscard]] string User::getUsername() const{
    return this->username;
}

[[nodiscard]] string User::getPassword() const{
    return this->password;
}

[[nodiscard]] time_t User::getTimeAdded() const{
    const time_t t = this->dateAdded;
    return t;
}

void User::setUserId(const long long id){
    this->id = id;
}

void User::setUsername(const string &name){
    this->username = name;
}

void User::setPassword(const string &pass){
    this->password = pass;
}

void User::setDateAdded(const time_t dateAdded){
    this->dateAdded = dateAdded;
}

bool User::operator==(const User& o) const {
    return this->username == o.username &&
           this->password == o.password;
}


