#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "org_tree.h"
#define LEN 256
// trims white spaces for field values

static void trim(char *s) 
{
    int i = 0, j;
    while (isspace((unsigned char)s[i]))
    {
        i++;
    } 
    if (i) memmove(s, s + i, strlen(s + i) + 1);
    j = (int)strlen(s) - 1;
    while (j >= 0 && isspace((unsigned char)s[j]))
    {
        s[j] = '\0';
        j -= 1;
    } 
}
 
static void append(Node **head, Node **tail, Node *n)
 {
    n->next = NULL;
    if (!*head)
    {
        *head = *tail = n;
    } 
    else { (*tail)->next = n; *tail = n; }
}

void extract_value(char *str, int size, const char *line)
{
    const char *p = strchr(line, ':');

    // move past ':' if it exists
    if (p)
        p++;
    else
        p = line;

    // skip spaces after ':'
    while (*p && isspace((unsigned char)*p))
    {
        p++;
    }    
    strncpy(str, p, size - 1);
    str[size - 1] = '\0';
    trim(str);
}

static Node *read_entry(FILE *fp) 
{
    char a[LEN], b[LEN], c[LEN], d[LEN];

    // skip blank lines
    do {
        if (!fgets(a, sizeof(a), fp)) return NULL;
        trim(a);
    } while (a[0] == '\0');

    if (!fgets(b, sizeof(b), fp)) return NULL;
    if (!fgets(c, sizeof(c), fp)) return NULL;
    if (!fgets(d, sizeof(d), fp)) return NULL;

    trim(b); trim(c); trim(d);

    Node *n = NULL;
    n = (Node*)malloc(sizeof(Node));
    if (!n) return NULL;

    extract_value(n->first,       sizeof(n->first),       a);
    extract_value(n->second,      sizeof(n->second),      b);
    extract_value(n->fingerprint, sizeof(n->fingerprint), c);
    extract_value(n->position,    sizeof(n->position),    d);

    return n;
}

Org build_org_from_clean_file(const char *path) 
{
    // TODO: implement
    Org org = {0};
    FILE *fp = fopen(path, "r");
    if (!fp) return org;

    Node *left_tail = NULL, *right_tail = NULL;

    // supports that appear before their Hand
    Node *pendL_h = NULL, *pendL_t = NULL;
    Node *pendR_h = NULL, *pendR_t = NULL;
    Node* n = NULL;
    n = (Node*)malloc(sizeof(Node));
    if (n == NULL)
    {
        return org;
    }

    while (n != NULL) 
    {
        if (strcmp(n->position, "Boss") == 0) 
        {
            org.boss = n;
        } 
        else if (strcmp(n->position, "Left Hand") == 0) 
        {
            org.left_hand = n;
            if (pendL_h) 
            {
                n->supports_head = pendL_h;
                left_tail = pendL_t;
                pendL_h = pendL_t = NULL;
            }
        } 
        else if (strcmp(n->position, "Right Hand") == 0) 
        {
            org.right_hand = n;
            if (pendR_h)
            {
                n->supports_head = pendR_h;
                right_tail = pendR_t;
                pendR_h = pendR_t = NULL;
            }
        } 
        else if (strcmp(n->position, "Support_Left") == 0) 
        {
            if (org.left_hand)
                append(&org.left_hand->supports_head, &left_tail, n);
            else
                append(&pendL_h, &pendL_t, n);
        } 
        else if (strcmp(n->position, "Support_Right") == 0) 
        {
            if (org.right_hand)
                append(&org.right_hand->supports_head, &right_tail, n);
            else
                append(&pendR_h, &pendR_t, n);
        } 
        else 
        {
            free(n);  // unknown position
        }

        n = read_entry(fp);  // read next entry
    }

    fclose(fp);

    if (org.boss) 
    {
        org.boss->left  = org.left_hand;
        org.boss->right = org.right_hand;
    }
    return org;
}
    

void print_tree_order(const Org *org) {
    // TODO: implement
}

void free_org(Org *org) {
    // TODO: implement
}

