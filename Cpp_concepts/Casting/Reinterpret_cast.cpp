#include<iostream>
using namespace std;

struct mydata{
    int a;
    int a1;
    char c;
    bool b;
};

int main()
{
    mydata d;
    d.a=5;d.c='a';d.b=true;d.a1=10;

    int *p1=reinterpret_cast<int*>(&d);
    // this will assume pointer of int size 
    cout<<*p1<<endl;// will show output as 5
    p1++;
    cout<<*p1<<endl;// will show output as 10

    // but if did increment a random value will come as char takes only 1 byte memory
    int *e=p1;
    e++;
    cout<<"garbage value:"<<*e<<endl;

    p1++;// offset p1 till end of it's memory block
    char *p2=reinterpret_cast<char*>(p1);
    cout<<*p2<<endl;// will show output as a

    bool *p3=reinterpret_cast<bool*>(p2);
    p3++;
    cout << *p3 << endl; 


    
    return 0;
}