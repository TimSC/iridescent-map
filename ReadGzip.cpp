
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


DecodeGzip::DecodeGzip(std::streambuf &inStream) : inStream(inStream), fs(&inStream)
{
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

	Decode();
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

				CopyToOutputBuffer();
			}
		}
	}

	err = inflate(&d_stream, Z_FINISH);
	if(err != Z_OK && err != Z_STREAM_END)
		throw runtime_error(ConcatStr("inflate failed: ", zError(err)));

	err = inflateEnd(&d_stream);
	if(err != Z_OK)
		throw runtime_error(ConcatStr("inflateEnd failed: ", zError(err)));
	
	CopyToOutputBuffer();
	return true;
}

DecodeGzip::~DecodeGzip()
{

}

void DecodeGzip::CopyToOutputBuffer()
{
	size_t outLen = DECODE_BUFF_SIZE - d_stream.avail_out;
	outBuff.append(decodeBuff, outLen);
	d_stream.next_out = (Bytef*)this->decodeBuff;
	d_stream.avail_out = (uInt)DECODE_BUFF_SIZE;
}

streamsize DecodeGzip::ReturnDataFromOutBuff(char* s, streamsize n)
{
	int lenToCopy = outBuff.size();
	if(n < lenToCopy) lenToCopy = n;
	memcpy(s, outBuff.c_str(), lenToCopy);
	if(lenToCopy > 0)
		outBuff = std::string(&outBuff[lenToCopy], outBuff.size()-lenToCopy);
	return lenToCopy;
}

streamsize DecodeGzip::xsgetn (char* s, streamsize n)
{	
	int err = Z_OK;

	if(outBuff.size() > 0)
		return ReturnDataFromOutBuff(s, n);


	return ReturnDataFromOutBuff(s, n);
}

streamsize DecodeGzip::showmanyc()
{
	if(d_stream.avail_in > 0)
		return 1;
	if(outBuff.size() > 0)
		return 1;
	return inStream.in_avail() > 1;
}


