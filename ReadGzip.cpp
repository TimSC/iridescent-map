
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include "ReadGzip.h"
using namespace std;

std::string ConcatStr(const char *a, const char *b)
{
	string out(a);
	out.append(b);
	return out;
}


DecodeGzip::DecodeGzip(std::streambuf &inStream) : inStream(inStream), fs(&inStream), decodeDone(false)
{
	decodeBuffCursor = NULL;
	fs.read(this->readBuff, READ_BUFF_SIZE);

	d_stream.zalloc = (alloc_func)NULL;
	d_stream.zfree = (free_func)NULL;
	d_stream.opaque = (voidpf)NULL;
	d_stream.next_in  = (Bytef*)this->readBuff;
	d_stream.avail_in = (uInt)fs.gcount();
	d_stream.next_out = (Bytef*)this->decodeBuff;
	d_stream.avail_out = (uInt)DECODE_BUFF_SIZE;

	//cout << "read " << d_stream.avail_in << endl;
	int err = inflateInit2(&d_stream, 16+MAX_WBITS);
	if(err != Z_OK)
		throw runtime_error(ConcatStr("inflateInit2 failed: ", zError(err)));
	decodeBuffCursor = decodeBuff;
}

bool DecodeGzip::Decode()
{
	int err = Z_OK;
	while(!fs.eof())
	{
		if(d_stream.avail_in == 0 && !fs.eof())
		{
			fs.read(this->readBuff, READ_BUFF_SIZE);
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

				decodeBuffCursor = decodeBuff;
				return false;
			}
		}
	}

	err = inflate(&d_stream, Z_FINISH);
	if(err != Z_OK && err != Z_STREAM_END)
		throw runtime_error(ConcatStr("inflate failed: ", zError(err)));

	err = inflateEnd(&d_stream);
	if(err != Z_OK)
		throw runtime_error(ConcatStr("inflateEnd failed: ", zError(err)));
	
	decodeDone = true;
	decodeBuffCursor = decodeBuff;
	return true;
}

DecodeGzip::~DecodeGzip()
{

}

streamsize DecodeGzip::xsgetn (char* s, streamsize n)
{	
	int err = Z_OK;
	char *outputBuffCursor = s;
	streamsize outputTotal = 0;

	while(outputTotal < n && showmanyc() > 0)
	{
		if(!decodeDone && d_stream.avail_out == (uInt)DECODE_BUFF_SIZE)
		{
			Decode();
		}

		streamsize bytesInDecodeBuff = (char *)d_stream.next_out - decodeBuffCursor;
		if(bytesInDecodeBuff > 0)
		{
			streamsize bytesToCopy = n - outputTotal;
			if (bytesToCopy > bytesInDecodeBuff)
				bytesToCopy = bytesInDecodeBuff;
			memcpy(outputBuffCursor, decodeBuffCursor, bytesToCopy);
			outputBuffCursor += bytesToCopy;
			decodeBuffCursor += bytesToCopy;
			outputTotal += bytesToCopy;
		}

		bytesInDecodeBuff = (char *)d_stream.next_out - decodeBuffCursor;
		if(bytesInDecodeBuff == 0)
		{
			d_stream.next_out = (Bytef*)this->decodeBuff;
			d_stream.avail_out = (uInt)DECODE_BUFF_SIZE;
		}

	}

	return outputTotal;
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
	streamsize bytesInDecodeBuff = (char *)d_stream.next_out - decodeBuffCursor;
	if(bytesInDecodeBuff > 0)
		return 1;
	if(d_stream.avail_in > 0)
		return 1;
	return inStream.in_avail() > 1;
}


