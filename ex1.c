#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string.h>

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
    size_t memory_index = 0;
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

//help functions that gets the start of a "field name" and a start of the next "field" and gets the word between them
char* trim_and_copy(const char* a, const char* b)
{
    while (a < b && isspace((unsigned char)*a)) 
    {
        a++;
    }
    while (b > a && isspace((unsigned char)*(b - 1))) 
    {
        b--;
    }
// memory allocation
    size_t n = (size_t)(b - a);
    char *memory = (char*)malloc(n + 1);
    if (memory == NULL) 
    {
        return NULL;
    }

    if (n > 0) 
    {
        memcpy(memory, a, n);
    }
    memory[n] = '\0';

    return memory;
}

char *extract_entry(char *fp, char **firstname,char **secondname, char **fingerprint, char **position)
{
    const char* F1 = "First Name:";
    const char* F2 = "Second Name:";
    const char* F3 = "Fingerprint:";
    const char* F4 = "Position:";
    
    const char* a = strstr(fp, F1); if (!a) {return NULL;}
    const char* b = strstr(a+strlen(F1), F2); if (!b) {return NULL;}
    const char* c = strstr(b+strlen(F2), F3); if (!c) {return NULL;}
    const char* d = strstr(c+strlen(F3), F4); if (!d) {return NULL;}
    const char* nexta = strstr(d + strlen(F4), F1);
    
    *firstname = trim_and_copy(a+strlen(F1), b);
    *secondname = trim_and_copy(b+strlen(F2), c);
    *fingerprint = trim_and_copy(c+strlen(F3), d);
    const char* end;
    if (nexta != NULL) 
    {
        end = nexta;
    } 
    else 
    {
        end = d + strlen(d);
    }
    *position = trim_and_copy(d+strlen(F1), end);
    if (!*firstname || !*secondname || !*fingerprint || !*position) 
    {
        free(*firstname);
        free(*secondname);
        free(*fingerprint);
        free(*position);
        return NULL;
    }

    return end;   
}
    
int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_corrupted.txt> <output_clean.txt>\n", argv[0]);
        return 0;
    }
    // TODO: implement
    return 0;
}
