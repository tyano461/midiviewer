#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "common.h"
#include "midi.hpp"

CMidi::CMidi(const char *path)
{
    FILE *fp;
    char *buf;
    uint64_t ret;
    int fd;
    struct stat st;
    size_t size;

    fp = fopen(path, "rb");
    ERRRET(!fp, "file open failed");

    fd = fileno(fp);
    ret = fstat(fd, &st);
    ERRRET(!ret || !st.st_size, "stat failed");
    size = st.st_size;

    buf = (char *)malloc(size);
    ERRRET(!buf, "malloc failed.");

    ret = (uint64_t)fread(buf, size, 1, fp);
    ERRRET(ret <= 0, "fread failed");

    midi = (midi_t *)buf;
    pos = 0;
error_return:
    if (fp)
        fclose(fp);
    return;
}

bool CMidi::isMidi()
{
    const char *SIG = "MThd";
    return midi &&
           (*(uint32_t *)&midi->header.signature == *(uint32_t *)SIG);
}

uint32_t CMidi::datasize()
{
    if (midi)
    {
        return __bswap_32(midi->header.datasize);
    }
    return 0;
}

uint16_t CMidi::format()
{
    if (midi)
    {
        return __bswap_16(midi->header.format);
    }
    return 0;
}

uint16_t CMidi::tracknum()
{
    if (midi)
    {
        return __bswap_16(midi->header.tracknum);
    }
    return 0;
}

uint16_t CMidi::delta()
{
    if (this->midi)
    {
        return __bswap_16(this->midi->header.delta);
    }
    return 0;
}

uint32_t CMidi::trackdatasize()
{
    if (this->midi)
    {
        return __bswap_32(this->midi->track->size);
    }
    return 0;
}

uint32_t CMidi::measure_num()
{
    return 0;
}

uint32_t CMidi::eventnum()
{
    uint32_t pos = 0;
    uint8_t *p = midi->track->data;
    uint32_t datasize = this->trackdatasize();
    uint32_t size;

    while (p < (midi->track->data + datasize))
    {
        size = eventsize(p);
    }
    return 0;
}

measure_t *CMidi::measure(uint32_t measure)
{
    return nullptr;
}

uint32_t CMidi::eventsize(uint8_t *data)
{
    ERRRET(!data, "invalid pointer");

    if (!(*data & 0x80)) return (uint32_t)(*data);

    if (!(data[1] & 0x80)) {
        return ((data[0] & 0x7f) * 0x80) + data[1];
    }

    if (!(data[2] & 0x80)) {
        return ((data[0] & 0x7f) * 0x80 * 0x80)
        + ((data[1] & 0x7f) * 0x80)
        + data[2];
    }


    return 0;
}
