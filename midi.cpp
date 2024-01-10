#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "common.h"
#include "midi.hpp"

char debugbuf[256];
char *b2s(const char *s, int size)
{
    int i;
    char *p = debugbuf;
    uint32_t val;
    for (i = 0; i < size; i++)
    {
        p += (int)sprintf(p, "%02x ", ((uint8_t *)s)[i]);
    }
    return debugbuf;
}

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
    ERRRET(ret || !st.st_size, "stat failed");
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
        return __bswap_32(this->midi->track.size);
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
    uint8_t *p = midi->track.data;
    uint32_t datasize = this->trackdatasize();
    uint32_t size;
    mtrkevent_t ev;

    ev.next = p;
    while (p < (midi->track.data + datasize))
    {
        p = eventdata(p, &ev);
        printf("%8d: ", pos++);
        showevent(&ev);
    }
    return 0;
}

measure_t *CMidi::measure(uint32_t measure)
{
    return nullptr;
}

uint8_t *CMidi::eventdata(uint8_t *data, mtrkevent_t *ev)
{
    uint32_t delta = 0;
    uint32_t msize;
    uint8_t *p;
    uint8_t *tmp;
    int i;

    ASSERT(data);

    ev->next = NULL;
    ev->pos = data;
    ev->delta = eventsize(data, &p);

    if ((*p & 0xf0) == 0xf0)
    {
        if (*p == 0xff)
        {
            ev->event = (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16));
            tmp = &p[2];
            msize = eventsize(tmp, &p);
            ev->next = &p[msize];
        }
        else if (*p == 0xf7)
        {
            ev->event = (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16));
            tmp = &p[1];
            msize = eventsize(tmp, &p);
            ev->next = &p[msize];
        }
        else if (*p == 0xf0)
        {
            ev->event = (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16));
            tmp = &p[1];
            msize = eventsize(tmp, &p);
            ev->next = &p[msize];
        }
        else
        {
        }
    }
    else if ((*p & 0xd0) == 0xd0)
    {
        ev->event = (uint32_t)(p[0] | (p[1] << 8));
        ev->next = &p[2];
    }
    else if ((*p & 0xc0) == 0xc0)
    {
        ev->event = (uint32_t)(p[0] | (p[1] << 8));
        ev->next = &p[2];
    }
    else if ((*p & 0xb0) == 0xa0)
    {
        ev->event = (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16));
        ev->next = &p[3];
    }
    else
    {
        ev->event = (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16));
        ev->next = &p[3];
        // 3byte
    }

    return ev->next;
}
uint32_t CMidi::eventsize(uint8_t *data, uint8_t **after)
{
    ASSERT(data);

    if (!(*data & 0x80))
    {
        *after = &data[1];
        return (uint32_t)(*data);
    }

    if (!(data[1] & 0x80))
    {
        *after = &data[2];
        return ((data[0] & 0x7f) * 0x80) + data[1];
    }

    if (!(data[2] & 0x80))
    {
        *after = &data[3];
        return ((data[0] & 0x7f) * 0x80 * 0x80) + ((data[1] & 0x7f) * 0x80) + data[2];
    }

    *after = &data[4];
    return ((data[0] & 0x7f) * 0x80 * 0x80 * 0x80) +
           ((data[1] & 0x7f) * 0x80 * 0x80) +
           ((data[2] & 0x7f) * 0x80) +
           data[3];
}

void CMidi::show_meta(mtrkevent_t *ev)
{
    uint8_t *p = ev->pos;
    uint8_t *tmp;
    uint32_t size;
    uint8_t *buf;
    uint8_t kind;

    kind = p[2];
    tmp = &p[3];
    size = eventsize(tmp, &p);

    buf = (uint8_t *)malloc(size + 1);

    printf("%p %s [meta ] %02x sz:%3d\n", ev->pos - (uint64_t)midi, b2s((const char *)ev->pos, 3), kind, size);
    free(buf);
}
void CMidi::show_sys_ex(mtrkevent_t *ev)
{
    uint8_t *p;
    uint8_t *tmp;
    uint32_t size;
    uint32_t delta;

    p = ev->pos;
    tmp = p;
    delta = eventsize(tmp, &p);
    tmp = &p[1];
    size = eventsize(tmp, &p);
    printf("%p %s [SysEx] --  %3d\n", ev->pos - (uint64_t)midi, b2s((const char *)ev->pos, 3), size);
}
void CMidi::show_mevent(mtrkevent_t *ev)
{
    uint8_t *p;
    uint32_t delta;
    uint8_t *tmp;
    uint16_t tmp_u16;

    tmp = ev->pos;
    delta = eventsize(tmp, &p);
    printf("%p %s ", ev->pos - (uint64_t)midi, b2s((const char *)ev->pos, 3));

    if (((*p) & 0xe0) == 0xe0)
    {
        tmp_u16 = p[1] | (p[2] << 8);
        printf("[pitch bend] ch:%x v:%x\n", p[0] & 0xf, tmp_u16);
    }
    else if (((*p) & 0xd0) == 0xd0)
    {
        printf("[channel pressure] ch:%x val:%d\n", p[0] & 0xf, p[1]);
    }
    else if (((*p) & 0xc0) == 0xc0)
    {
        printf("[program change] ch:%x tone:%d\n", p[0] & 0xf, p[1]);
    }
    else if (((*p) & 0xb0) == 0xb0)
    {
        printf("[control change] ch:%x ctr:%02x val:%d\n", p[0] & 0xf, p[1], p[2]);
    }
    else if (((*p) & 0xa0) == 0xa0)
    {
        printf("[poliphonic key pressure] ch:%x note:%02x val:%d\n", p[0] & 0xf, p[1], p[2]);
    }
    else if (((*p) & 0x90) == 0x90)
    {
        if (p[2])
        {
            printf("[note on] ch:%x note:%d v:%d\n", p[0] & 0xf, p[1], p[2]);
        }
        else
        {
            printf("[note off] ch:%x note:%d\n", p[0] & 0xf, p[1]);
        }
    }
    else if (((*p) & 0x80) == 0x80)
    {
        printf("[note off] ch:%x note:%d\n", p[0] & 0xf, p[1]);
    }
    else
    {
        printf("[UNK  ] -- %2x\n", p[1]);
    }
}

void CMidi::showevent(mtrkevent_t *ev)
{
    uint8_t *p;

    p = (uint8_t *)&ev->event;

    switch (*p)
    {
    case 0xff:
        show_meta(ev);
        break;
    case 0xf7:
    case 0xf0:
        show_sys_ex(ev);
        break;

    default:
        show_mevent(ev);
        break;
    }
}
