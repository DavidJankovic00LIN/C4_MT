#include "utilities.hpp"

int toInt(unsigned char *buf)
{
	int val=0;
	val+=((int)buf[0])<<24;
	val+=((int)buf[1])<<16;
	val+=((int)buf[2])<<8;
	val+=((int)buf[3]);
	return val;
}

void toUchar(unsigned char *buf,int value)
{
	buf[0]=(char)(val>>24);
	buf[1]=(char)(val>>16);
	buf[2]=(char)(val>>8);
	buf[3]=(char)(val);
}


string toHex(unsigned char c) {
    char hex[3];
    snprintf(hex, sizeof(hex), "%02X", c);
    return std::string(hex);
}