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