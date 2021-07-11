#include "s98file.hpp"

S98File::S98File(){

}

void S98File::extractHeader() {

    constexpr uint8_t v3_utf8[] = {0xEF, 0xBB, 0xBF};
    uint32_t lastIndex = header->dataptr < header->nameptr ? filesize : header->dataptr;
    if (header->nameptr != 0){
        if (header->format == '3' && lastIndex - header->nameptr > 8){
            if (memcmp(data+header->nameptr, "[S98]", 5)){
                return;
            }

            if (memcmp(data+header->nameptr+5,v3_utf8,3)){
                //SJIS
                iconv_t ic = iconv_open("UTF-8","SHIFT-JIS");
                size_t str_length = lastIndex - header->nameptr - 5;
                size_t str_out_length = str_length * 3;
                char str_out[str_out_length];
                memset(str_out,0,sizeof(str_out));
                unsigned char* ptr_in = (data+header->nameptr+5);
                char* ptr_out = str_out;
                iconv(ic, (char**)&ptr_in, &str_length, &ptr_out, &str_out_length);
                iconv_close(ic);
                printf("%s", str_out);

            }else{
                //UTF-8
                size_t str_length = lastIndex - header->nameptr - 8;
                string infostring(str_length, 1);
                memmove(&infostring[0], data+header->nameptr+8, str_length);
                printf("%s", infostring.c_str());
            }


        }else if (header->format == '1' && lastIndex - header->nameptr > 0){

        }
    }

}

bool S98File::setFilePath(const char* filepath) {

    // attempt to read the file
	FILE *file = fopen(filepath, "rb");
	if (file == nullptr)
	{
		fprintf(stderr, "Error opening file '%s'\n", filepath);
        return false;
	}

	fseek(file, 0, SEEK_END);
	filesize = ftell(file);
	fseek(file, 0, SEEK_SET);
    data = new uint8_t[filesize];

    auto bytes_read = fread(data, 1, filesize, file);
    fclose(file);
	if (bytes_read != filesize)
	{
		fprintf(stderr, "Error reading file contents\n");
        return false;
	}

    header = reinterpret_cast<S98Header*>(data);

    //analyze header
    if (strncmp(header->magic, "S98", 3)){
        return false;
    }
    printf("datasize: %d\n", filesize);
    printf("dataptr: %d, loopptr: %d nameptr: %d\n", header->dataptr, header->loopptr, header->nameptr);
    printf("device count: %d\n", header->devicecount);
    
    for (int i = 0; i < header->devicecount;i++){
        printf("device type: %d\n", header->deviceInfo[i].type);
        if (header->deviceInfo[i].type == S98File::TYPE_NONE) header->deviceInfo[i].type = S98File::TYPE_OPNA;
    }

    //if timer is not set, use default value instead
    if (header->timer1 == 0) header->timer1 = 10;
    if (header->timer2 == 0) header->timer2 = 1000;

    //useful header data extraction
    std::filesystem::path path = filepath;
    songName = path.filename().generic_u8string();
    extractHeader();

    return true;
}

S98File::~S98File(){
    delete data;
}