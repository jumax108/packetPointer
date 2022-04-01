#include "../lib/objectFreeListTLS/headers/objectFreeListTLS.h"

#include "../headers/packetPointer.h"
#pragma comment(lib, "../release/packetPointer")

#include <stdio.h>

class CTestPacketPointer: public CPacketPointer{

public:
	virtual void setHeader(void*){}
	virtual void decoding(void*){}
	virtual void incoding(void*){}

};

void test(){
	
	{
		CTestPacketPointer pt;
		int k = 1;
	}


}

int main(){

	{
		CTestPacketPointer pt;
		pt << 111;
		int a;
		pt >> a;
	}
	
	test();
	test();
	test();
	test();
	test();
	test();
	test();
	test();
	test();
	test();
	test();
	test();
	test();
	test();
	test();
	test();
	test();






	return 0;

}