#ifndef RELAX_ISERIALIZER_HDR
#define RELAX_ISERIALIZER_HDR

#include <fstream>
#include <sstream>

class ISerializer {
public:
	virtual ~ISerializer() {
		mSerializerFileStream.flush();
		mSerializerFileStream.close();
	}

	virtual void Serialize(const std::string& fileName) = 0;

protected:
	std::fstream mSerializerFileStream;
};

#endif
