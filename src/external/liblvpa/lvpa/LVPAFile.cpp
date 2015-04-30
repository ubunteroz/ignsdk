#include "LVPAInternal.h"
#include "LVPAFile.h"
#include "LVPATools.h"

#include <memory>
#include <set>

#include "MersenneTwister.h"
#include "MyCrc32.h"
#include "LVPAStreamCipher.h"
#include "SHA256Hash.h"
#include "ProgressBar.h"

#include "ICompressor.h"

#ifdef LVPA_SUPPORT_LZMA
#  include "LZMACompressor.h"
#endif
#ifdef LVPA_SUPPORT_LZO
#  include "LZOCompressor.h"
#endif
#ifdef LVPA_SUPPORT_ZLIB
#  include "DeflateCompressor.h"
#endif
#ifdef LVPA_SUPPORT_LZF
#  include "LZFCompressor.h"
#endif
#ifdef LVPA_SUPPORT_LZHAM
#  include "LZHAMCompressor.h"
#endif

LVPA_NAMESPACE_START

// not the best way...
static ProgressBar *gProgress = NULL;

// these are part of the header of each file
static const char* gMagic = LVPA_MAGIC;
static const uint32 gVersion = LVPA_VERSION;


static bool default_open(const char *fn, void *opaque)
{
    LVPAFileReader *rd = (LVPAFileReader*)opaque;
    if(!rd->io)
        rd->io = fopen(fn, "rb");
    return !!rd->io;
}


static size_t default_read(void *opaque, void *ptr, size_t rpos, size_t bytes)
{
    LVPAFileReader *rd = (LVPAFileReader*)opaque;
    FILE *f = (FILE*)rd->io;
    size_t pos = ftell(f);
    if(pos != rpos)
        if(fseek(f, rpos, SEEK_SET) != 0)
            return 0;

    return fread(ptr, 1, bytes, f);
}

static void default_close(void *opaque)
{
    LVPAFileReader *rd = (LVPAFileReader*)opaque;
    if(rd->io)
    {
        fclose((FILE*)rd->io);
        rd->io = NULL;
    }
}

static void initDefaultFileReader(LVPAFileReader *rd)
{
    memset(rd, 0, sizeof(LVPAFileReader)); // just in case
    rd->opaque = rd;
    rd->closeF = &default_close;
    rd->openF = &default_open;
    rd->readF = &default_read;
}

// TODO: this does not belong here! Move to lvpak!
static int drawCompressProgressBar(void *, uint64 in, uint64 out)
{
    if(gProgress)
    {
        gProgress->done = uint32(in) / 1024; // show in kB
        gProgress->Update();
    }
    return 0; // SZ_OK
}

static ICompressor *allocCompressor(uint8 algo)
{
    switch(algo)
    {
        case LVPAPACK_INHERIT: // we choose the first available one as default
                               // the rest is roughly sorted by compression speed

#ifdef LVPA_SUPPORT_LZF
        case LVPAPACK_LZF:
            return new LZFCompressor;
#endif
#ifdef LVPA_SUPPORT_LZO
        case LVPAPACK_LZO1X:
            return new LZOCompressor;
#endif
#ifdef LVPA_SUPPORT_ZLIB
        case LVPAPACK_DEFLATE:
            return new DeflateCompressor;
#endif
#ifdef LVPA_SUPPORT_LZMA
        case LVPAPACK_LZMA:
            return new LZMACompressor;
#endif
#ifdef LVPA_SUPPORT_LZHAM
        case LVPAPACK_LZHAM:
            return new LZHAMCompressor;
#endif
        case LVPAPACK_NONE:
            return new ICompressor; // does nothing
    }

    logerror("allocCompressor(): unsupported algorithm id: %u", uint32(algo));
    return NULL;
}


ByteBuffer &operator >> (ByteBuffer& bb, LVPAMasterHeader& hdr)
{
    bb >> hdr.version;
    bb >> hdr.flags;
    bb >> hdr.hdrEntries;
    bb >> hdr.packedHdrSize;
    bb >> hdr.realHdrSize;
    bb >> hdr.hdrOffset;
    bb >> hdr.hdrCrcPacked;
    bb >> hdr.hdrCrcReal;
    bb >> hdr.algo;
    bb >> hdr.dataOffs;
    return bb;
}

ByteBuffer &operator << (ByteBuffer& bb, LVPAMasterHeader& hdr)
{
    bb << hdr.version;
    bb << hdr.flags;
    bb << hdr.hdrEntries;
    bb << hdr.packedHdrSize;
    bb << hdr.realHdrSize;
    bb << hdr.hdrOffset;
    bb << hdr.hdrCrcPacked;
    bb << hdr.hdrCrcReal;
    bb << hdr.algo;
    bb << hdr.dataOffs;
    return bb;
}

ByteBuffer &operator >> (ByteBuffer& bb, LVPAFileHeader& h)
{
    bb >> h.flags;
    bb >> h.realSize;
    bb >> h.crcReal;

    if(h.flags & LVPAFLAG_SCRAMBLED)
        bb.read(h.hash, LVPAHash_Size);
    else
        bb >> h.filename;

    if(h.flags & LVPAFLAG_PACKED)
    {
        bb >> h.packedSize;
        bb >> h.crcPacked;
        bb >> h.algo;
        bb >> h.level;
    }
    else
    {
        h.packedSize = h.realSize; // this is important, because it will read packedSize bytes from the file
        h.crcPacked = 0;
        h.algo = LVPAPACK_NONE;
        h.level = LVPACOMP_NONE;
    }

    if(h.flags & LVPAFLAG_SOLID)
    {
        bb >> h.blockId;
    }
    else
    {
        h.blockId = 0;
    }

    if(h.flags & (LVPAFLAG_ENCRYPTED | LVPAFLAG_SCRAMBLED))
    {
        bb >> h.cipherWarmup;
    }
    else
    {
        h.cipherWarmup = 0;
    }

    return bb;
}

ByteBuffer &operator << (ByteBuffer& bb, LVPAFileHeader& h)
{
    bb << h.flags;
    bb << h.realSize;
    bb << h.crcReal;

    if(h.flags & LVPAFLAG_SCRAMBLED)
        bb.append(h.hash, LVPAHash_Size);
    else
        bb << h.filename;

    if(h.flags & LVPAFLAG_PACKED)
    {
        bb << h.packedSize;
        bb << h.crcPacked;
        bb << h.algo;
        bb << h.level;
    }

    if(h.flags & LVPAFLAG_SOLID)
    {
        bb << h.blockId;
    }

    if(h.flags & (LVPAFLAG_ENCRYPTED | LVPAFLAG_SCRAMBLED))
        bb << h.cipherWarmup;

    return bb;
}


LVPAFile::LVPAFile()
: _realSize(0), _packedSize(0)
{
    _mtrand = new MTRand;
    initDefaultFileReader(&reader);
}

LVPAFile::~LVPAFile()
{
    delete _mtrand;
    Clear();
    _CloseFile();
}

void LVPAFile::Clear(bool del /* = true */)
{
    for(uint32 i = 0; i < _headers.size(); ++i)
    {
        // never try to delete files that are part of a bigger allocated block
        if(_headers[i].data.ptr && !_headers[i].otherMem)
        {
            // we can always delete solid blocks, because memory for these is allocated only when loaded
            // from file, and it can never be constant
            if(del || (_headers[i].flags & LVPAFLAG_SOLIDBLOCK))
            {
                delete [] _headers[i].data.ptr;
                _headers[i].data.ptr = NULL;
                _headers[i].data.size = 0;
            }
        }
    }
    _headers.clear();
    _indexes.clear();
}

bool LVPAFile::_OpenFile(void)
{
    return reader.open(_ownName.c_str());
}

void LVPAFile::_CloseFile(void)
{
   reader.close();
}

void LVPAFile::Close(void)
{
    _CloseFile();
}

uint32 LVPAFile::GetId(const char *fn)
{
    uint32 id = -1;
    _FindHeaderByName(fn, &id);
    return id;
}

void LVPAFile::_MakeSolid(LVPAFileHeader& h, const char *solidBlockName)
{
    if(!solidBlockName)
        return;
    if(h.flags & LVPAFLAG_SOLIDBLOCK)
    {
        DEBUG(logerror("_MakeSolid: Can't make solid block a solid file!"));
    }
    std::string n(solidBlockName);
    n += '*';

    h.flags |= LVPAFLAG_SOLID;
    if(h.flags & LVPAFLAG_SCRAMBLED)
    {
        DEBUG(logerror("_MakeSolid: Solid file '%s' has SCRAMBLED flag, removing that", h.filename.c_str()));
    }
    h.flags &= ~LVPAFLAG_SCRAMBLED; // can't have that flag in this mode, because can't encrypt based on filename inside solid block

    // this may enlarge _headers vector, thus forcing reallocation and maybe making our h reference invalid
    // but since this method is only called from Add(), and there we take care of reserving enough space, this is ok.
    if(!_FindHeaderByName(n.c_str(), &h.blockId)) // if successful, it will write properly into h.blockId
    {
        h.blockId = SetSolidBlock(solidBlockName); // create not existing with default properties
    }

    // if at least one file inside the solid block must be encrypted, encrypt the whole solid block
    _headers[h.blockId].flags |= (h.flags & LVPAFLAG_ENCRYPTED);
}

uint32 LVPAFile::SetSolidBlock(const char *name, uint8 compression /* = LVPACOMP_INHERIT */, uint8 algo /* = LVPAPACK_INHERIT */)
{
    std::string n(name);
    n += '*';
    uint32 id;
    if(_FindHeaderByName(n.c_str(), &id))
    {
        _headers[id].algo = algo;
        _headers[id].level = compression;
    }
    else // add new solid block
    {
        LVPAFileHeader hdr;
        hdr.flags = LVPAFLAG_SOLIDBLOCK;
        hdr.filename = n;
        hdr.algo = algo;
        hdr.level = compression;
        id = hdr.id = _headers.size();
        _headers.push_back(hdr);
        _indexes[n] = id; // save the index of the hdr we just added
    }
    return id;
}

void LVPAFile::Add(const char *fn, memblock mb, const char *solidBlockName /* = NULL */,
                   uint8 algo /* = LVPAPACK_INHERIT */, uint8 level /* = LVPACOMP_INHERIT */,
                   uint8 encrypt /* = LVPAENCR_INHERIT */, bool scramble /* = false */)
{
    uint32 id = -1;
    if(_FindHeaderByName(fn, &id))
    {
        // already exists, overwrite old with new info
        LVPAFileHeader& hdrRef = _headers[id];
        if(hdrRef.data.ptr && hdrRef.data.ptr != mb.ptr)
        {
            delete [] hdrRef.data.ptr;
            hdrRef.otherMem = false;
            // will be overwritten anyways, not necessary here to set to null values
        }
    }
    else
    {
        // create new header entry
        LVPAFileHeader hdr;
        // save the index it will have after adding
        hdr.id = id = _headers.size();
        _indexes[fn] = id;
        _headers.push_back(hdr);
    }
    DEBUG(ASSERT(id != uint32(-1)));

    // Always reserve a little more space, it would be bad if the vector had to reallocate
    // while we are holding references to its elements.
    // That can happen if a new solid block header entry has to be added in _MakeSolid(), and the vector is out of space.
    _headers.reserve(_headers.size() + 2);

    LVPAFileHeader& h = _headers[id];

    h.filename = fn;
    h.data = mb;
    h.flags = scramble ? LVPAFLAG_SCRAMBLED : LVPAFLAG_NONE;
    h.encryption = encrypt;
    h.algo = algo;
    h.level = level;
    _MakeSolid(h, solidBlockName); // this will also fix up flags a bit if necessary
}

memblock LVPAFile::Remove(const char  *fn)
{
    uint32 id;
    memblock mb;
    if(_FindHeaderByName(fn, &id))
        mb = _headers[id].data; // copy ptr

    _headers[id].data = memblock(); // overwrite with empty
    _indexes.erase(fn); // remove entry

    return mb;
}

bool LVPAFile::Delete(const char *fn)
{
    uint32 id;
    if(_FindHeaderByName(fn, &id))
    {
        memblock mb = _headers[id].data;
        _headers[id].data = memblock(); // overwrite with empty
        _indexes.erase(fn); // remove entry

        if(mb.ptr && !_headers[id].otherMem)
        {
            delete [] mb.ptr;
            return true;
        }
    }
    return false;
}

memblock LVPAFile::Get(const char *fn, bool checkCRC /* = true */)
{
    uint32 id;
    memblock mb;
    if(_FindHeaderByName(fn, &id))
        mb = _PrepareFile(_headers[id], checkCRC);
    return mb;
}

memblock LVPAFile::Get(uint32 index, bool checkCRC /* = true */)
{
    return _PrepareFile(_headers[index], checkCRC);
}

bool LVPAFile::Free(const char *fn)
{
    uint32 id;
    if(_FindHeaderByName(fn, &id))
        return Free(id);
    return true; // file never existed, no problem
}

bool LVPAFile::Free(uint32 id)
{
    LVPAFileHeader& hdrRef = _headers[id];
    bool freed = false;
    if(hdrRef.data.ptr && !hdrRef.otherMem)
    {
        delete [] hdrRef.data.ptr;
        freed = true;
    }

    if(freed || !(hdrRef.flags & LVPAFLAG_SOLIDBLOCK))
    {
        hdrRef.data.ptr = NULL; // can be retrieved easily if needed later
        hdrRef.data.size = 0;
    }

    hdrRef.sparePtr = NULL; // now its definitely not used anymore.
    return freed;
}

uint32 LVPAFile::FreeUnused(void)
{
    uint32 freed = 0;
    std::set<uint8*> ptrsInUse;
    // first, collect all pointers from files inside a solid block
    for(uint32 i = 0; i < HeaderCount(); ++i)
    {
        LVPAFileHeader& h = _headers[i];
        if(h.flags & LVPAFLAG_SOLID)
        {
            // inserting NULL is not a problem
            ptrsInUse.insert(_headers[i].data.ptr);
            ptrsInUse.insert(_headers[i].sparePtr);
        }
    }
    std::set<uint8*>::iterator it;
    for(uint32 i = 0; i < HeaderCount(); ++i)
    {
        LVPAFileHeader& h = _headers[i];
        if(h.flags & LVPAFLAG_SOLIDBLOCK && h.data.ptr) // because we check h.data.ptr here, *it below can be NULL without causing trouble
        {
            bool used = false;
            for(it = ptrsInUse.begin(); it != ptrsInUse.end(); ++it)
            {
                uint8 *ptr = *it;
                if(ptr >= h.data.ptr && ptr < h.data.ptr + h.data.size) // pointer inside solid block memory?
                {
                    used = true;
                    break;
                }
            }
            if(!used)
                freed += Free(h.id);
        }
    }
    return freed;
}


bool LVPAFile::Drop(const char *fn)
{
    uint32 id;
    if(_FindHeaderByName(fn, &id))
        return Drop(id);
    return true; // the file isn't there, no problem.
}

bool LVPAFile::Drop(uint32 id)
{
    LVPAFileHeader& hdrRef = _headers[id];
    if(hdrRef.data.ptr)
        hdrRef.sparePtr = hdrRef.data.ptr;

    hdrRef.data.ptr = NULL;
    hdrRef.data.size = 0;

    return !hdrRef.otherMem;
}

bool LVPAFile::LoadFrom(const char *fn, LVPAFileReader *rd /* = NULL */)
{
    _ownName = fn;

    if(rd)
        reader = *rd;
    else
        initDefaultFileReader(&reader);

    if(!_OpenFile())
        return false;

    Clear();

    uint32 bytes;
    char magic[4];

    bytes = reader.read(magic, 4);
    if(bytes != 4 || memcmp(magic, gMagic, 4))
    {
        _CloseFile();
        return false;
    }

    ByteBuffer masterBuf;
    LVPAMasterHeader masterHdr;

    masterBuf.resize(sizeof(LVPAMasterHeader));
    bytes = reader.read((void*)masterBuf.contents(), sizeof(LVPAMasterHeader));
    masterBuf >> masterHdr; // not reading it directly via fread() is intentional

    DEBUG(logdebug("master: version: %u", masterHdr.version));
    DEBUG(logdebug("master: flags: %u", masterHdr.flags));

    if(masterHdr.version != gVersion)
    {
        logerror("Unsupported LVPA file version: %u", masterHdr.version);
        _CloseFile();
        return false;
    }

    if(masterHdr.flags & LVPAHDR_ENCRYPTED && !_masterKey.size())
    {
        logerror("Headers are encrypted, but no key set, can't read!");
        _CloseFile();
        return false;
    }

    // decrypt if required
    LVPACipher hdrCiph;
    if(masterHdr.flags & LVPAHDR_ENCRYPTED)
    {
        if(_masterKey.size())
            hdrCiph.Init(&_masterKey[0], _masterKey.size());
        hdrCiph.WarmUp(LVPA_HDR_CIPHER_WARMUP);
        uint32 ox = offsetof(LVPAMasterHeader, packedHdrSize);

        // dumb: due to padding, masterBuf is probably a bit too large, so correct size
        masterBuf.resize(1);
        masterBuf.wpos(0);
        masterBuf.rpos(0);
        masterBuf << masterHdr;

        hdrCiph.Apply((uint8*)masterBuf.contents() + ox, masterBuf.size() - ox);
        masterBuf.rpos(0);
        masterBuf >> masterHdr; // read again
    }

    DEBUG(logdebug("master: data offset: %u", masterHdr.dataOffs));
    DEBUG(logdebug("master: header offset: %u", masterHdr.hdrOffset));
    DEBUG(logdebug("master: header entries: %u", masterHdr.hdrEntries));
    DEBUG(logdebug("master: packed header crc: %X", masterHdr.hdrCrcPacked));
    DEBUG(logdebug("master: unpacked header crc: %X", masterHdr.hdrCrcReal))
    DEBUG(logdebug("master: packed size: %u", masterHdr.packedHdrSize));
    DEBUG(logdebug("master: real size: %u", masterHdr.realHdrSize));

    // sanity check
    if( !(masterHdr.hdrEntries && masterHdr.packedHdrSize && masterHdr.realHdrSize) )
    {
        logerror("Can't read headers, file contains no valid data");
        _CloseFile();
        return false;
    }

    // ... space for additional data/headers here...

    // seek to the file header's offset if we are not yet there
    reader.seek(masterHdr.hdrOffset);

    std::auto_ptr<ICompressor> hdrBuf(allocCompressor(masterHdr.algo));

    if(!hdrBuf.get())
    {
        logerror("Unable to decompress headers, wrong encryption key?");
        _CloseFile();
        return false;
    }


    // read the (packed) file headers
    hdrBuf->resize(masterHdr.packedHdrSize);
    bytes = reader.read((void*)hdrBuf->contents(), masterHdr.packedHdrSize);
    if(bytes != masterHdr.packedHdrSize)
    {
        logerror("Can't read headers, file is corrupt");
        _CloseFile();
        return false;
    }

    // decrypt if necessary
    if(masterHdr.flags & LVPAHDR_ENCRYPTED)
    {
        hdrCiph.Apply((uint8*)hdrBuf->contents(), hdrBuf->size());
    }

    // decompress the headers if packed
    if(masterHdr.flags & LVPAHDR_PACKED)
    {
        // check CRC of packed header data
        if(CRC32::Calc((uint8*)hdrBuf->contents(), hdrBuf->size()) != masterHdr.hdrCrcPacked)
        {
            logerror("CRC mismatch, packed header is damaged");
            _CloseFile();
            return false;
        }

        hdrBuf->Compressed(true); // tell the buf that it is compressed so it will allow decompression
        hdrBuf->RealSize(masterHdr.realHdrSize);
        hdrBuf->Decompress();
    }

    // check CRC of unpacked header data
    if(CRC32::Calc((uint8*)hdrBuf->contents(), hdrBuf->size()) != masterHdr.hdrCrcReal)
    {
        logerror("CRC mismatch, unpacked header is damaged");
        _CloseFile();
        return false;
    }

    _realSize = _packedSize = 0;

    // read the headers
    _headers.resize(masterHdr.hdrEntries);
    uint32 dataStartOffs = masterHdr.dataOffs;
    uint32 solidOffs = 0;
    for(uint32 i = 0; i < masterHdr.hdrEntries; ++i)
    {
        LVPAFileHeader &h = _headers[i];
        *hdrBuf >> h;
        h.good = true;

        DEBUG(logdebug("'%s' bytes: %u; blockId: %u; [%s%s%s%s%s]",
            h.filename.c_str(), h.packedSize, h.blockId,
            (h.flags & LVPAFLAG_PACKED) ? "PACKED " : "",
            (h.flags & LVPAFLAG_SOLID) ? "SOLID " : "",
            (h.flags & LVPAFLAG_SOLIDBLOCK) ? "SBLOCK " : "",
            (h.flags & LVPAFLAG_ENCRYPTED) ? "ENCR " : "",
            (h.flags & LVPAFLAG_SCRAMBLED) ? "SCRAM " : ""
            ));

        // sanity check - can't be in a solid block and a solid block itself
        if((h.flags & LVPAFLAG_SOLID) &&( h.flags & LVPAFLAG_SOLIDBLOCK))
        {
            h.good = false;
            logerror("File '%s' has wrong/incompatible flags, whoops!", h.filename.c_str());
            _CloseFile();
            return false;
        }

        // for stats -- do not account files inside a solid block, because the solid block is likely packed, not the individual files
        if(!(h.flags & LVPAFLAG_SOLID))
            _packedSize += h.packedSize;
        // -- here, do not account solid blocks, because the individual files' real size matters
        if(!(h.flags & LVPAFLAG_SOLIDBLOCK))
            _realSize += h.realSize;
    }

    // at this point we have processed all headers
    _CreateIndexes();
    _CalcOffsets(masterHdr.dataOffs);

    // leave the file open, as we may want to read more later on

    return true;
}

bool LVPAFile::Save(LVPAComprLevels compression, LVPAAlgos algo /* = LVPAPACK_INHERIT */, bool encrypt /* = false */)
{
    return SaveAs(_ownName.c_str(), compression, algo, encrypt);
}

// note: this function must NOT modify existing headers in memory!
bool LVPAFile::SaveAs(const char *fn, LVPAComprLevels compression /* = LVPA_DEFAULT_LEVEL */, LVPAAlgos algo /* = LVPAPACK_INHERIT */,
                      bool encrypt /* = false */)
{
    // check before showing progress bar
    if(!_headers.size())
    {
        logerror("No files to write to '%s'", fn);
        return false;
    }

    // apply default settings for INHERIT modes
    if(compression == LVPACOMP_INHERIT)
        compression = LVPA_DEFAULT_LEVEL;

    std::auto_ptr<ICompressor> zhdr(allocCompressor(algo));
    if(!zhdr.get())
    {
        logerror("Unknown compression method '%u'", (uint32)algo);
        return false;
    }

    if(encrypt && _masterKey.empty())
    {
        logwarn("Archive should be encrypted, but no master key - not encrypting.");
        encrypt = false;
    }

    zhdr->reserve(_headers.size() * (sizeof(LVPAFileHeader) + 30)); // guess size

    _realSize = _packedSize = 0;

    // we copy all headers, because some have to be modified while data are packed, these changes would likely mess up when reading data
    std::vector<LVPAFileHeader> headersCopy = _headers;

    // compressing is possibly going to take some time, better to show a progress bar
    ProgressBar bar;
    gProgress = &bar;
    bar.msg = "Preparing:    ";

    // Check & load any data that are required but not yet present
    for(uint32 i = 0; i < headersCopy.size(); ++i)
    {
        const LVPAFileHeader& h = headersCopy[i];

        if(!h.good)
        {
            logerror("Damaged file: '%s'", h.filename.c_str());
            continue;
        }

        if(h.flags & LVPAFLAG_SOLID)
        {
            LVPAFileHeader& sh = headersCopy[h.blockId];

            // quick check - this file's data are known, but the block is not loaded?
            // then the block has to be loaded, otherwise we can't append to it later.
            // If sh.realSize is 0, the block does not yet exist (will be created further below)
            if(h.data.ptr && !sh.data.ptr && sh.realSize)
            {
                memblock mbs = Get(sh.id, true); // this possibly modifies headers...
                if(!mbs.ptr)
                {
                    logerror("Failed to load required solid block '%s'; can't add file '%s'", sh.filename.c_str(), h.filename.c_str());
                    return false;
                }
                // ... so copy back the header afterwards
                sh = _headers[sh.id];
            }
        }
    }

    // find out sizes early to prevent re-allocation,
    // and prepare some of the header fields
    for(uint32 i = 0; i < headersCopy.size(); ++i)
    {
        LVPAFileHeader& h = headersCopy[i];
        if(!h.good)
            continue;

        // make used algo/compression level consistent
        // level 0 is always no algorithm, and vice versa
        if(h.algo == LVPAPACK_NONE)
            h.level = 0;
        else if(h.level == LVPACOMP_NONE)
            h.algo = 0;

        // overwrite settings if not specified otherwise
        // for now, only solid blocks, or non-solid files, the remaining solid files will come in next iteration
        if((h.flags & LVPAFLAG_SOLIDBLOCK) || !(h.flags & LVPAFLAG_SOLID))
        {
            if(h.flags & LVPAFLAG_SOLIDBLOCK)
                h.realSize = h.packedSize = 0; // we will fill this soon (***)
            else
            {
                // Got pointer? If not, the archive was loaded, not all files used, and is now saved again.
                // If we don't have a pointer, we keep the settings stored in the header.
                // (**) Keep in mind that if a file inside of a solid block was loaded, the whole solid block was loaded,
                //      and in turn if a solid block ptr is NULL, then none of the files in it was loaded.
                if(h.data.ptr)
                    h.realSize = h.packedSize = h.data.size;

                _realSize += h.realSize; // for stats
            }

            h.blockId = 0;
            DEBUG(h.blockId = -1); // to see if an error occurs - this value should never be used with these flags

            if(h.level == LVPACOMP_INHERIT)
                h.level = compression;
            if(h.algo == LVPAPACK_INHERIT)
                h.algo = algo;

            if(h.encryption == LVPAENCR_INHERIT)
            {
                if(encrypt)
                    h.flags |= LVPAFLAG_ENCRYPTED;
                else
                    h.flags &= ~LVPAFLAG_ENCRYPTED;
            }
            else if(h.encryption == LVPAENCR_NONE)
                h.flags &= ~LVPAFLAG_ENCRYPTED;
            else
                h.flags |= LVPAFLAG_ENCRYPTED;


            if( (h.flags & LVPAFLAG_ENCRYPTED) && _masterKey.empty())
            {
                logwarn("File '%s' should be encrypted, but no master key - not encrypting.", h.filename.c_str());
                h.flags &= ~LVPAFLAG_ENCRYPTED;
            }
        }
    }

    // one buf for each file - not all have to be used.
    // it WILL be used if a file has LVPAFLAG_PACKED set, which means the data went into a compressor
    // they might not be packed if the data are incompressible, in this case, the original data in memory are used
    AutoPtrVector<ICompressor> fileBufs(headersCopy.size());

    // first iteration - find out required sizes for the solid block buffers, and renumber solid block ids for the files stored inside
    for(uint32 i = 0; i < headersCopy.size(); ++i)
    {
        LVPAFileHeader& h = headersCopy[i];
        if(!h.good)
            continue;

        // files in a solid block are never marked as packed, because the solid block itself is already packed
        // but because we set the packed flag later, we can remove all of them.
        if(h.flags & LVPAFLAG_SOLID)
        {
            LVPAFileHeader& sh = headersCopy[h.blockId];

            h.flags &= ~LVPAFLAG_PACKED;

            // See (**)
            if(h.data.ptr)
                h.realSize = h.packedSize = h.data.size;
                
            sh.realSize += h.realSize + LVPA_EXTRA_BUFSIZE;
            _realSize += h.realSize; // for stats

            // overwrite settings if not specified otherwise
            // solid blocks were done in last iteration, now adjust the remaining files
            // files in a solid block will inherit its settings
            if(h.level == LVPACOMP_INHERIT)
                h.level = sh.level;
            if(h.algo == LVPAPACK_INHERIT)
                h.algo = sh.algo;

            // inherit from global settings, or just encrypt right away if set to do so
            // in case a file inside a solid block should be encrypted, the whole solid block needs to be encrypted,
            // so adjust its settings if necessary.
            if(h.encryption == LVPAENCR_ENABLED || (encrypt && h.encryption == LVPAENCR_INHERIT))
            {
                h.flags &= ~LVPAFLAG_ENCRYPTED; // remove it from this file, because the solid block gets encrypted, not this file
                sh.flags |= LVPAFLAG_ENCRYPTED;
            }
            else // not encrypting this file, but do not change the solid block's settings, as there may be other encrypted files in it.
            {
                h.flags &= ~LVPAFLAG_ENCRYPTED;
            }
        }
    }

    bar.total = _realSize / 1024; // we know the total size now, show in kB

    // second iteration - allocate the buffers and reserve sizes
    for(uint32 i = 0; i < headersCopy.size(); ++i)
    {
        const LVPAFileHeader& h = headersCopy[i];
        // Do we need to write the file to a buffer?
        // - All files but solid files, compressed, or a solid block that has data.
        // - h.data.ptr can be NULL here, for solid blocks which have not yet been created (= saving first time)
        // But h.data.ptr can also be NULL if the file was not loaded, see (**).
        bool needbuf_solidblock = h.realSize && (h.flags & LVPAFLAG_SOLIDBLOCK);
        bool needbuf_normal = h.data.ptr && (h.level != LVPACOMP_NONE) && !(h.flags & (LVPAFLAG_SOLID | LVPAFLAG_SOLIDBLOCK));
        if(h.good && !(h.flags & LVPAFLAG_SOLID) && (needbuf_solidblock || needbuf_normal) )
        {
            // each file (or solid block) can have its own compression algo, and level
            fileBufs.v[i] = allocCompressor(h.algo);
            if(!fileBufs.v[i])
            {
                logerror("Unknown compression algorithm %u for file '%s'", uint32(h.algo), h.filename.c_str());
                return false;
            }
        }
    }
    uint8 solidPadding[LVPA_EXTRA_BUFSIZE];
    memset(&solidPadding[0], 0, LVPA_EXTRA_BUFSIZE);
    // third iteration - append solid files to their blocks
    for(uint32 i = 0; i < headersCopy.size(); ++i)
    {
        LVPAFileHeader& h = headersCopy[i];
        if(h.good && (h.flags & LVPAFLAG_SOLID))
        {
            const LVPAFileHeader& sh = headersCopy[h.blockId];

            // nothing loaded at all? then neither the block nor the file were touched,
            // means it can be skipped here (and raw-copied below)
            if(!h.data.ptr && !sh.data.ptr)
                continue;

            // the solid block was loaded but the file not? Get the pointer.
            // If the block was loaded, a file was appended or changed, so we need to make sure
            // that all related files are present in memory when it comes to building the buffer.
            if(!h.data.ptr && sh.data.ptr)
            {
                h.data = Get(h.id); // note that this modifies the original headers
                if(!h.data.ptr)
                {
                    logerror("Failed to load file '%s' from solid block '%s' to append!", h.filename.c_str(), sh.filename.c_str());
                    return false;
                }
            }

            ICompressor *solidblock = fileBufs.v[h.blockId];
            DEBUG(ASSERT(solidblock));
            DEBUG(ASSERT(h.data.ptr));
            solidblock->reserve(h.realSize);
            solidblock->append(h.data.ptr, h.data.size);
            solidblock->append(&solidPadding[0], LVPA_EXTRA_BUFSIZE);
        }
    }

    bar.msg = "Compressing:  ";

    // fourth iteration - append each non-solid file to its buffer, and compress each file / solid block
    // also append each header to the header compressor buf
    uint32 writtenHeaders = 0;
    for(uint32 i = 0; i < headersCopy.size(); ++i)
    {
        LVPAFileHeader& h = headersCopy[i];
        if(!h.good)
            continue;
        ICompressor *block = fileBufs.v[i];
        if(block)
        {
            // solid blocks were already filled, and solid files didn't get their own buf allocated
            if(!(h.flags & (LVPAFLAG_SOLID | LVPAFLAG_SOLIDBLOCK)))
            {
                DEBUG(ASSERT(block->size() == 0));
                DEBUG(ASSERT(h.data.ptr));
                block->append(h.data.ptr, h.data.size);
            }

            if(block->size())
            {
                // calc unpacked crc before compressing
                h.crcReal = CRC32::Calc(block->contents(), block->size());

                if(h.level != LVPACOMP_NONE)
                    block->Compress(h.level, drawCompressProgressBar);

                h.packedSize = block->size();
                if(block->Compressed())
                {
                    h.flags |= LVPAFLAG_PACKED; // this flag was cleared earlier
                    h.crcPacked = CRC32::Calc(block->contents(), block->size());
                }

                // encrypt? these blocks will be thrown away, so we can just directly apply encryption
                _CryptBlock((uint8*)block->contents(), h, true);

                bar.PartialFix();
            }
            else
            {
                goto no_block_data; // ------->
            }
        }
        else if(h.data.ptr)
        {
            // Just to be sure, these must be set earlier
            DEBUG(ASSERT(h.data.size == h.realSize));
            DEBUG(ASSERT(h.data.size == h.packedSize));

            // we still need to calc crc
            h.crcReal = CRC32::Calc(h.data.ptr, h.data.size);

            // if the file should be encrypted, we have to make a copy anyways.
            if(h.data.size && (h.flags & (LVPAFLAG_ENCRYPTED | LVPAFLAG_SCRAMBLED)))
            {
                fileBufs.v[i] = new ICompressor;
                fileBufs.v[i]->append(h.data.ptr, h.data.size);
                _CryptBlock((uint8*)fileBufs.v[i]->contents(), h, true);
            }
        }
        else
        {
            no_block_data: // <---------
            // h.data.ptr == NULL, and no compressor block? Too bad. (**)

            // However, this was purposely set to 0 in case of a solid block. (***)
            // If it's not going to be compressed now, restore the setting.
            if(h.flags & LVPAFLAG_SOLIDBLOCK)
                h.packedSize = _headers[h.id].packedSize;
        }

        // for stats
        if(!(h.flags & LVPAFLAG_SOLID))
            _packedSize += h.packedSize;

        *zhdr << h;
        ++writtenHeaders;
    }

    if(!writtenHeaders)
    {
        logerror("No valid files - there were some, but they got lost on the way. Something is wrong.");
        return false;
    }

    // prepare master header (incomplete - not yet knowing all data!)
    LVPAMasterHeader masterHdr;
    ByteBuffer masterBuf;

    masterHdr.version = gVersion;
    masterHdr.hdrEntries = writtenHeaders;
    masterHdr.algo = algo;
    masterHdr.realHdrSize = zhdr->size();
    masterHdr.hdrCrcReal = 0;
    masterHdr.hdrCrcPacked = 0;

    if(zhdr->size())
    {
        masterHdr.hdrCrcReal = CRC32::Calc(zhdr->contents(), zhdr->size());

        // now we can compress the headers
        if(compression)
            zhdr->Compress(compression);

        if(zhdr->Compressed())
            masterHdr.hdrCrcPacked = CRC32::Calc(zhdr->contents(), zhdr->size());
    }

    masterHdr.flags = zhdr->Compressed() ? LVPAHDR_PACKED : LVPAHDR_NONE;
    if(encrypt)
        masterHdr.flags |= LVPAHDR_ENCRYPTED;
    // its not bad if its not packed now, then packed and unpacked sizes are just equal
    masterHdr.packedHdrSize = zhdr->size();

    // we don't know these yet
    masterHdr.hdrOffset = 0;
    masterHdr.dataOffs = 0;

    masterBuf << masterHdr;


    // -- write everything into the container file --
    gProgress = NULL;
    bar.Reset();
    bar.msg = "Writing file: ";
    bar.total = writtenHeaders;
    bar.Update();

    std::string tmpfn = GenerateTempFileName(fn);
    if(tmpfn.empty())
    {
        logerror("Failed to generate temporary file name for output!", fn);
        return false;

        // TODO: In that case, we could still open the original file and start writing to it,
        // (?)   possibly pre-loading files with h.data.ptr == NULL, because after fopen()
        //       it is no longer possible to read from the old file...
    }

    FILE *outfile = fopen(tmpfn.c_str(), "wb");
    if(!outfile)
    {
        logerror("Failed to open '%s' for writing!", fn);
        return false;
    }

    uint32 written = fwrite(gMagic, 1, 4, outfile);
    written += fwrite(masterBuf.contents(), 1, masterBuf.size(), outfile); // this will be overwritten later
    if(written != masterBuf.size() + 4)
    {
        logerror("Failed writing master header to LVPA file - disk full?");
        fclose(outfile);
        return false;
    }

    // ... space for additional data ...

    // now we know all fields of the master header, write it again
    masterHdr.hdrOffset = ftell(outfile);
    masterHdr.dataOffs = masterHdr.hdrOffset + zhdr->size(); // data follows directly after the headers, so we can safely assume this here

    masterBuf.wpos(0); // overwrite
    masterBuf << masterHdr;

    if(encrypt)
    {
        LVPACipher hdrCiph;
        hdrCiph.Init(&_masterKey[0], _masterKey.size()); // size was checked above
        hdrCiph.WarmUp(LVPA_HDR_CIPHER_WARMUP);
        uint32 ox = offsetof(LVPAMasterHeader, packedHdrSize);
        hdrCiph.Apply((uint8*)masterBuf.contents() + ox, masterBuf.size() - ox);
        hdrCiph.Apply((uint8*)zhdr->contents(), zhdr->size());
    }

    // write headers (we are still at the correct position in the file)
    written = fwrite(zhdr->contents(), 1, zhdr->size(), outfile);
    if(written != zhdr->size())
    {
        logerror("Failed writing headers block to LVPA file - disk full?");
        fclose(outfile);
        return false;
    }

    // write fixed master header
    fseek(outfile, 4, SEEK_SET); // after "LVPA"
    fwrite(masterBuf.contents(), 1, masterBuf.size(), outfile);
    fseek(outfile, masterHdr.dataOffs, SEEK_SET); // seek back

    // write the files
    for(uint32 i = 0; i < headersCopy.size(); ++i)
    {
        LVPAFileHeader& h = headersCopy[i];
        if(!h.good || (h.flags & LVPAFLAG_SOLID))
            continue;
        ICompressor *block = fileBufs.v[i];
        uint32 expected;

        if(block && block->size())
        {
            written = fwrite(block->contents(), 1, block->size(), outfile);
            expected = block->size();
        }
        else
        {
            if(h.data.ptr)
            {
                expected = h.data.size;
                written = 0;
                if(h.data.size)
                    written = fwrite(h.data.ptr, 1, h.data.size, outfile);
            }
            else
            {
                // When we are here, the file was not loaded until now,
                // does not exist in memory, and has all flags intact:
                // If it is compressed or encrypted, the data from the header are still valid
                // Just load the binary blob, and dump it into the output file.
                DEBUG(ASSERT(h.data.size == 0));
                DEBUG(ASSERT(h.packedSize));
                DEBUG(ASSERT(h.realSize));
                memblock blob;
                blob.size = h.packedSize;
                blob.ptr = new uint8[blob.size + LVPA_EXTRA_BUFSIZE];

                if(!_LoadFile(blob, h))
                {
                    logerror("Can't load '%s' from original file to raw-copy to outfile", h.filename.c_str());
                    delete [] blob.ptr;
                    fclose(outfile);
                    return false;
                }
                // TODO: if encrypted, decrypt & add a CRC check here
                
                expected = blob.size;
                written = 0;
                if(blob.size)
                    written = fwrite(blob.ptr, 1, blob.size, outfile);
                delete [] blob.ptr;
            }
        }
        if(written != expected)
        {
            logerror("Failed writing data to LVPA file - disk full?");
            fclose(outfile);
            return false;
        }
        ++bar.done;
        bar.Update();
    }

    // close the file if still open, to allow deletion
    _CloseFile();

    fclose(outfile);

    remove(fn);
    int renameRes = rename(tmpfn.c_str(), fn);
    bar.Finalize();

    if(renameRes != 0)
    {
        logerror("Renaming temp. output file (%s) failed: %s", tmpfn.c_str(), strerror(renameRes));
        logerror("You can still copy/rename it by hand now.");
        return false; // Return false anyways to indicate it didn't work 100% as it should.
    }

    return true;
}

static bool _memnull(void *buf, uint32 size)
{
    for(uint32 i = 0; i < size; ++i)
        if(((const char*)buf)[i] != 0)
            return false;
    return true;
}

memblock LVPAFile::_PrepareFile(LVPAFileHeader& h, bool checkCRC /* = true */)
{
    // h.good is set to false if there was a previous attempt to load the file that failed irrecoverably
    if(!h.good)
        return memblock();

    // already known? -- if h.data.ptr != NULL, the data must have been fully decrypted and unpacked already.
    if(h.data.ptr)
    {
        return h.data;
    }
    else
    {
        if(h.flags & LVPAFLAG_SOLID)
        {
            // these can never appear on files inside solid blocks
            h.flags &= ~(LVPAFLAG_PACKED | LVPAFLAG_ENCRYPTED | LVPAFLAG_SCRAMBLED);

            if(h.blockId >= _headers.size())
            {
                logerror("File '%s' is marked as solid (block %u), but there is no solid block with that ID", h.filename.c_str(), h.blockId);
                h.good = false;
                return memblock();
            }

            memblock solidMem = _PrepareFile(_headers[h.blockId], checkCRC);
            if(!solidMem.ptr)
            {
                logerror("Unable to load solid block for file '%s'", h.filename.c_str());
                return memblock();
            }

            if(h.offset + h.packedSize <= solidMem.size)
            {
                h.data.ptr = solidMem.ptr + h.offset;
                h.data.size = h.realSize;
                h.otherMem = true;
            }
            else
            {
                logerror("Solid file '%s' exceeds solid block length, can't read", h.filename.c_str());
                h.good = false;
                return memblock();
            }
        }
        else
        {
            h.otherMem = false;
            h.data = _UnpackFile(h);
        }

        if(!h.data.ptr) // if its still NULL, it failed to load
        {
            return memblock();
        }
    }

    // optionally check CRC32 of the unpacked data
    // -- for encrypted files, this is the only chance to find out whether the decryption key was correct
    // -- solid blocks can be skipped because the individual files are checksummed on their own
    if(checkCRC && !h.checkedCRC && !(h.flags & LVPAFLAG_SOLIDBLOCK))
    {
        h.checkedCRC = true;
        uint32 crc = CRC32::Calc(h.data.ptr, h.data.size);
        if(crc != h.crcReal)
        {
            logerror("CRC mismatch for unpacked '%s', file is corrupt, or decrypt fail", h.filename.c_str());
            if(h.flags & LVPAFLAG_ENCRYPTED)
                h.checkedCRC = false; // encrypted but failed, maybe the key was wrong, allow re-check
            else
                h.good = false; // if its not encrypted, there is nothing that could fix this
            return memblock();
        }
    }

    DEBUG(ASSERT(_memnull(h.data.ptr + h.data.size, LVPA_EXTRA_BUFSIZE)));

    return h.data;
}

memblock LVPAFile::_UnpackFile(LVPAFileHeader& h)
{
    DEBUG(ASSERT(h.good && !(h.flags & LVPAFLAG_SOLID))); // if this flag is set this function should not be entered

    ICompressor *buf = NULL;
    memblock target;

    if(h.flags & LVPAFLAG_PACKED)
    {
        buf = allocCompressor(h.algo);
        target.size = h.packedSize;
        if(target.size)
        {
            buf->resize(target.size);
            target.ptr = (uint8*)buf->contents();
        }
        buf->wpos(0);
        buf->rpos(0);
    }
    else
    {
        target.size = h.realSize;
        target.ptr = new uint8[target.size + LVPA_EXTRA_BUFSIZE];
    }

    if(!_DecryptFile(target, h))
    {
        if(buf)
            delete buf;
        return memblock();
    }

    // if the file is compressed, we allocated a decompressor buf earlier
    if(buf)
    {
        // check CRC32 of the packed data
        if(!h.checkedCRCPacked)
        {
            h.checkedCRCPacked = true;
            uint32 crc = CRC32::Calc(target.ptr, target.size);
            if(crc != h.crcPacked)
            {
                logerror("CRC mismatch for packed '%s', file is corrupt, or decrypt fail", h.filename.c_str());
                if(h.flags & LVPAFLAG_ENCRYPTED)
                    h.checkedCRCPacked = false; // encrypted but failed, maybe the key was wrong, allow re-check
                else
                    h.good = false; // if its not encrypted, there is nothing that could fix this
                return memblock();
            }
        }

        buf->Compressed(true); // tell the buf that it is compressed so it will allow decompression
        buf->RealSize(h.realSize);
        DEBUG(logdebug("'%s': uncompressing %u -> %u", h.filename.c_str(), h.packedSize, h.realSize));
        buf->Decompress();
        target.size = buf->size();
        target.ptr = new uint8[target.size + LVPA_EXTRA_BUFSIZE];

        if(target.size)
            buf->read(target.ptr, target.size);



        delete buf;
    }

    memset(target.ptr + target.size, 0, LVPA_EXTRA_BUFSIZE); // zero out extra space
    return target;
}

bool LVPAFile::_DecryptFile(memblock &target, LVPAFileHeader& h)
{
    DEBUG(ASSERT(h.good && !(h.flags & LVPAFLAG_SOLID))); // if this flag is set this function should not be entered

    if(!_LoadFile(target, h))
        return false;

    if(!_CryptBlock(target.ptr, h, false))
    {
        // failed to decrypt - this means the settings are not sufficient to decrypt the file,
        // NOT that the key was wrong!
        return false;
    }

    return true;
}

bool LVPAFile::_LoadFile(memblock& target, LVPAFileHeader& h)
{
    DEBUG(ASSERT(h.good && !(h.flags & LVPAFLAG_SOLID))); // if this flag is set this function should not be entered

    if(!_OpenFile())
        return false;

    // seek if necessary
    reader.seek(h.offset);

    uint32 bytes = reader.read(target.ptr, target.size);
    if(bytes != h.packedSize)
    {
        logerror("Unable to read enough data for file '%s'", h.filename.c_str());
        h.good = false;
        return false;
    }
    return true;
}

const LVPAFileHeader& LVPAFile::GetFileInfo(uint32 i) const
{
    DEBUG(ASSERT(i < _headers.size()));
    return _headers[i];
}

void LVPAFile::_CreateIndexes(void)
{
    for(uint32 i = 0; i < _headers.size(); ++i)
    {
        LVPAFileHeader& h = _headers[i];
        h.id = i; // this is probably not necessary...

        // do not index if the file name is not known
        if(h.flags & LVPAFLAG_SCRAMBLED && h.filename.empty())
            continue;

        _indexes[h.filename] = i;
    }
}

void LVPAFile::_CalcOffsets(uint32 startOffset)
{
    std::vector<uint32> solidOffsets(_headers.size());
    std::fill(solidOffsets.begin(), solidOffsets.end(), 0);

    for(uint32 i = 0; i < _headers.size(); ++i)
    {
        LVPAFileHeader& h = _headers[i];

        if(h.flags & LVPAFLAG_SOLID) // solid files use relative addressing inside their solid block
        {
            uint32& o = solidOffsets[h.blockId];
            h.offset = o;
            o += h.realSize + LVPA_EXTRA_BUFSIZE;
            DEBUG(logdebug("Rel offset %u for '%s'", h.offset, h.filename.c_str()));
        }
        else // non-solid files or solid blocks themselves use absolute file position addressing
        {
            h.offset = startOffset;
            startOffset += h.packedSize;
            DEBUG(logdebug("Abs offset %u for '%s'", h.offset, h.filename.c_str()));
        }

    }
}

void LVPAFile::RandomSeed(uint32 seed)
{
    _mtrand->seed(seed);
}

bool LVPAFile::AllGood(void) const
{
    for(uint32 i = 0; i < _headers.size(); ++i)
    if(!_headers[i].good)
        return false;

    return true;
}

void LVPAFile::SetMasterKey(const void *key, uint32 size)
{
    _masterKey.resize(size);
    if(size)
    {
        memcpy(&_masterKey[0], key, size);
        LVPAHash::Calc(&_masterSalt[0], (const uint8*)key, size);
    }
}

void LVPAFile::_CalcSaltedFilenameHash(uint8 *dst, const std::string& fn)
{
    LVPAHash sha(dst);
    sha.Update((uint8*)fn.c_str(), fn.size() + 1); // this DOES include the terminating '\0'
    if(_masterKey.size()) // if so, we also have the salt
        sha.Update(&_masterSalt[0], LVPAHash_Size);
    sha.Finalize();
}

bool LVPAFile::_CryptBlock(uint8 *buf, LVPAFileHeader& hdr, bool writeMode)
{
    if(!(hdr.flags & (LVPAFLAG_ENCRYPTED | LVPAFLAG_SCRAMBLED)))
        return true; // not encrypted, not scrambled, nothing to do, all fine

    uint8 mem[LVPAHash_Size];
    LVPACipher ciph;

    if(hdr.flags & LVPAFLAG_SCRAMBLED)
    {
        if(hdr.filename.empty())
        {
            DEBUG(logerror("File is scrambled, but given filename is empty, can't decrypt"));
            return false;
        }
        _CalcSaltedFilenameHash(&mem[0], hdr.filename.c_str());
        if(writeMode)
        {
            memcpy(&hdr.hash[0], &mem[0], LVPAHash_Size);
        }
        else if(memcmp(&mem[0], hdr.hash, LVPAHash_Size))
        {
            DEBUG(logerror("_CryptBlock: wrong file name"));
            return false;
        }

        LVPAHash::Calc(&mem[0], (uint8*)hdr.filename.c_str(), hdr.filename.length()); // this does NOT include the terminating '\0'

        if(hdr.flags & LVPAFLAG_ENCRYPTED)
        {
            LVPAHash sha(&mem[0]);
            if(_masterKey.size())
                sha.Update(&_masterKey[0], _masterKey.size());
            sha.Update(&mem[0], LVPAHash_Size);
            sha.Finalize();
        }

        ciph.Init(&mem[0], LVPAHash_Size);
    }
    else if(hdr.flags & LVPAFLAG_ENCRYPTED)
    {
        if(_masterKey.size())
            ciph.Init(&_masterKey[0], _masterKey.size());
        else
        {
            DEBUG(logerror("_CryptBlock: encrypted, not scrambled, and no master key!"));
            return false;
        }
    }

    if(!hdr.cipherWarmup)
    {
        uint32 r = _mtrand->randInt(128) + 30;
        hdr.cipherWarmup = uint16(r * sizeof(uint32)); // for speed, we always use full uint32 blocks
    }

    ciph.WarmUp(hdr.cipherWarmup);
    ciph.Apply(buf, hdr.packedSize); // packedSize because the file is encrypted AFTER compression!

    // CRC is checked elsewhere

    return true;
}

bool LVPAFile::_FindHeaderByName(const char *fn, uint32 *id)
{
    LVPAIndexMap::iterator it = _indexes.find(fn);
    if(it != _indexes.end())
    {
        *id = it->second;
        return true;
    }

    // not indexed, maybe its scrambled & hashed?
    uint8 mem[LVPAHash_Size];
    _CalcSaltedFilenameHash(&mem[0], fn);

    if(_FindHeaderByHash(&mem[0], id))
    {
        _headers[*id].filename = fn; // index now, for quick lookup later, and for later unscrambling, if needed
        _indexes[fn] = *id;
        return true;
    }

    return false;
}

bool LVPAFile::_FindHeaderByHash(uint8 *hash, uint32 *id)
{
    for(uint32 i = 0; i < _headers.size(); ++i)
    {
        const LVPAFileHeader &h = _headers[i];
        if(h.flags & LVPAFLAG_SCRAMBLED)
        {
            if(!memcmp(hash, h.hash, LVPAHash_Size))
            {
                *id = i;
                return true;
            }
        }
    }
    return false;
}


bool IsSupported(LVPAAlgos algo)
{
    switch(algo)
    {
#ifdef LVPA_SUPPORT_LZF
    case LVPAPACK_LZF:
        return true;
#endif
#ifdef LVPA_SUPPORT_LZO
    case LVPAPACK_LZO1X:
        return true;
#endif
#ifdef LVPA_SUPPORT_ZLIB
    case LVPAPACK_DEFLATE:
        return true;
#endif
#ifdef LVPA_SUPPORT_LZMA
    case LVPAPACK_LZMA:
        return true;
#endif
#ifdef LVPA_SUPPORT_LZHAM
    case LVPAPACK_LZHAM:
        return true;
#endif
    case LVPAPACK_INHERIT:
    case LVPAPACK_NONE:
        return true; // doing nothing is always supported :)
    }
    return false;
}

LVPA_NAMESPACE_END
