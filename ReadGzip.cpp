
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <string.h>
#include "ReadGzip.h"
using namespace std;

std::string ConcatStr(const char *a, const char *b)
{
	string out(a);
	out.append(b);
	return out;
}

DecodeGzip::DecodeGzip(std::streambuf &inStream, 
		size_t readBufferSize, 
		size_t decodeBufferSize) : inStream(inStream), 
			fs(&inStream),
			readBufferSize(readBufferSize),
			decodeBufferSize(decodeBufferSize)
{
	this->readBuff = new char [readBufferSize];
	this->decodeBuff = new char [decodeBufferSize];

	fs.read(this->readBuff, readBufferSize);

	d_stream.zalloc = (alloc_func)NULL;
	d_stream.zfree = (free_func)NULL;
	d_stream.opaque = (voidpf)NULL;
	d_stream.next_in  = (Bytef*)this->readBuff;
	d_stream.avail_in = (uInt)fs.gcount();
	d_stream.next_out = (Bytef*)this->decodeBuff;
	d_stream.avail_out = (uInt)decodeBufferSize;

	//cout << "read " << d_stream.avail_in << endl;
	int err = inflateInit2(&d_stream, 16+MAX_WBITS);
	if(err != Z_OK)
		throw runtime_error(ConcatStr("inflateInit2 failed: ", zError(err)));

}

DecodeGzip::~DecodeGzip()
{
	delete [] this->readBuff;
	delete [] this->decodeBuff;
}

void DecodeGzip::CopyDataToOutputBuff()
{
	int lengthInBuff = (char *)d_stream.next_out - this->decodeBuff;
	this->outputBuff.append(this->decodeBuff, lengthInBuff);
	d_stream.next_out = (Bytef*)this->decodeBuff;
	d_stream.avail_out = (uInt)decodeBufferSize;
}

streamsize DecodeGzip::ReturnDataFromOutBuff(char* s, streamsize n)
{
	int lengthInBuff = this->outputBuff.size();
	int lenToCopy = lengthInBuff;
	if(n < lenToCopy) lenToCopy = n;
	
	memcpy(s, this->outputBuff.c_str(), lenToCopy);
	this->outputBuff = std::string(&this->outputBuff[lenToCopy], this->outputBuff.size() - lenToCopy);
	return lenToCopy;
}

streamsize DecodeGzip::xsgetn (char* s, streamsize n)
{	
	int err = Z_OK;

	if(d_stream.avail_out < decodeBufferSize)
		return ReturnDataFromOutBuff(s, n);

	if(d_stream.avail_in == 0 && !fs.eof())
	{
		fs.read(this->readBuff, readBufferSize);
		d_stream.next_in  = (Bytef*)this->readBuff;
		d_stream.avail_in = (uInt)fs.gcount();
		//cout << "read " << d_stream.avail_in << endl;
	}

	if(d_stream.avail_in > 0)
	{
		err = inflate(&d_stream, Z_NO_FLUSH);

		if (err != Z_STREAM_END)
		{
			if(err != Z_OK)
				throw runtime_error(ConcatStr("inflate failed: ", zError(err)));
			CopyDataToOutputBuff();
			return ReturnDataFromOutBuff(s, n);
		}
	}

	err = inflateEnd(&d_stream);
	if(err != Z_OK)
		throw runtime_error(ConcatStr("inflateEnd failed: ", zError(err)));	
	CopyDataToOutputBuff();
	return ReturnDataFromOutBuff(s, n);
}

int DecodeGzip::uflow()
{
	streamsize inputReady = showmanyc();
	if(inputReady==0) return EOF;
	char buff[1];
	xsgetn(buff, 1);
	return *(unsigned char *)&(buff[0]);
}

streamsize DecodeGzip::showmanyc()
{
	if(this->outputBuff.size() > 0)
		return 1;
	if(d_stream.avail_in > 0)
		return 1;
	int lengthInBuff = (char *)d_stream.next_out - this->decodeBuff;
	if(lengthInBuff > 0)
		return 1;
	return inStream.in_avail() > 1;
}

