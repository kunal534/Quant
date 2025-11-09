#include<iostream>
#include<vector>
/*
    There are major 2 types of casting:
    1. Implicit-> the one done by complier example int to double but it has risk of data loss
    2. Explicit-> Done by programmer
        (2.a) Static_cast<int>variable => in this one we simply mention the of data type we want if present then complies else it would through error in syntax anlyser
        (2.b) Dynamic_cast<[derived class]*>() => Do a downgrading[ meaning the pointer is of base class converted to one of child class] 
        (2.c) Const_cast<> => Removes the const quantifier
        (2.d) Reinterpret_cast => pointer to int and vice versa


*/
class Shape{
    public:
    virtual void draw() { }// For polymorphism as in during run time based on varible the correct one is choosen
};
class Circle : public Shape {
public:
    void draw() override { std::cout << "Drawing circle" << "\n"; }
    void setRadius(int r) { radius = r; }  // Circle-specific method
private:
    int radius;
};

class Square : public Shape {
public:
    void draw() override { std::cout << "Drawing square" << "\n"; }
    void setSide(int s) { side = s; }  // Square-specific method
private:
    int side;
};
void modifyValue(const int* ptr) {
    // Remove const qualifier
    int* modifiable = const_cast<int*>(ptr);
    *modifiable = 100;
}
int main(){

    int a;
    std::cout<<"Choose the type of casting example"<<"\n";
    std::cin>>a;
    switch(a){
        case 1:{
            float a=3.0;
            int ans=static_cast<int>(a);
            std::cout<<ans<<"\n";
        break;
        }
        
        case 2:{
            std::vector<Shape*>temp;
            temp.push_back(new Circle());
            temp.push_back(new Square());
            temp.push_back(new Circle());

            for(Shape* it:temp){
                /*
                    if it find the value in vtable (virtual table) while using the RTTI (Run-Time Type Information) that is created
                    when for a virtual function storing its meta data
                */
                Circle* circle=dynamic_cast<Circle*>(it);
                if(circle!=nullptr)
                {
                    std::cout<< " casting successfull"<<"\n";
                }
            }
        break;
        }

        case 3:{
            int value = 50;
            const int* constPtr = &value;
            
            std::cout << "Before: " << value << "\n";
            modifyValue(constPtr);
            std::cout << "After: " << value << "\n";
        break;
        }

        case 4:{
            int num = 42;
            int* ptr = &num;
            
            // Reinterpret pointer as integer
            uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
            std::cout << "Address as integer: " << address << "\n";
            
            // Convert back to pointer
            int* newPtr = reinterpret_cast<int*>(address);
            std::cout << "Value: " << *newPtr << "\n";  // Result: 42
            break;
            
            }
        }
            return 0;

}