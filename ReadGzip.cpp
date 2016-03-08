//g++ ReadGzip.cpp -lz -o readgzip
#include <zlib.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <string.h>
using namespace std;
const int READ_BUFF_SIZE = 1024*10;
const int DECODE_BUFF_SIZE = 1024*100;

std::string ConcatStr(const char *a, const char *b)
{
	string out(a);
	out.append(b);
	return out;
}

class DecodeGzip : public streambuf
{
protected:
	char readBuff[READ_BUFF_SIZE];
	char decodeBuff[DECODE_BUFF_SIZE];
	streambuf &inStream;
	std::iostream fs;
	z_stream d_stream;

public:
	DecodeGzip(std::streambuf &inStream);
	virtual ~DecodeGzip();
	streamsize xsgetn (char* s, streamsize n);
};

DecodeGzip::DecodeGzip(std::streambuf &inStream) : inStream(inStream), fs(&inStream)
{
	fs.read(this->readBuff, READ_BUFF_SIZE);

	d_stream.zalloc = (alloc_func)NULL;
	d_stream.zfree = (free_func)NULL;
	d_stream.opaque = (voidpf)NULL;
	d_stream.next_in  = (Bytef*)this->readBuff;
	d_stream.avail_in = (uInt)fs.gcount();
	//cout << "read " << d_stream.avail_in << endl;
	int err = inflateInit2(&d_stream, 16+MAX_WBITS);
	if(err != Z_OK)
		throw runtime_error(ConcatStr("inflateInit2 failed: ", zError(err)));

}

DecodeGzip::~DecodeGzip()
{

}

streamsize DecodeGzip::xsgetn (char* s, streamsize n)
{	
	int err = Z_OK;
	string output;

	while(true)
	{
		d_stream.next_out = (Bytef*)this->decodeBuff;
		d_stream.avail_out = (uInt)DECODE_BUFF_SIZE;
		err = inflate(&d_stream, Z_NO_FLUSH);
		if (err == Z_STREAM_END) break;
		if(err != Z_OK)
			throw runtime_error(ConcatStr("inflate failed: ", zError(err)));

		output.append(this->decodeBuff, DECODE_BUFF_SIZE - d_stream.avail_out);

		if(fs.eof())
			break;

		fs.read(this->readBuff, READ_BUFF_SIZE);
		d_stream.next_in  = (Bytef*)this->readBuff;
		d_stream.avail_in = (uInt)fs.gcount();
		//cout << "read " << d_stream.avail_in << endl;
		if(d_stream.avail_in == 0)
			break;
	}

	d_stream.avail_in = (uInt)0;
	err = inflate(&d_stream, Z_FINISH);
	if(err != Z_OK && err != Z_STREAM_END)
		throw runtime_error(ConcatStr("inflate failed: ", zError(err)));
	output.append(this->decodeBuff, DECODE_BUFF_SIZE - d_stream.avail_out);

	err = inflateEnd(&d_stream);
	if(err != Z_OK)
		throw runtime_error(ConcatStr("inflateEnd failed: ", zError(err)));
	
	size_t outLen = output.size();
	if(n < outLen) outLen = n;
	strncpy(s, output.c_str(), outLen);
	return outLen;
}

void Test(streambuf &st)
{
	char buff[READ_BUFF_SIZE];
	st.sgetn(buff, READ_BUFF_SIZE);
	cout << buff << endl;
}

int main()
{
	std::filebuf fb;
	fb.open("test.txt.gz", std::ios::in);
	class DecodeGzip decodeGzip(fb);

	Test(decodeGzip);
}

