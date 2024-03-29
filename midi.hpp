#ifndef __MIDI_H__
#define __MIDI_H__
#include <stdint.h>
#include <stdbool.h>
#include <cstddef>

enum MTrkEVENT
{
    NOTEON = 0,
    NTEOFF,
    PITCHVEND,
    EXCLUSIVE,
#define EV_SYSEX_BASE EXCLUSIVE
    TEMPO,
#define EV_META_BASE TEMPO
    TITLE,
};

#pragma pack(push, 1)
typedef struct str_midi_hdr
{
    uint32_t signature;
    uint32_t datasize;
    uint16_t format;
    uint16_t tracknum;
    uint16_t delta;
} midihdr_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct str_track
{
    uint32_t signature;
    uint32_t size;
    uint8_t data[0];
} track_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct str_midi
{
    midihdr_t header;
    track_t track;
} midi_t;
#pragma pack(pop)

typedef struct str_mtrkevent
{
    uint32_t delta;
    uint32_t event;
    uint8_t *pos;
    uint8_t *next;
} mtrkevent_t;

typedef struct str_measure
{
    uint32_t measure;
    uint32_t delta;
    uint32_t evnum;
    mtrkevent_t data[];
} measure_t;

class CMidi
{
public:
    CMidi(const char *path);
    bool isMidi();
    uint32_t datasize();
    uint16_t format();
    uint16_t tracknum();
    uint16_t delta();
    uint32_t trackdatasize(int i);
    uint32_t measure_num();
    uint32_t eventnum();
    measure_t *measure(uint32_t measure);

private:
    midi_t *midi;
    size_t pos;
    uint32_t eventsize(uint8_t *data, uint8_t **after);
    uint8_t *eventdata(uint8_t *data, mtrkevent_t *ev);
    void showevent(mtrkevent_t *ev);
    void show_meta(mtrkevent_t *ev);
    void show_sys_ex(mtrkevent_t *ev);
    void show_mevent(mtrkevent_t *ev);
    bool isMtrk(track_t* track);
};

#endif
