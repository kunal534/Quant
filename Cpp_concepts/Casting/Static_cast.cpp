#include<iostream>
#include<typeinfo>
using namespace std;

int main()
{
    int a=10;
    double b=static_cast<double>(a);
    cout<<typeid(b).name()<< " along with value "<<b<<endl;
}