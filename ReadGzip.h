#ifndef _READ_GZIP_H
#define _READ_GZIP_H

#include <zlib.h>
#include <streambuf>

const int READ_BUFF_SIZE = 1024*128;
const int DECODE_BUFF_SIZE = 1024*128;

class DecodeGzip : public std::streambuf
{
protected:
	char readBuff[READ_BUFF_SIZE];
	char decodeBuff[DECODE_BUFF_SIZE];
	std::streambuf &inStream;
	std::iostream fs;
	z_stream d_stream;
	std::string outBuff;
	bool decodeDone;

	bool Decode();
	void CopyToOutputBuffer();
	std::streamsize ReturnDataFromOutBuff(char* s, std::streamsize n);

public:
	DecodeGzip(std::streambuf &inStream);
	virtual ~DecodeGzip();
	std::streamsize xsgetn (char* s, std::streamsize n);
	std::streamsize showmanyc();
};

#endif //_READ_GZIP_H

