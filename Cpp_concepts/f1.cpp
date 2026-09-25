#include <iostream>
#include <cstring>
#include <utility>   // for std::move

class MyString {
    char* data;
public:
    // 1. Parameterized constructor
    MyString(const char* s) {
        data = new char[strlen(s) + 1];
        strcpy(data, s);
        std::cout << "Constructed!\n";
    }

    // 2. Copy constructor — takes const lvalue ref
    //    The 'const' is a PROMISE: "I won't modify the source."
    //    That's why it can bind to BOTH lvalues and rvalues.
    MyString(const MyString& other) {
        data = new char[strlen(other.data) + 1];
        strcpy(data, other.data);
        std::cout << "Copied!\n";
    }

    // 3. Move constructor — takes non-const rvalue ref
    //    No 'const' — because it WILL gut the source.
    //    Safe only because the source is an rvalue (disposable).
    MyString(MyString&& other) noexcept {
        data = other.data;
        other.data = nullptr;
        std::cout << "Moved!\n";
    }

    // 4. Destructor
    ~MyString() {
        delete[] data;
        std::cout << "Destroyed!\n";
    }

    void print() const {
        std::cout << (data ? data : "(null)") << "\n";
    }
};

int main() {
    std::cout << "--- Case 1: lvalue -> Copy ---\n";
    MyString s1("Hello");        // Constructed!
    MyString s2 = s1;            // Copied!   (s1 is lvalue, const T& binds)

    std::cout << "\n--- Case 2: rvalue -> Move ---\n";
    MyString s3 = MyString("World");   // Constructed! then Moved!
    // The temporary is an rvalue -> move constructor is preferred.

    std::cout << "\n--- Case 3: std::move(lvalue) -> Move ---\n";
    MyString s4 = std::move(s1);       // Moved!   (cast makes it an rvalue)
    // s1 is now gutted (data == nullptr). Safe to destroy later.

    std::cout << "\n--- Case 4: const rvalue -> Copy (move skipped!) ---\n";
    const MyString cs("ConstTemp");
    MyString s5 = std::move(cs);       // Copied!  (NOT moved!)
    // std::move(cs) is a const rvalue. Move ctor takes non-const T&&,
    // so it can't bind. Copy ctor (const T&) takes it instead.
    // The 'const' promise is what allows this — but it blocks the move.

    std::cout << "\n--- Values ---\n";
    s2.print();   // Hello
    s3.print();   // World
    s4.print();   // Hello   (stolen from s1)
    s5.print();   // ConstTemp

    std::cout << "\n--- End of scope: destruction (reverse order) ---\n";
    return 0;
}