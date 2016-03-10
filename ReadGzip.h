#ifndef _READ_GZIP_H
#define _READ_GZIP_H

#include <zlib.h>
#include <streambuf>

class DecodeGzip : public std::streambuf
{
protected:
	char *readBuff;
	char *decodeBuff;
	std::streambuf &inStream;
	std::iostream fs;
	z_stream d_stream;
	bool decodeDone;
	char *decodeBuffCursor;
	std::streamsize readBuffSize, decodeBuffSize;

	bool Decode();
	int uflow();

public:
	DecodeGzip(std::streambuf &inStream, std::streamsize readBuffSize = 1024*128, std::streamsize decodeBuffSize = 1024*128);
	virtual ~DecodeGzip();
	std::streamsize xsgetn (char* s, std::streamsize n);
	std::streamsize showmanyc();
};

#endif //_READ_GZIP_H

