#include "../headers/packetPointer.h"
#pragma comment(lib, "../release/packetPointer")

#include <stdio.h>

class CTestPacketPointer: public CPacketPointer{

public:
	virtual void setHeader(void*){}
	virtual void decoding(void*){}
	virtual void incoding(void*){}

};

int main(){

	CTestPacketPointer pt;

	pt << 123 << 456;

	int a , b;
	pt >> a >> b;

	printf("%d %d\n",a, b);


	return 0;

}