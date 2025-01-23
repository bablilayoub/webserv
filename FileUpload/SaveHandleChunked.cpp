void    FileUpload::HandleChunkedData(std::string &Body) {
    while (true) 
    {
        if (bytesLeft > 0 && Body.length() >= bytesLeft) {
            chunkData = Body.substr(0, bytesLeft);
            if (!chunkData.empty()) {
                this->WriteToFile(chunkData);
            }
            Body = Body.substr(chunkData.length(), Body.length());
            bytesLeft = 0;
        }
        else {
            if (bytesLeft > 0) {
                chunkData = Body;
                if (!chunkData.empty()) {
                    this->WriteToFile(chunkData);
                }
                bytesLeft = bytesLeft - chunkData.length();
                Body = "";
            }
        }
        pos = Body.find(CRLF);
        ChunkSizeString = Body.substr(0, pos);
        std::istringstream iss(ChunkSizeString);
        chunkSize = 0;
        iss >> std::hex >> chunkSize;
        chunkData = Body.substr(pos + 2, chunkSize);
        if (chunkData.length() < chunkSize) 
        {
            bytesLeft = chunkSize - chunkData.length();
        }
        Body = Body.substr(pos + 2 + chunkData.length(), Body.length());
        
        if (!chunkData.empty()) {
            this->WriteToFile(chunkData);
        }
        if (Body.length() == 0) {
            break;
        }
    }
}

// Handle Binary Data save
void    FileUpload::HandleBinaryData(std::string mimeType) {
    size_t posMime;

    if ((posMime = mimeType.find("/")) != std::string::npos) {
        // mimeType = "." + mimeType.substr(posMime + 1, mimeType.length());
        // if (mimeType == ".plain")
        //     mimeType = ".txt";

        // else if (mimeType == ".javascript")
        //     mimeType = ".js";
        std::cout << "MIME TYPE : " << mimeType << std::endl;
        mimeType = this->MimeTypeMap[mimeType];
        this->FileName = "RBL" + generate_random_string(5) + mimeType;
        this->BinaryFileOpen = true;
        this->HeaderFetched = true;
        this->openFile = true;
    }
}