#ifndef _READ_GZIP_H
#define _READ_GZIP_H

#include <zlib.h>
#include <streambuf>

///Decoding gzip streams in C++ based on a streambuf based class
class DecodeGzip : public std::streambuf
{
protected:
	char *readBuff;
	char *decodeBuff;
	std::streambuf &inStream;
	std::iostream fs;
	z_stream d_stream;
	size_t readBufferSize;
	size_t decodeBufferSize;
	std::string outputBuff;

	void CopyDataToOutputBuff();
	std::streamsize ReturnDataFromOutBuff(char* s, std::streamsize n);

public:
	DecodeGzip(std::streambuf &inStream, 
		size_t readBufferSize = 1024*128, 
		size_t decodeBufferSize = 1024*128);
	virtual ~DecodeGzip();
	std::streamsize xsgetn (char* s, std::streamsize n);
	int uflow();
	std::streamsize showmanyc();
};

#endif //_READ_GZIP_H

