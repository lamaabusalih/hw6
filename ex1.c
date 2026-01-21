#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//TODO create functions that you can use to clean up the file!
int is_corruption(char ch)
{
    return (ch == '#' || ch == '?' || ch == '!' || ch == '@' || ch == '&' || ch == '$');
}

char* clean_up_file(FILE* fp)
{
// memory allocation for our new file
    long filesize;
    if (fseek(fp, 0, SEEK_END) !=0)
    {
        return NULL;
    }
    filesize = ftell(fp);
    if (filesize<0)
    {
        return NULL;
    }
    rewind(fp);

    char *memory = NULL;
    memory = (char*)malloc(filesize+1);
    if (memory == NULL)
    {
        printf("Memory allocation failed!");
        return NULL;
    }
    int c, is_previous_char_space = 0;
    long memory_index = 0;
    while ((c = fgetc(fp)) != EOF)
    {
        char ch = (char)c;
        if (is_corruption(ch))
        {
            continue;
        }
        if (ch == '\n' || ch == '\r' || ch == '\t')
        {
            ch = ' ';
        }   
           
// a code that checks if the previous char was also ' ' and skeps white spaces!       
        if (ch == ' ')
        {            
            if (is_previous_char_space == 1)
            {
                continue; // since this char is ' ', if the last one was too ten no need to add this one!
            }
            is_previous_char_space = 1; // if the preious char wasnt ' ' then next itteration's previous char (which is this itterations char) will indeed be ' '!                                   
        }
        else { is_previous_char_space = 0; } // if this char isn't ' ' then next itteration's previous char is indeed not ' '!            

//now we add the char we read to the memory!       
        memory[memory_index] = ch;
        memory_index +=1 ;
    }
    memory[memory_index] = '\0';
    return memory;
}

    
int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_corrupted.txt> <output_clean.txt>\n", argv[0]);
        return 0;
    }
    // TODO: implement
    return 0;
}
