typedef struct
{
   const char *name;
   const char *url;
} Stream_t;

Stream_t streams[] = {
    {"CBC Radio One", "http://cbc_r1_mtl.akacast.akamaistream.net/7/706/451661/v1/rc.akacast.akamaistream.net/cbc_r1_mtl"},
    {"CBC Radio 2", "http://cbc_r2_mtl.akacast.akamaistream.net/7/706/451661/v1/rc.akacast.akamaistream.net/cbc_r2_mtl"},
    {"CBC Radio 3", "http://cbc_r3_mtl.akacast.akamaistream.net/7/706/451661/v1/rc.akacast.akamaistream.net/cbc_r3_mtl"},
    {"Jazz 24", "https://live.amperwave.net/direct/ppm-jazz24mp3-ibc1"}};

const int StreamCount = 4;