#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "midi.hpp"

void usage(void);
char *exepath(void);
extern void read_midi_data(char *path);

int main(int argc, char *argv[])
{
    CMidi *midi = NULL;
    ERRPROC(argc < 2, usage());

    midi = new CMidi(argv[1]);

    if(midi->isMidi()) {
        d("t:%d sz:%x f:%x d:%x"
        , midi->tracknum()
        , midi->datasize()
        , midi->format()
        , midi->delta()
        );

        midi->eventnum();
    } else {
        d("not midi");
    }

error_return:
    if (midi)delete midi;
    return 0;
}

void usage(void)
{
    char *path = exepath();
    char *f = strrchr(path, '/') + 1;
    printf("usage:\n\t%s [midifile]\n", f);
}

char _exepath[MAX_PATH];
char *exepath(void)
{
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "/proc/%d/exe", getpid());
    readlink(path, _exepath, MAX_PATH);
    return _exepath;
}

