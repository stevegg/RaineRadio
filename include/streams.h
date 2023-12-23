typedef struct
{
    int channel;
    const char *name;
    const char *url;
} Stream_t;

Stream_t streams[] = {
    {1, "CBC Radio One", "http://playerservices.streamtheworld.com/pls/CBCVFM_CBC.pls"},
    {2, "CBC Music", "https://cbcradiolive.akamaized.net/hls/live/2041059/ES_R2PVC/master.m3u8"},
    {3, "BBC World", "http://stream.live.vc.bbcmedia.co.uk/bbc_world_service"},
    {4, "WNYC", "https://fm939.wnyc.org/wnycfm.aac"},
    {5, "NPR", "https://www.npr.org/streams/mp3/nprlive24.m3u"},
    {6, "NW Public Radio", "http://ett-ts02.ett.wsu.edu:8000/NWPRNEWS"},
    {7, "Jazz 24", "https://live.amperwave.net/direct/ppm-jazz24mp3-ibc1"},
    {8, "Smooth Jazz", "http://www.101smoothjazz.com/101-smoothjazz.m3u"},
    {9, "Sky Radio Xmas", "http://playerservices.streamtheworld.com/api/livestream-redirect/SRGSTR08.mp3"},
    {10, "KNKX", "https://live.amperwave.net/playlist/ppm-knkxfmaac48-ibc1.m3u"}};

const int StreamCount = 10;