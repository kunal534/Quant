#include<iostream>
#include<thread>
#include<chrono>

using namespace std;
using namespace std::chrono;
typedef long long int  ull;

void findEven(ull start, ull end, ull* EvenSum) {
	for (ull i = start; i <= end; ++i){
		if (!(i & 1)){
			*(EvenSum) += i;
		}
	}
}

void findOdd(ull start, ull end, ull* OddSum) {
	for (ull i = start; i <= end; ++i){
		if (i & 1){
			(*OddSum) += i;
		}
	}
}

int main() {
	
	ull start = 0, end = 1900000000;

	ull OddSum = 0;
	ull EvenSum = 0;
    
    auto startTime = high_resolution_clock::now(); 
	
	// // WITH THREAD called with function pointer
    std::thread t1(findEven, start, end, &(EvenSum));
    std::thread t2(findOdd, start, end, &(OddSum));
	
	t1.join();
	t2.join();
    auto stopTime = high_resolution_clock::now(); 
    auto duration = duration_cast<microseconds>(stopTime - startTime);

	cout << "time with threading " << duration.count()/1000000 << endl;
	
	// // WITHOUT THREAD
	OddSum=0;
	EvenSum=0;
	startTime= high_resolution_clock::now();
	findEven(start,end, &EvenSum);
	findOdd(start, end, &OddSum);
	auto endTime=high_resolution_clock::now();

	duration=duration_cast<microseconds>(endTime-startTime);

	cout<<"time without threading "<<duration.count()/1000000<<"\n";
	return 0;
}