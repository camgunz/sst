#ifndef SPLITTER_H__
#define SPLITTER_H__

typedef struct {
    gchar  *data;
    String  section;
    bool    section_is_code;
} Splitter;

void splitter_clear(Splitter *splitter);
void splitter_init(Splitter *splitter, gchar *data);
bool splitter_load_next(Splitter *splitter);

#endif

