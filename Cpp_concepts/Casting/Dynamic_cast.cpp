#include <iostream>
using namespace std;

class base {
public:
    static int total_count;

    base() { total_count++; }

    virtual void print() {
        cout << "base print" << endl;
    }

    virtual ~base() {}
};
int base::total_count = 0;

class c1 : public base {
public:
    void print() override {
        cout << "caller 1" << endl;
    }
};

class c2 : public base {
public:
    void print() override {
        cout << "caller 2" << endl;
    }
};

int main() {
    c1 first;
    c2 second;

    // normal call
    first.print();

    // upcasting
    base* b1 = &first;
    base* b2 = &second;

    b1->print();
    b2->print();

    // downcast that FAILS: b2 actually points to a c2, not a c1
    c1* d = dynamic_cast<c1*>(b2);
    if (d) {
        cout << "Downcast succeeded" << endl;
        d->print();
    } else {
        cout << "Downcast failed: object is not a c1" << endl;
    }

    cout << base::total_count << endl;
    return 0;
}