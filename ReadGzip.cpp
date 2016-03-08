//g++ ReadGzip.cpp -lz -o readgzip
#include <zlib.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
using namespace std;
const int READ_BUFF_SIZE = 1024*10;
const int DECODE_BUFF_SIZE = 1024*10;

std::string ConcatStr(const char *a, const char *b)
{
	string out(a);
	out.append(b);
	return out;
}

class DecodeGzip : std::istream
{
public:
	std::istream &inStream;

	DecodeGzip(std::istream &inStream);
	virtual ~DecodeGzip();
};

DecodeGzip::DecodeGzip(std::istream &inStream) : inStream(inStream)
{

}

DecodeGzip::~DecodeGzip()
{

}

ostream& operator<<( ostream& os, const DecodeGzip& obj )
{
	string output;
	string buff;
	buff.resize(READ_BUFF_SIZE);
	obj.inStream.read(&buff[0], READ_BUFF_SIZE);
	int readLen = obj.inStream.gcount();

	z_stream d_stream;
	d_stream.zalloc = (alloc_func)NULL;
	d_stream.zfree = (free_func)NULL;
	d_stream.opaque = (voidpf)NULL;
	d_stream.next_in  = (Bytef*)&buff[0];
	d_stream.avail_in = (uInt)readLen;
	int err = inflateInit2(&d_stream, 16+MAX_WBITS);
	if(err != Z_OK)
		throw runtime_error(ConcatStr("inflateInit2 failed: ", zError(err)));

	char outBuff[DECODE_BUFF_SIZE];
	while(true)
	{
		d_stream.next_out = (Bytef*)outBuff;
		d_stream.avail_out = (uInt)DECODE_BUFF_SIZE;
		err = inflate(&d_stream, Z_NO_FLUSH);
		if (err == Z_STREAM_END) break;
		if(err != Z_OK)
			throw runtime_error(ConcatStr("inflate failed: ", zError(err)));

		output.append(outBuff, d_stream.total_out);

		if(obj.inStream.eof())
			break;

		obj.inStream.read(&buff[0], READ_BUFF_SIZE);
		d_stream.next_in  = (Bytef*)&buff[0];
		int readLen = obj.inStream.gcount();
		d_stream.avail_in = (uInt)readLen;
		if(readLen == 0)
			break;
	}

	err = inflateEnd(&d_stream);
	if(err != Z_OK)
		throw runtime_error(ConcatStr("inflateEnd failed: ", zError(err)));
	output.append(outBuff, d_stream.total_out);

	return os << output;
}

int main()
{
	std::ifstream fi("test.txt.gz");

	cout << DecodeGzip(fi) << endl;
}

