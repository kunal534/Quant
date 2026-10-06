#include<iostream>
using namespace std;

int main()
{
    
    const int d1=5;
    const int *p1=&d1;

    int *np1=const_cast<int*>(p1);
    *np1=10;// this is undefined behavior as complier would allow it 
    cout<<*np1<<endl; // this will give 10 as it has broken the promise of const 
    cout<<d1<< endl; // this will give 5 as complier trust const and has replace it with 5
    // the correct way would have been to make sure the intended value is not const

    int d2=10;
    const int *p2=&d2;
    int *np2=const_cast<int*>(p2);
    *np2=15;
    // both will give 10
    cout<<*np2<<endl;
    cout<<d2<<endl;
    return 0;
}