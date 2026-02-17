namespace LibTinyThing {

class TinyThingReader;
class TinyThingWriter;
struct VerificationData;
struct CInterfaceMetadata;

// READER FUNCTIONS

extern "C" {
TINYTHING_API TinyThingReader* NewTinyThingReader(const char* filepath, int fd);

TINYTHING_API int DestroyTinyThingReader(TinyThingReader* tinything);

TINYTHING_API bool UnzipMetadata(TinyThingReader* tinything);

TINYTHING_API int DoesMetadataMatch(const TinyThingReader* tinything,
                                    const VerificationData* data);

TINYTHING_API int GetMetadata(const TinyThingReader* tinything,
                              CInterfaceMetadata* out);

TINYTHING_API int GetSliceProfile(const TinyThingReader* tinything,
                                  const char** out);

TINYTHING_API bool IsValid(const TinyThingReader* tinything);

// WRITER FUNCTIONS

TINYTHING_API TinyThingWriter* NewTinyThingWriter(const char* filepath);

TINYTHING_API int DestroyTinyThingWriter(TinyThingWriter* tinything);

TINYTHING_API void SetToolpathFile(TinyThingWriter* tinything, const char* filepath);

TINYTHING_API void SetThumbnailDirectory(TinyThingWriter* tinything, const char* filedir);

TINYTHING_API void SetMetadataFile(TinyThingWriter* tinything, const char* filepath);

TINYTHING_API bool Zip(TinyThingWriter* tinything);
}

}
