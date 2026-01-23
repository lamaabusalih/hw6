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
    long filesize;
    if (fseek(fp, 0, SEEK_END) != 0) return NULL;
    filesize = ftell(fp);
    if (filesize < 0) return NULL;
    rewind(fp);

    char *memory = (char*)malloc(filesize + 1);
    if (!memory) {
        printf("memory allocation failed\n");
        return NULL;
    }

    int c, prev_space = 0;
    size_t idx = 0;

    while ((c = fgetc(fp)) != EOF) {
        char ch = (char)c;

        if (is_corruption(ch)) continue;

        if (ch == '\n' || ch == '\r' || ch == '\t')
            ch = ' ';

        if (ch == ' ') {
            if (prev_space) continue;
            prev_space = 1;
        } else {
            prev_space = 0;
        }

        memory[idx++] = ch;
    }

    memory[idx] = '\0';
    return memory;
}

char* trim_and_copy(const char* a, const char* b)
{
    while (a < b && isspace((unsigned char)*a)) a++;
    while (b > a && isspace((unsigned char)*(b - 1))) b--;

    size_t n = (size_t)(b - a);
    char *memory = (char*)malloc(n + 1);
    if (!memory) {
        printf("memory allocation failed\n");
        return NULL;
    }

    memcpy(memory, a, n);
    memory[n] = '\0';
    return memory;
}

char *extract_entry(char *fp, char **firstname,char **secondname, char **fingerprint, char **position)
{
    const char* F1 = "First Name:";
    const char* F2 = "Second Name:";
    const char* F3 = "Fingerprint:";
    const char* F4 = "Position:";
    
    const char* a = strstr(fp, F1); if (!a) return NULL;
    const char* b = strstr(a + strlen(F1), F2); if (!b) return NULL;
    const char* c = strstr(b + strlen(F2), F3); if (!c) return NULL;
    const char* d = strstr(c + strlen(F3), F4); if (!d) return NULL;

    const char* nexta = strstr(d + strlen(F4), F1);
    const char* end = nexta ? nexta : (d + strlen(d));

    *firstname   = trim_and_copy(a + strlen(F1), b);
    *secondname  = trim_and_copy(b + strlen(F2), c);
    *fingerprint = trim_and_copy(c + strlen(F3), d);
    *position    = trim_and_copy(d + strlen(F4), end);   // FIXED

    if (!*firstname || !*secondname || !*fingerprint || !*position) {
        free(*firstname);
        free(*secondname);
        free(*fingerprint);
        free(*position);
        return NULL;
    }

    return (char*)end;
}

typedef struct {
    char *fn, *sn, *fp, *pos;
} Entry;

int seen_before(char **saved_fps, int count, const char *fp)
{
    for (int i = 0; i < count; i++)
        if (strcmp(saved_fps[i], fp) == 0) return 1;
    return 0;
}

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
    if (argc != 3) return 0;

    FILE *in = fopen(argv[1], "r");
    if (!in) {
        printf("Error opening file: %s\n", argv[1]);
        return 0;
    }

    char *clean = clean_up_file(in);
    fclose(in);
    if (!clean) return 0;

    Entry boss = {0}, righthand = {0}, lefthand = {0};
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

        if (seen_before(saved_fps, saved_count, e.fp)) {
            FREE_ENTRY(e.fn, e.sn, e.fp, e.pos);
            continue;
        }

        if (saved_count == saved_cap) {
            saved_cap = saved_cap ? saved_cap * 2 : 8;
            char **tmp = realloc(saved_fps, saved_cap * sizeof(*saved_fps));
            if (!tmp) { printf("memory allocation failed\n"); free(clean); return 0; }  /* FIX 2 */
            saved_fps = tmp;
        }

        saved_fps[saved_count] = malloc(strlen(e.fp) + 1);
        if (!saved_fps[saved_count]) { printf("memory allocation failed\n"); free(clean); return 0; } /* FIX 2 */
        strcpy(saved_fps[saved_count++], e.fp);

        if (!strcmp(e.pos, "Boss") && !boss.fp) boss = e;
        else if (!strcmp(e.pos, "Right Hand") && !righthand.fp) righthand = e;
        else if (!strcmp(e.pos, "Left Hand") && !lefthand.fp) lefthand = e;
        else if (!strcmp(e.pos, "Support_Right")) {
            if (supR_count == supR_cap) {
                supR_cap = supR_cap ? supR_cap * 2 : 8;
                Entry *tmp = realloc(supR, supR_cap * sizeof(*supR));
                if (!tmp) { printf("memory allocation failed\n"); free(clean); return 0; } /* FIX 2 */
                supR = tmp;
            }
            supR[supR_count++] = e;
        }
        else if (!strcmp(e.pos, "Support_Left")) {
            if (supL_count == supL_cap) {
                supL_cap = supL_cap ? supL_cap * 2 : 8;
                Entry *tmp = realloc(supL, supL_cap * sizeof(*supL));
                if (!tmp) { printf("memory allocation failed\n"); free(clean); return 0; } /* FIX 2 */
                supL = tmp;
            }
            supL[supL_count++] = e;
        }
        else {
            FREE_ENTRY(e.fn, e.sn, e.fp, e.pos);
        }
    }

    FILE *out = fopen(argv[2], "w");
    if (!out) {
        printf("Error opening file: %s\n", argv[2]);
        free(clean); /* FIX 1 */
        return 0;
    }

    if (boss.fp) print_entry(out, boss);
    if (righthand.fp) print_entry(out, righthand);
    if (lefthand.fp) print_entry(out, lefthand);
    for (int i = 0; i < supR_count; i++) print_entry(out, supR[i]);
    for (int i = 0; i < supL_count; i++) print_entry(out, supL[i]);

    fclose(out);

    FREE_ENTRY(boss.fn, boss.sn, boss.fp, boss.pos);
    FREE_ENTRY(righthand.fn, righthand.sn, righthand.fp, righthand.pos);
    FREE_ENTRY(lefthand.fn, lefthand.sn, lefthand.fp, lefthand.pos);

    for (int i = 0; i < supR_count; i++)
        FREE_ENTRY(supR[i].fn, supR[i].sn, supR[i].fp, supR[i].pos);
    for (int i = 0; i < supL_count; i++)
        FREE_ENTRY(supL[i].fn, supL[i].sn, supL[i].fp, supL[i].pos);

    free(supR);
    free(supL);

    for (int i = 0; i < saved_count; i++)
        free(saved_fps[i]);
    free(saved_fps);

    free(clean);

    return 0;

}

    

