/*
   miniunz.c
   Version 1.1, February 14h, 2010
   sample part of the MiniZip project - ( http://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )
*/

#if (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64
#endif
#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#ifndef _FILE_OFFSET_BIT
#define _FILE_OFFSET_BIT 64
#endif
#endif


#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "contrib/minizip/unzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)

static int
do_extract_currentfile(unzFile uf, char **res, size_t *length) {
    char filename_inzip[256];
    int err = UNZ_OK;
    void *buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), nullptr, 0, nullptr, 0);

    if (err != UNZ_OK) {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n", err);
        return err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void *) malloc(size_buf);
    if (buf == nullptr) {
        printf("Error allocating memory\n");
        free(buf);
        return UNZ_INTERNALERROR;
    }

    err = unzOpenCurrentFilePassword(uf, nullptr);
    if (err != UNZ_OK) {
        printf("error %d with zipfile in unzOpenCurrentFilePassword\n", err);
    }

//    printf(" extracting: %s\n", filename_inzip);

    if (*res != nullptr) {
        printf("%s not null, freeing %p\n", *res);
        free(*res);
    }

    *res = (char *) malloc(file_info.uncompressed_size);
    if (*res == nullptr) {
        printf("malloc failure\n");
        free(*res);
        exit(1);
    }

//    printf("%s res: %p, %s\n", __FUNCTION__, *res, filename_inzip);

    int pos = 0;
    do {
        err = unzReadCurrentFile(uf, buf, size_buf);
        if (err < 0) {
            printf("error %d with zipfile in unzReadCurrentFile\n", err);
            free(buf);
            return 1;
        }

        memcpy(((char *) *res + pos), buf, err);
        pos += err;
    } while (err > 0);

    *length = size_t(file_info.uncompressed_size);
//    printf("%x\n", ((char *) res)[0]);

    free(buf);
    return err;
}

static int do_list(unzFile uf, std::vector<std::string> *res) {
    uLong i;
    unz_global_info64 gi;
    int err;

    err = unzGetGlobalInfo64(uf, &gi);
    if (err != UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo \n", err);

    for (i = 0; i < gi.number_entry; i++) {
        char filename_inzip[256];
        unz_file_info64 file_info;
        err = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), nullptr, 0, nullptr, 0);
        if (err != UNZ_OK) {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n", err);
            break;
        }
        res->push_back(std::string(filename_inzip));


        if ((i + 1) < gi.number_entry) {
            err = unzGoToNextFile(uf);
            if (err != UNZ_OK) {
                printf("error %d with zipfile in unzGoToNextFile\n", err);
                break;
            }
        }
    }

    return err;
}

static int do_extract_onefile(unzFile uf, const char *filename, char **res, size_t *length) {
    if (unzLocateFile(uf, filename, CASESENSITIVITY) != UNZ_OK) {
        printf("file %s not found in the zipfile\n", filename);
        return 2;
    }

    int ret = do_extract_currentfile(uf, res, length);
    if (*res == nullptr) {
        printf("returned null\n");
    }
//    printf("%s res is %p, %d\n", __FUNCTION__, *res, *length);
    return ret;
}


int unzipFiles(const char *zipfilename, std::map<std::string, TextureMapEntry> *result) {
    unzFile uf;
    uf = unzOpen64(zipfilename);
    if (uf == nullptr) {
        free(uf);
        return UNZ_BADZIPFILE;
    }

    std::vector<std::string> filenames;
    do_list(uf, &filenames);

    for (std::string &filename: filenames) {
        std::string lowered;
        for (char &c: filename) {
            lowered += std::tolower(c, std::locale());
        }

        auto parts = split(lowered, "/");
        auto basename = parts.at(parts.size() - 1);

        if (result->count(basename)) {
            // two files with different names in archive
            // BALANCE.BMP and balance.bmp
            if ((*result)[basename].data != nullptr) {
//                printf("%s freeing already existing entry in map %s: %p\n", __PRETTY_FUNCTION__, lowered.c_str(), result[lowered].first);
                free((*result)[basename].data);
            }

            char *data = nullptr;
            size_t length = 0;

            int ret_value = do_extract_onefile(uf, filename.c_str(), &data, &length);
            if (ret_value != UNZ_OK) {
                printf("fail\n");
                return ret_value;
            }


            if (data == nullptr) {
                printf("%s panic\n", __FUNCTION__);
            }

            (*result)[basename].data = data;
            (*result)[basename].len = length;
        }
    }

    int ret = unzClose(uf);
    return ret;
}
