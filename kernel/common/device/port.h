#pragma once

extern unsigned char portByteIn(unsigned short port);
extern void portByteOut(unsigned short port, unsigned char data);
extern unsigned short portWordIn(unsigned short port);
extern void portWordOut(unsigned short port, unsigned short data);
