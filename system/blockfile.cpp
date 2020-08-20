}
    fileMode = FM_CLOSED;
    blockMode = BM_CLOSED;
    mapPointer = nullptr;
    Close();
    // Close any open file
    Close();
    // Setup header info
    fileHeader.blockId = BLOCK_ID;
    fileHeader.verId = BLOCK_VER;
    fileHeader.blockCount = 0;
    // Other info
    filePos = 0;
    Utils::Strcpy(lastError, "No error");
    ASSERT(fileMode != FM_CLOSED);
    // Search for the entry
    IndexEntry* entry = index.Find(key);
    // Set last error because some functions pass result through
    if (!entry)
    {
        Utils::Sprintf(lastError, 256, "Unable to find block 0x%08X in file '%s'", key, nameId.str);
    }
    // Return result
    return (entry);
    // Allocate entry
    IndexEntry* entry = new IndexEntry;
    // Setup info
    entry->info.key = key;
    entry->info.size = size;
    // Add to index
    index.Add(entry->info.key, entry);
    // Return it
    return (entry);
    ASSERT(wFile.IsOpen());
    // Set file position
    filePos = pos;
    // Go to position in file
    if (!wFile.Seek(File::SET, pos))
    {
        ERR_FATAL(("Failed seek to %d in block file '%s'", pos, nameId.str));
    }
    // Set file position
    filePos = pos;

    // Go to position in file
    if (!rFile->Seek(pos))
    {
        ERR_FATAL(("Failed seek to %d in block file '%s'", pos, nameId.str));
    }
void BlockFile::WriteData(const void* data, U32 size)
    ASSERT((fileMode == FM_CREATE) || (fileMode == FM_APPEND));
    // Write the data
    if (wFile.Write(data, size) != size)
    {
        ERR_FATAL(("Failed writing %d bytes to block file '%s'", size, nameId.str));
    }
    // Update current write position
    filePos += size;
void BlockFile::ReadData(void* dest, U32 size)
    ASSERT(dest);
    // Read the data
    if (rFile->Read(dest, size) != size)
    {
        ERR_FATAL(("Failed reading %d bytes from block file '%s'", size, nameId.str));
    }
    // Update current read position
    filePos += size;
    // Jump to start of file
    WriteSeekTo(0);
    // Write the header
    WriteData(&fileHeader, sizeof(BlockHeader));
Bool BlockFile::Open(const char* name, OpenMode mode, Bool fatal)
    // Initialise data
    Initialize();
    // Record file name
    nameId = name;
    // Mode dependent
    switch (mode)
        case CREATE:
        {
            // Create the file
            if (!wFile.Open(name, File::WRITE | File::CREATE))
            {
                Utils::Sprintf(lastError, 256, "Unable to create file '%s'", name);
                CHECK_FATAL;
                return (FALSE);
            }
            // Set new file mode 
            fileMode = FM_CREATE;
            // Write the header
            WriteBlockHeader();
            break;
        }
        case READ:
            // Open the file
            rFile = FileSys::Open(name);

            if (!rFile)
            {
                // unable to open file
                Utils::Sprintf(lastError, 256, "Unable to open file '%s'", name);
                CHECK_FATAL;
                return (FALSE);
            }
            // Is the file large enough?
            if (rFile->Size() < sizeof(BlockHeader))
            {
                FileSys::Close(rFile);
                Utils::Sprintf(lastError, 256, "'%s' is not a valid block file", name);
                CHECK_FATAL;
                return (FALSE);
            }
            // Read header
            ReadData(&fileHeader, sizeof(BlockHeader));
            // Is this file for us
            if (fileHeader.blockId != BLOCK_ID)
            {
                FileSys::Close(rFile);
                Utils::Sprintf(lastError, 256, "'%s' is not a valid block file", name);
                CHECK_FATAL;
                return (FALSE);
            }
            // Do version specific loading
            switch (fileHeader.verId)
                case BLOCK_VER001:
                {
                    BlockInfo bInfo;

                    // Read each block info
                    for (U32 i = 1; i <= fileHeader.blockCount; i++)
                    {
                        // Read block info
                        ReadData(&bInfo, sizeof(BlockInfo));

                        // Allocate a new index entry
                        IndexEntry* entry = NewIndexEntry(bInfo.key, bInfo.size);

                        // Set the data position
                        entry->dataPos = filePos;

                        // Seek past data for all but last blocky
                        if (i < fileHeader.blockCount)
                        {
                            ReadSeekTo(entry->dataPos + entry->info.size);
                        }
                    }
                    break;
                }

                default:
                {
                    FileSys::Close(rFile);
                    Utils::Sprintf(lastError, 256, "'%s' is an unsupported file version", name);
                    CHECK_FATAL;
                    return (FALSE);
                }
            // Map block file into memory
            mapPointer = static_cast<U8*>(rFile->GetMemoryPtr());
            // Set new file mode
            fileMode = FM_READ;
            break;
        }
        case APPEND:
        ERR_FATAL(("FIXME : Blockfile append isn't done yet ;)"));
    return (TRUE);
    ASSERT(blockMode == BM_CLOSED);

    // Mode dependent
    switch (fileMode)
    {
            // Already closed
        case FM_CLOSED:
            break;

            // Just close the file
        case FM_READ:
            FileSys::Close(rFile);
            break;

            // Write out header
        case FM_CREATE:
        case FM_APPEND:
            WriteBlockHeader();
            wFile.Close();
            break;

        default:
        ERR_FATAL(("Unknown mode"));
    }

    // flag file closed
    fileMode = FM_CLOSED;

    // clear memory mapped pointer
    mapPointer = nullptr;

    // delete any index entries
    index.DisposeAll();
{
    // Just do tree search
    return (index.Exists(key));
}
    IndexEntry* entry = index.Find(key);

    // Did we find it
    if (!entry)
    {
        // Block not found
        ERR_FATAL(("Unable to find block 0x%08x in %s", key, nameId.str));
    }

    // Return the size
    return (entry->info.size);
U8* BlockFile::GetBlockPtr()
    if (fileMode == FM_READ && blockMode == BM_READ)
    {
        ASSERT(mapPointer);
        ASSERT(cEntry);

        return (mapPointer + cEntry->dataPos);
    }
    return (nullptr);
Bool BlockFile::OpenBlock(U32 key, Bool fatal, U32* size)
    ASSERT(fileMode != FM_CLOSED);
    ASSERT(blockMode == BM_CLOSED);
    // Depends on current file mode
    switch (fileMode)
        case FM_CREATE:
        case FM_APPEND:
        {
            // Make sure key doesn't already exist
            if (FindKey(key))
            {
                Utils::Sprintf(lastError, 256, "Key 0x%08X already exists in file '%s'", key, nameId.str);
                CHECK_FATAL;
                return (FALSE);
            }
            // Setup new index entry
            cEntry = NewIndexEntry(key, 0);
            // Write the block entry (will be overwritten in BlockClose)
            WriteData(&cEntry->info, sizeof(BlockInfo));
            // Set data position
            cEntry->dataPos = filePos;
            // Increase the block count
            fileHeader.blockCount++;

            // Clear the size
            if (size)
            {
                *size = 0;
            }

            // Set the new block mode
            blockMode = BM_WRITE;
            break;
        }

        case FM_READ:
        {
            // Get the block index entry
            if ((cEntry = GetIndexEntry(key)) == nullptr)
            {
                CHECK_FATAL;
                return (FALSE);
            }
            // Seek to data
            ReadSeekTo(cEntry->dataPos);

            // Set the size
            if (size)
            {
                *size = cEntry->info.size;
            }

            // Set the new block mode
            blockMode = BM_READ;
            break;
        }

        default:
        ERR_FATAL(("Unknown case"));
    }

    // Success
    return (TRUE);
    ASSERT(fileMode != FM_CLOSED);
    // Mode dependent
    switch (blockMode)
            // Finish by writing out index
        case BM_WRITE:
        {
            // Calculate block size
            cEntry->info.size = filePos - cEntry->dataPos;
            // Seek to start of block header
            WriteSeekTo(cEntry->dataPos - sizeof(BlockInfo));
            // Write the header
            WriteData(&cEntry->info, sizeof(BlockInfo));
            // Seek past data
            WriteSeekTo(cEntry->dataPos + cEntry->info.size);
            break;
        }
        default:
            break;
    }
    // Just set the new mode
    blockMode = BM_CLOSED;
U32 BlockFile::ReadFromBlock(void* dest, U32 size, Bool fatal)
    ASSERT(fileMode == FM_READ);
    ASSERT(blockMode == BM_READ);
    ASSERT((cEntry->info.size - (filePos - cEntry->dataPos)) >= 0);
    // Get remaining bytes
    U32 remain = cEntry->info.size - (filePos - cEntry->dataPos);
    // Clip our size
    if (size > remain)
    {
        Utils::Sprintf
        (
            lastError, 256, "Expected to read %d bytes from file '%s' but found %d", size, nameId.str,
            remain
        );
        CHECK_FATAL;
        size = remain;
    }
    // Read requested size
    if (size)
    {
        ReadData(dest, size);
    }
    // Return bytes read
    return (size);
void BlockFile::WriteToBlock(const void* data, U32 size)
    ASSERT(blockMode == BM_WRITE);
    // Just write data
    WriteData(data, size);
Bool BlockFile::ReadBlock(U32 key, void* dest, U32 size, Bool fatal)
    ASSERT(fileMode == FM_READ);

    // Open requested block
    if (OpenBlock(key, fatal))
    {
        // Read entire block
        U32 r = ReadFromBlock(dest, size, fatal);
        // Close the block
        CloseBlock();
        // Was the block the expected size
        if (r == size)
        {
            return (TRUE);
        }
        // Set error string
        Utils::Sprintf(lastError, 256, "Block 0x%08x (%s) was smaller than expected (%d/%d)", key, nameId.str, r, size);
    // Some error
    CHECK_FATAL;
    return (FALSE);
Bool BlockFile::WriteBlock(U32 key, const void* data, U32 size, Bool fatal)
    ASSERT((fileMode == FM_CREATE) || (fileMode == FM_APPEND));
    // Open requested block
    if (OpenBlock(key, fatal))
    {
        WriteToBlock(data, size);
        // Close the block
        CloseBlock();
        // Success
        return (TRUE);
    }
    // Some error
    CHECK_FATAL;
    return (FALSE);
void* BlockFile::ReadBlockAlloc(U32 key, U32* size, Bool fatal)
    ASSERT(fileMode == FM_READ);
    // Get index entry
    IndexEntry* entry = GetIndexEntry(key);
    // Did we find it
    if (entry)
        // Allocate buffer
        char* dest = new char[entry->info.size];

        // Read the block
        if (ReadBlock(key, dest, entry->info.size, fatal))
        {
            // If requested, return the size
            if (size)
            {
                *size = entry->info.size;
            }
            // Success
            return (dest);
        }
        // Error, so delete buffer
        delete dest;
    }

    // Failed
    CHECK_FATAL;
    return (nullptr);