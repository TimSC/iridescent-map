//Decoding gzip streams in C++ based on a streambuf based class
//g++ ReadGzip.cpp -lz -o readgzip
#include <zlib.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <string.h>
using namespace std;
const size_t READ_BUFF_SIZE = 1024*128;
const size_t DECODE_BUFF_SIZE = 1024*128;

std::string ConcatStr(const char *a, const char *b)
{
	string out(a);
	out.append(b);
	return out;
}

class DecodeGzip : public streambuf
{
protected:
	char *readBuff;
	char *decodeBuff;
	streambuf &inStream;
	std::iostream fs;
	z_stream d_stream;
	char *outCursor;
	size_t readBufferSize;
	size_t decodeBufferSize;

	streamsize ReturnDataFromOutBuff(char* s, streamsize n);

public:
	DecodeGzip(std::streambuf &inStream, 
		size_t readBufferSize = READ_BUFF_SIZE, 
		size_t decodeBufferSize = DECODE_BUFF_SIZE);
	virtual ~DecodeGzip();
	streamsize xsgetn (char* s, streamsize n);
	streamsize showmanyc();
};

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
	outCursor = this->decodeBuff;

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

streamsize DecodeGzip::ReturnDataFromOutBuff(char* s, streamsize n)
{
	int lengthInBuff = (char *)d_stream.next_out - outCursor;
	int lenToCopy = lengthInBuff;
	if(n < lenToCopy) lenToCopy = n;
	
	strncpy(s, outCursor, lenToCopy);
	outCursor += lenToCopy;
	if(lengthInBuff == lenToCopy)
	{
		//Clear buffer
		d_stream.next_out = (Bytef*)this->decodeBuff;
		d_stream.avail_out = (uInt)decodeBufferSize;
		outCursor = this->decodeBuff;
	}
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
			return ReturnDataFromOutBuff(s, n);
		}
	}

	err = inflateEnd(&d_stream);
	if(err != Z_OK)
		throw runtime_error(ConcatStr("inflateEnd failed: ", zError(err)));	
	return ReturnDataFromOutBuff(s, n);
}

streamsize DecodeGzip::showmanyc()
{
	if(d_stream.avail_in > 0)
		return 1;
	int lengthInBuff = (char *)d_stream.next_out - outCursor;
	if(lengthInBuff > 0)
		return 1;
	return inStream.in_avail() > 1;
}

void Test(streambuf &st)
{
	int testBuffSize = 200;
	char buff[testBuffSize];
	ofstream testOut("testout.txt");
	while(st.in_avail()>0)
	{
		int len = st.sgetn(buff, testBuffSize-1);
		buff[len] = '\0';
		cout << buff;
		testOut << buff;
	}
	testOut.flush();
}

int main()
{
	std::filebuf fb;
	fb.open("test2.txt.gz", std::ios::in);
	class DecodeGzip decodeGzip(fb);

	Test(decodeGzip);
}

