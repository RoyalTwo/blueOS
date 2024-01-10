#define VID_MEM_START 0xb8000
#define ROWS 25
#define COLS 80

void kprint_string(char *input, int *pos)
{
}

void main()
{
    int vid_mem_pos = 0;
    char *display_mem = (char *)VID_MEM_START;
    // First byte is character, second is color
    // A = 65, Z = 90
    int x = 0; // x = placement
    for (int i = 65; i < 91; i++)
    {
        display_mem[x * 2] = i;
        x++;
    }
    return 0;
}