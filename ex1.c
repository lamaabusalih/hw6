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
//structure of type entry eith 4 fields
typedef struct 
{
    char* fn, *sn, *fp, *pos;
} Entry;
// a function which accepts an array of strings and the num of strings, and chicks if we had this fingerprint before and if we do returns 1 if we dont 0!
int seen_before(char **saved_fingerprints, int saved_fingerprints_count, const char *fp) {
    for (int i = 0; i < saved_fingerprints_count; i++)
        if (strcmp(saved_fingerprints[i], fp) == 0) return 1;
    return 0;
}
// a function which prints the 4 fields of the structure
void print_entry(FILE *out, Entry e) 
{
    fprintf(out, "First Name: %s\n", e.fn);
    fprintf(out, "Second Name: %s\n", e.sn);
    fprintf(out, "Fingerprint: %s\n", e.fp);
    fprintf(out, "Position: %s\n\n", e.pos);
}


void FREE_ENTRY(char *fn, char* sn, char* fp, char* pos)
{
    free(fn);
    free(sn);
    free(fp);
    free(pos);
}


int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s <input_corrupted.txt> <output_clean.txt>\n", argv[0]);
        return 0;
    }
    // TODO: implement

    FILE *in = fopen(argv[1], "r");
    if (!in) { perror("reading failed"); return 1; }

    char *clean = clean_up_file(in);
    fclose(in);
    if (!clean) { fprintf(stderr, "clean_up_file failed\n"); return 1; }

    /* ---------- TASK 3 (dedup + ordering + write) ---------- */

    Entry boss = (Entry){0}, righthand = (Entry){0}, lefthand = (Entry){0};

    Entry *supR = NULL, *supL = NULL;
    int supR_count = 0, supL_count = 0;
    int supR_cap = 0, supL_cap = 0;

    char **saved_fps = NULL;
    int saved_count = 0, saved_cap = 0;

    char *p = clean;

    while (1) {
        Entry e;
        char *next = extract_entry(p, &e.fn, &e.sn, &e.fp, &e.pos);
        if (!next) break;
        p = next;

        /* dedup by fingerprint (keep first) */
        if (seen_before(saved_fps, saved_count, e.fp)) {
            free(e.fn); free(e.sn); free(e.fp); free(e.pos);
            continue;
        }

        // store fp in seen list 
        if (saved_count == saved_cap) {
            saved_cap = (saved_cap == 0) ? 8 : saved_cap * 2;
            saved_fps = realloc(saved_fps, saved_cap * sizeof(*saved_fps));
            if (!saved_fps) { fprintf(stderr, "Out of memory\n"); exit(1); }
        }
        saved_fps[saved_count++] = e.fp; // freed when freeing entries 

        // bucket by position (Boss, Right Hand, Left Hand, then supports in first-seen order) 
        if (!strcmp(e.pos, "Boss") && boss.fp == NULL) boss = e;
        else if (!strcmp(e.pos, "Right Hand") && righthand.fp == NULL) righthand = e;
        else if (!strcmp(e.pos, "Left Hand") && lefthand.fp == NULL) lefthand = e;
        else if (!strcmp(e.pos, "Support_Right")) 
        {
            if (supR_count == supR_cap) {
                supR_cap = (supR_cap == 0) ? 8 : supR_cap * 2;
                supR = realloc(supR, supR_cap * sizeof(*supR));
                if (!supR) { fprintf(stderr, "Out of memory\n"); exit(1); }
            }
            supR[supR_count++] = e;
        } 
        else if (!strcmp(e.pos, "Support_Left")) 
        {
            if (supL_count == supL_cap) {
                supL_cap = (supL_cap == 0) ? 8 : supL_cap * 2;
                supL = realloc(supL, supL_cap * sizeof(*supL));
                if (!supL) { fprintf(stderr, "Out of memory\n"); exit(1); }
            }
            supL[supL_count++] = e;
        } 
        else 
        {
            //duplicate boss/right/left after first, or unknown position 
            free(e.fn); free(e.sn); free(e.fp); free(e.pos);
        }
    }

    FILE *out = fopen(argv[2], "w");
    if (!out) { perror("output"); free(clean); return 1; }

    if (boss.fp)      print_entry(out, boss);
    if (righthand.fp) print_entry(out, righthand);
    if (lefthand.fp)  print_entry(out, lefthand);
    for (int i = 0; i < supR_count; i++) print_entry(out, supR[i]);
    for (int i = 0; i < supL_count; i++) print_entry(out, supL[i]);

    fclose(out);


    FREE_ENTRY(boss.fn, boss.sn, boss.fp,boss.pos);
    FREE_ENTRY(righthand.fn, righthand.sn, righthand.fp,righthand.pos);
    FREE_ENTRY(lefthand.fn, lefthand.sn, lefthand.fp, lefthand.pos);
    for (int i = 0; i < supR_count; i++) FREE_ENTRY(supR[i].fn, supR[i].sn, supR[i].fp, supR[i].pos);
    for (int i = 0; i < supL_count; i++) FREE_ENTRY(supL[i].fn, supL[i].sn, supL[i].fp, supL[i].pos);

    free(supR);
    free(supL);
    free(saved_fps);
    free(clean);
    

    return 0;

}

    

