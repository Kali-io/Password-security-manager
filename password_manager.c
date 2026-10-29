//Default password : admin123
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TABLE_SIZE 10
#define MAX 100
#define MASTER_HASH 407908580

struct Entry {
    char site[30];
    char username[30];
    unsigned int hash;
    char strength[10];
    struct Entry *next;
};

struct TrieNode {
    struct TrieNode *child[128];
    int isEnd;
};

struct Action {
    int type;
    struct Entry data;
};

struct Entry *table[TABLE_SIZE];
struct TrieNode *root;
struct Action stack[MAX];
int top = -1;
int total = 0;

void makeLower(char s[]) {
    int i;
    for (i = 0; s[i] != '\0'; i++)
        s[i] = tolower(s[i]);
}

unsigned int hashPassword(char pw[]) {
    unsigned int h = 5381;
    int i;
    for (i = 0; pw[i] != '\0'; i++)
        h = h * 33 + pw[i];
    return h;
}

void checkStrength(char pw[], char result[]) {
    int hasDigit = 0, hasLetter = 0, i;
    for (i = 0; pw[i] != '\0'; i++) {
        if (isdigit(pw[i])) hasDigit = 1;
        if (isalpha(pw[i])) hasLetter = 1;
    }
    if (strlen(pw) >= 8 && hasDigit && hasLetter)
        strcpy(result, "Strong");
    else if (strlen(pw) >= 6)
        strcpy(result, "Medium");
    else
        strcpy(result, "Weak");
}

void showEntry(struct Entry *e) {
    printf("\nSite     : %s\n", e->site);
    printf("Username : %s\n", e->username);
    printf("Hash     : %u\n", e->hash);
    printf("Strength : %s\n", e->strength);
}

int tableIndex(char site[]) {
    unsigned int sum = 0;
    int i;
    for (i = 0; site[i] != '\0'; i++)
        sum = sum + site[i];
    return sum % TABLE_SIZE;
}

struct Entry *findSite(char site[]) {
    struct Entry *p = table[tableIndex(site)];
    while (p != NULL) {
        if (strcmp(p->site, site) == 0)
            return p;
        p = p->next;
    }
    return NULL;
}

void insertTable(struct Entry *e) {
    int i = tableIndex(e->site);
    e->next = table[i];
    table[i] = e;
    total++;
}

void deleteFromTable(char site[]) {
    int i = tableIndex(site);
    struct Entry *p = table[i];
    struct Entry *prev = NULL;
    while (p != NULL) {
        if (strcmp(p->site, site) == 0) {
            if (prev == NULL)
                table[i] = p->next;
            else
                prev->next = p->next;
            free(p);
            total--;
            return;
        }
        prev = p;
        p = p->next;
    }
}

struct TrieNode *newNode() {
    struct TrieNode *n = malloc(sizeof(struct TrieNode));
    int i;
    for (i = 0; i < 128; i++)
        n->child[i] = NULL;
    n->isEnd = 0;
    return n;
}

void insertTrie(char word[]) {
    struct TrieNode *cur = root;
    int i, c;
    for (i = 0; word[i] != '\0'; i++) {
        c = word[i];
        if (cur->child[c] == NULL)
            cur->child[c] = newNode();
        cur = cur->child[c];
    }
    cur->isEnd = 1;
}

void deleteTrie(char word[]) {
    struct TrieNode *cur = root;
    int i, c;
    for (i = 0; word[i] != '\0'; i++) {
        c = word[i];
        if (cur->child[c] == NULL)
            return;
        cur = cur->child[c];
    }
    cur->isEnd = 0;
}

void printWords(struct TrieNode *node, char word[], int len, int *count) {
    int c;
    if (node->isEnd == 1) {
        word[len] = '\0';
        (*count)++;
        printf("  %d. %s\n", *count, word);
    }
    for (c = 0; c < 128; c++) {
        if (node->child[c] != NULL) {
            word[len] = c;
            printWords(node->child[c], word, len + 1, count);
        }
    }
}

int suggest(char prefix[]) {
    struct TrieNode *cur = root;
    char word[30];
    int len = 0, count = 0, i, c;
    for (i = 0; prefix[i] != '\0'; i++) {
        c = prefix[i];
        if (cur->child[c] == NULL)
            return 0;
        cur = cur->child[c];
        word[len] = prefix[i];
        len++;
    }
    printWords(cur, word, len, &count);
    return count;
}

void push(int type, struct Entry *e) {
    if (top == MAX - 1)
        return;
    top++;
    stack[top].type = type;
    stack[top].data = *e;
}

void saveToFile() {
    FILE *fp = fopen("passwords.txt", "w");
    struct Entry *p;
    int i;
    if (fp == NULL) {
        printf("Could not save file.\n");
        return;
    }
    for (i = 0; i < TABLE_SIZE; i++) {
        for (p = table[i]; p != NULL; p = p->next)
            fprintf(fp, "%s %s %u %s\n", p->site, p->username, p->hash, p->strength);
    }
    fclose(fp);
}

void loadFromFile() {
    FILE *fp = fopen("passwords.txt", "r");
    struct Entry *e;
    char site[30], user[30], strength[10];
    unsigned int h;
    if (fp == NULL)
        return;
    while (fscanf(fp, "%29s %29s %u %9s", site, user, &h, strength) == 4) {
        e = malloc(sizeof(struct Entry));
        strcpy(e->site, site);
        strcpy(e->username, user);
        e->hash = h;
        strcpy(e->strength, strength);
        insertTable(e);
        insertTrie(e->site);
    }
    fclose(fp);
}

int login() {
    char pw[30];
    int t;
    for (t = 1; t <= 3; t++) {
        printf("Enter master password: ");
        scanf("%29s", pw);
        if (hashPassword(pw) == MASTER_HASH)
            return 1;
        printf("Wrong password (%d/3)\n", t);
    }
    return 0;
}

void addPassword() {
    char site[30], username[30], password[30];
    struct Entry *e;

    if (total >= MAX) {
        printf("Storage is full.\n");
        return;
    }
    printf("Enter site name : ");
    scanf("%29s", site);
    makeLower(site);
    if (findSite(site) != NULL) {
        printf("This site already exists. Use Update.\n");
        return;
    }
    printf("Enter username  : ");
    scanf("%29s", username);
    printf("Enter password  : ");
    scanf("%29s", password);

    e = malloc(sizeof(struct Entry));
    strcpy(e->site, site);
    strcpy(e->username, username);
    e->hash = hashPassword(password);
    checkStrength(password, e->strength);

    insertTable(e);
    insertTrie(site);
    push(1, e);

    printf("\nPassword saved. Strength: %s\n", e->strength);
    if (strcmp(e->strength, "Weak") == 0)
        printf("Warning: this password is weak!\n");
}

void searchPassword() {
    char site[30];
    struct Entry *e;

    printf("Enter site name (or first letters): ");
    scanf("%29s", site);
    makeLower(site);

    e = findSite(site);
    if (e != NULL) {
        showEntry(e);
        return;
    }
    printf("\nNo exact match. Suggestions:\n");
    if (suggest(site) == 0)
        printf("  Nothing found.\n");
    else
        printf("Search again with the full name.\n");
}

void merge(struct Entry *a[], int low, int mid, int high) {
    struct Entry *temp[MAX];
    int i = low, j = mid + 1, k = low;

    while (i <= mid && j <= high) {
        if (strcmp(a[i]->site, a[j]->site) <= 0)
            temp[k++] = a[i++];
        else
            temp[k++] = a[j++];
    }
    while (i <= mid)
        temp[k++] = a[i++];
    while (j <= high)
        temp[k++] = a[j++];
    for (i = low; i <= high; i++)
        a[i] = temp[i];
}

void mergeSort(struct Entry *a[], int low, int high) {
    int mid;
    if (low < high) {
        mid = (low + high) / 2;
        mergeSort(a, low, mid);
        mergeSort(a, mid + 1, high);
        merge(a, low, mid, high);
    }
}

void viewAll() {
    struct Entry *list[MAX];
    struct Entry *p;
    int n = 0, i;

    if (total == 0) {
        printf("\nNo passwords saved yet.\n");
        return;
    }
    for (i = 0; i < TABLE_SIZE; i++)
        for (p = table[i]; p != NULL; p = p->next)
            list[n++] = p;

    mergeSort(list, 0, n - 1);

    printf("\n%-4s %-20s %-20s %-8s\n", "No.", "Site", "Username", "Strength");
    for (i = 0; i < n; i++)
        printf("%-4d %-20s %-20s %-8s\n", i + 1, list[i]->site,
               list[i]->username, list[i]->strength);
}

void updatePassword() {
    char site[30], password[30];
    struct Entry *e;

    printf("Enter site name to update: ");
    scanf("%29s", site);
    makeLower(site);
    e = findSite(site);
    if (e == NULL) {
        printf("Site not found.\n");
        return;
    }
    printf("Enter new password: ");
    scanf("%29s", password);

    push(3, e);
    e->hash = hashPassword(password);
    checkStrength(password, e->strength);
    printf("\nPassword updated. Strength: %s\n", e->strength);
}

void deletePassword() {
    char site[30], ans;
    struct Entry *e;

    printf("Enter site name to delete: ");
    scanf("%29s", site);
    makeLower(site);
    e = findSite(site);
    if (e == NULL) {
        printf("Site not found.\n");
        return;
    }
    printf("Are you sure? (y/n): ");
    scanf(" %c", &ans);
    if (ans != 'y' && ans != 'Y') {
        printf("Cancelled.\n");
        return;
    }
    push(2, e);
    deleteTrie(site);
    deleteFromTable(site);
    printf("\nEntry deleted.\n");
}

void undoLast() {
    struct Action a;
    struct Entry *e;

    if (top == -1) {
        printf("\nNothing to undo.\n");
        return;
    }
    a = stack[top];
    top--;

    if (a.type == 1) {
        deleteTrie(a.data.site);
        deleteFromTable(a.data.site);
        printf("\nUndo: %s removed.\n", a.data.site);
    }
    else if (a.type == 2) {
        e = malloc(sizeof(struct Entry));
        *e = a.data;
        insertTable(e);
        insertTrie(e->site);
        printf("\nUndo: %s restored.\n", e->site);
    }
    else {
        e = findSite(a.data.site);
        e->hash = a.data.hash;
        strcpy(e->strength, a.data.strength);
        printf("\nUndo: old password of %s restored.\n", e->site);
    }
}

int main() {
    int choice;
    printf("Default password : admin123\n\n");

    root = newNode();

    if (login() == 0) {
        printf("Access denied.\n");
        return 0;
    }
    loadFromFile();

    while (1) {
        printf("\n==============================================\n");
        printf("          PASSWORD SECURITY MANAGER\n");
        printf("==============================================\n");
        printf(" 1. Add New Password\n");
        printf(" 2. Search Password\n");
        printf(" 3. View All Passwords (Sorted)\n");
        printf(" 4. Update Password\n");
        printf(" 5. Delete Password\n");
        printf(" 6. Undo Last Change\n");
        printf(" 7. Exit\n");
        printf("==============================================\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) {
            choice = 0;
            while (getchar() != '\n');
        }

        if (choice == 1)      addPassword();
        else if (choice == 2) searchPassword();
        else if (choice == 3) viewAll();
        else if (choice == 4) updatePassword();
        else if (choice == 5) deletePassword();
        else if (choice == 6) undoLast();
        else if (choice == 7) {
            saveToFile();
            printf("\nData saved. Goodbye!\n");
            break;
        }
        else
            printf("\nInvalid choice. Try again.\n");
    }
    return 0;
}
