
#ifndef TINYTHINGCONSTANTS_HH_
#define TINYTHINGCONSTANTS_HH_

#include <string>

namespace LibTinyThing {
    struct TINYTHING_API Config {
        static const std::string kMetadataFilename;
        static const std::string kJsonToolpathFilename;
        static const std::string kGcodeToolpathFilename;

        static const std::string kIsometricLargeThumbnailFilename;
        static const std::string kIsometricMediumThumbnailFilename;
        static const std::string kIsometricSmallThumbnailFilename;

        static const std::string kLargeThumbnailFilename;
        static const std::string kMediumThumbnailFilename;
        static const std::string kSmallThumbnailFilename;

        static const std::string kSombreroLargeThumbnailFilename;
        static const std::string kSombreroMediumThumbnailFilename;
        static const std::string kSombreroSmallThumbnailFilename;

        static const std::string kSmallSquareThumbnailFilename;

        static const int kIsometricLargeThumbnailWidth;
        static const int kIsometricMediumThumbnailWidth;
        static const int kIsometricSmallThumbnailWidth;

        static const int kIsometricLargeThumbnailHeight;
        static const int kIsometricMediumThumbnailHeight;
        static const int kIsometricSmallThumbnailHeight;

        static const int kLargeThumbnailWidth;
        static const int kMediumThumbnailWidth;
        static const int kSmallThumbnailWidth;

        static const int kLargeThumbnailHeight;
        static const int kMediumThumbnailHeight;
        static const int kSmallThumbnailHeight;

        static const int kSombreroLargeThumbnailWidth;
        static const int kSombreroMediumThumbnailWidth;
        static const int kSombreroSmallThumbnailWidth;

        static const int kSombreroLargeThumbnailHeight;
        static const int kSombreroMediumThumbnailHeight;
        static const int kSombreroSmallThumbnailHeight;

        static const int kSmallSquareThumbnailWidth;
        static const int kSmallSquareThumbnailHeight;
	};
}

#endif /* TINYTHINGCONSTANTS_HH_ */

