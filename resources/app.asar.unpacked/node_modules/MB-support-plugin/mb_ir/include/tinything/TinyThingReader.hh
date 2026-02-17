#ifndef TINYTHINGREADER_HH_
#define TINYTHINGREADER_HH_

#include <memory>
#include <string>
#include <vector>

namespace bwcoreutils {
    enum class TOOL : int;
    enum class TYPE : int;
}

namespace LibTinyThing {
    // I feel really badly for anyone who makes the uuid
    // longer than 100 chars and has to troubleshoot why print history
    // isn't working. 
    // The good news is if you've found this, you've probably
    // found what you were looking for. 
    static const unsigned int UUID_MAX_LENGTH = 100;
    // this would be a pretty dang wordy material
    static const unsigned int MATERIAL_MAX_LENGTH = 50;

    // Struct that contains the contents of a metadata file in
    // a version-independent way
    struct TINYTHING_API Metadata {
        Metadata();
        int extruder_count;
        float extrusion_mass_g[2];
        float extrusion_distance_mm[2];
        int extruder_temperature[2];
        int chamber_temperature;
        int shells;
        float layer_height;
        float infill_density;
        bool uses_support;
        float duration_s;
        float max_flow_rate[2];
        uint32_t thing_id;
        bool uses_raft;
        char uuid[UUID_MAX_LENGTH];
        char material[2][MATERIAL_MAX_LENGTH];
        std::string slicer_name;
        bwcoreutils::TYPE tool_type[2];
        unsigned int bot_pid;
        float bounding_box_x_min;
        float bounding_box_x_max;
        float bounding_box_y_min;
        float bounding_box_y_max;
        float bounding_box_z_min;
        float bounding_box_z_max;
        uint32_t file_size;
    };
    // Struct containing all information required to verify whether
    // the TinyThing has been sliced for a given printer
    
    struct TINYTHING_API VerificationData {
        uint8_t tool_count;
        bwcoreutils::TOOL tool_id[2];
        uint8_t pid;
    };

    // Struct containing only that data that doesn't absolutely suck
    // to export through a C interface
    struct TINYTHING_API CInterfaceMetadata {
        int extruder_count;
        float extrusion_distance_mm[2];
        int extruder_temperature[2];
        float extrusion_mass_g[2];
        int chamber_temperature;
        uint32_t thing_id;
        float duration_s;
        bool uses_raft;
        char uuid[UUID_MAX_LENGTH];
        char material[2][MATERIAL_MAX_LENGTH];
        bwcoreutils::TYPE tool_type[2];
        unsigned int bot_pid;
        float bounding_box_x_min;
        float bounding_box_x_max;
        float bounding_box_y_min;
        float bounding_box_y_max;
        float bounding_box_z_min;
        float bounding_box_z_max;
        uint32_t file_size;
    };
    
    class TINYTHING_API TinyThingReader {
    public:
        enum Error {
            kOK=0,
            kNotYetUnzipped=1,
            kToolMismatch=2,
            kBotTypeMismatch=3,
            kVersionMismatch=4,
            kMaxStringLengthExceeded=5
        };
        
        // passing in a fd will use an already opened file handle
        TinyThingReader(const std::string& filePath, int fd = -1);
        ~TinyThingReader();

        // these functions unzip the contents of each of these files,
        // and cache them in memory. they return true if the unzip is
        // succesful
        bool unzipMetadataFile();
        bool unzipIsometricSmallThumbnailFile();
        bool unzipIsometricMediumThumbnailFile();
        bool unzipIsometricLargeThumbnailFile();
        bool unzipSmallThumbnailFile();
        bool unzipMediumThumbnailFile();
        bool unzipLargeThumbnailFile();
        bool unzipSombreroSmallThumbnailFile();
        bool unzipSombreroMediumThumbnailFile();
        bool unzipSombreroLargeThumbnailFile();
        bool unzipSmallSquareThumbnailFile();
        bool unzipToolpathFile();

        // these are accessors for the unzipped contents of each of the
        // files. they should only be called after each has been
        // unzipped.
        void getIsometricSmallThumbnailFileContents(std::string* contents) const;
        void getIsometricMediumThumbnailFileContents(std::string* contents) const;
        void getIsometricLargeThumbnailFileContents(std::string* contents) const;
        void getSmallThumbnailFileContents(std::string* contents) const;
        void getMediumThumbnailFileContents(std::string* contents) const;
        void getLargeThumbnailFileContents(std::string* contents) const;
        void getSombreroSmallThumbnailFileContents(std::string* contents) const;
        void getSombreroMediumThumbnailFileContents(std::string* contents) const;
        void getSombreroLargeThumbnailFileContents(std::string* contents) const;
        void getSmallSquareThumbnailFileContents(std::string* contents) const;
        void getToolpathFileContents(std::string* contents) const;
        std::string getToolpathFileContents() const;
        // checks to see if toolpath is present
        bool hasJsonToolpath() const;
        bool hasGcodeToolpath() const;

        // checks to see if all files are present
        bool isValid() const;

        // checks to see if specifically metadata is present - this is
        // a hack required by DRM prints, where the metadata is downloaded
        // independently and stuck in a zipfile, which is then opened by
        // printerpanel like it was a full .makerbot.
        bool hasMetadata() const;

        // check to see if the metadata file is acceptable given
        // a config (defined above)
        Error doesMetadataFileMatchConfig(const VerificationData& config) const;
        Error getMetadata(Metadata* out) const;
        Error getMetadata(CInterfaceMetadata* out) const;
        // Methods to get the slice profile, as a cstring (will be in the value
        // pointed to by the out arg). This pointer is a pointer to a buffer
        // held by the TinyThingReader object; it is no longer valid if this
        // instance dies. it is likely a good idea to copy it out to somewhere
        // under the caller's control as soon as possible.
        Error getSliceProfile(const char** profile) const;

        // Hacky method to support MBD doing its own metadata parsing for some
        // reason
        std::string getMetadataFileContents() const;
        // incremental unzipping of toolpath
        // unzipping any other part of the file
        // or checking if it is valid will reset
        // the incremental pointer
        bool resetToolpath();
        std::string getToolpathIncr(const int chars);
        // just like getToolpathIncr, but put the data into buff
        // read at most chars number of bytes
        // return actual number of characters read
        // in case of error, return 0 and do not bodify buff
        int getToolpathIncr(char* buff, int chars);

    private:
        class Private;
        std::unique_ptr<Private> m_private;
    };

}

#endif /* TINYTHINGREADER_HH_ */
