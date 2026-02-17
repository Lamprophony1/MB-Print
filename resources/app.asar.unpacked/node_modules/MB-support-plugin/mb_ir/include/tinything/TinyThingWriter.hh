
#ifndef TINYTHINGWRITER_HH_
#define TINYTHINGWRITER_HH_

#include <memory>
#include <string>

namespace LibTinyThing {

	class TINYTHING_API TinyThingWriter {
	public:
		TinyThingWriter(const std::string& filePath);

		~TinyThingWriter();

		void setThumbnailDirectory(const std::string& dirPath);

		void setToolpathFile(const std::string& filePath);

		void setMetadataFile(const std::string& filePath);

		bool zip();

	private:
        class Private;
        std::unique_ptr<Private> m_private;
	};
}


#endif /* TINYTHINGWRITER_HH_ */




