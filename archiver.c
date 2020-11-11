// Copyright 2019 Dragne Lavinia-Stefana 314CA
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include<time.h>
#define N_MAX 512
#define N_LUN 100
#define N_LUN1 32
#define N_LUN2 12
#define N_LUN3 8
union record {
    char charptr[N_MAX];
    struct header {
        char name[N_LUN];
        char mode[N_LUN3];
        char uid[N_LUN3];
        char gid[N_LUN3];
        char size[N_LUN2];
        char mtime[N_LUN2];
        char chksum[N_LUN3];
        char typeflag;
        char linkname[N_LUN];
        char magic[N_LUN3];
        char uname[N_LUN1];
        char gname[N_LUN1];
        char devmajor[N_LUN3];
        char devminor[N_LUN3];
    }
    header;
};
int permission(char caracter) {
    int u = 0;
    if (caracter == 'r') u = 4;
    else if (caracter == 'w') u = 2;
    else if (caracter == 'x') u = 1;
    return u;
}
// Functie pentru calculat permisiunile, asociez fiecarui caracter posibil
// numarul corespunzator in octal
void cai_fisiere(char *director, char *files, char *usermap) {
        strcpy(files, "./");
        strcat(files, director);
        strcat(files, "files.txt");
        strcpy(usermap, "./");
        strcat(usermap, director);
        strcat(usermap, "usermap.txt");
}
// Functie pentru concatenarea fisierelor, din care extrag datele,
// si calea catre ele
int chksum(union record arch) {
        int dim = 0, i;
        for (i = 0; i < N_LUN; i++) dim = dim + arch.header.name[i];
        for (i = 0; i < N_LUN3; i++) dim = dim + arch.header.mode[i];
        for (i = 0; i < N_LUN3; i++) dim = dim + arch.header.uid[i];
        for (i = 0; i < N_LUN3; i++) dim = dim + arch.header.gid[i];
        for (i = 0; i < N_LUN2; i++) dim = dim + arch.header.size[i];
        for (i = 0; i < N_LUN2; i++) dim = dim + arch.header.mtime[i];
        dim += N_LUN3 * 32;
        dim += arch.header.typeflag;
        for (i = 0; i < N_LUN; i++) dim = dim + arch.header.linkname[i];
        for (i = 0; i < N_LUN3; i++) dim = dim + arch.header.magic[i];
        for (i = 0; i < N_LUN1; i++) dim = dim + arch.header.uname[i];
        for (i = 0; i < N_LUN1; i++) dim = dim + arch.header.gname[i];
        for (i = 0; i < N_LUN3; i++) dim = dim + arch.header.devmajor[i];
        for (i = 0; i < N_LUN3; i++) dim = dim + arch.header.devminor[i];
        return dim;
}
// Functie pentru calculat chksum
void write_archive(union record arch, FILE *f)
{       fwrite(arch.header.name, N_LUN, 1, f);
        fwrite(arch.header.mode, N_LUN3, 1, f);
        fwrite(arch.header.uid, N_LUN3, 1, f);
        fwrite(arch.header.gid, N_LUN3, 1, f);
        fwrite(arch.header.size, N_LUN2, 1, f);
        fwrite(arch.header.mtime, N_LUN2, 1, f);
        fwrite(arch.header.chksum, N_LUN3, 1, f);
        fwrite("0", 1, 1, f);
        fwrite(arch.header.linkname, N_LUN, 1, f);
        fwrite(arch.header.magic, N_LUN3, 1, f);
        fwrite(arch.header.uname, N_LUN1, 1, f);
        fwrite(arch.header.gname, N_LUN1, 1, f);
        fwrite(arch.header.devmajor, N_LUN3, 1, f);
        fwrite(arch.header.devminor, N_LUN3, 1, f);
        int i;
        for (i = 0; i < 167; i++) fwrite("\0", 1, 1, f);
}
// Functie de scris header-ul in arhiva
void create(char comanda[N_MAX]) {
    int ok = 1;
    char * arhiva, * director, * p, files[N_MAX], usermap[N_MAX];
    p = strtok(comanda, " \n");
    if (p != NULL) { p = strtok(NULL, " ");}
    while (p != NULL && ok <= 2) {
        if (ok == 1) arhiva = strdup(p);
        if (ok == 2) director = strdup(p);
        ok++;
        p = strtok(NULL, " \n");
    }
    if (p != NULL || ok != 3) {
        printf("> Wrong command!\n"); return;
    } else {
        // Daca comanda a fost data cu succes
        cai_fisiere(director, files, usermap);
        FILE * f = fopen(arhiva, "wb");
        if (f) {
            FILE * g = fopen(files, "rt");
            FILE * h = fopen(usermap, "rt");
            if (g == NULL || h == NULL) {
                printf("> Failed!\n"); return;
            }
            // Daca fiserele "files" si "usermap" au fost deschise
            // cu succes, continui algoritmul
            union record arch; int i; char * r;
            char * line = malloc(N_MAX * sizeof(char));
            while (fgets(line, N_MAX, g) && strcmp((line), "\n") != 0) {
                // Cat timp fisierul "files" mai are linii necitite,
                // continui algoritmul
                arch.header.mode[0] = '0';
                arch.header.mode[1] = '0';
                arch.header.mode[2] = '0';
                arch.header.mode[3] = '0';
                // Initializez primele 4 char-uri din mode cu 0
                int j = 4; long long dimd;
                for (i = 1; i <= 9; i = i + 3) {
                    arch.header.mode[j] = (permission(line[i])) +
                    (permission(line[i + 1])) + (permission(line[i + 2])) + '0';
                    j++;
                }
                // Calculez permisiunile pentru fiecare fisier, iar pe ultima
                // pozitie din mode, pun '\0'
                arch.header.mode[7] = '\0';
                r = strtok(line, " ");
                int nr = 1; long ret; struct tm info = { 0 };
                while (r != NULL) {
                    if (nr == 3) {
                        strcpy(arch.header.uname, r);
                        // Al treilea camp din files e uname-ul
                        for (i = strlen(arch.header.uname); i < N_LUN1; i++)
                            arch.header.uname[i] = '\0';
                        // Completez uname-ul cu '\0' pana la 32
                    }
                    if (nr == 4) {
                        strcpy(arch.header.gname, r);
                        // Al patrulea camp din files e gname-ul
                        for (i = strlen(arch.header.gname); i < N_LUN1; i++)
                            arch.header.gname[i] = '\0';
                            // Completez gname-ul cu '\0' pana la 32
                    }
                    if (nr == 5) {
                        dimd = atol(r);
                        // Al cincilea camp din files e size-ul, ce trebuie
                        // transformat in octal
                        sprintf(arch.header.size, "%011llo", dimd);
                        arch.header.size[11] = '\0';
                        // Completez size-ul cu '\0' pana la 12
                    }
                    if (nr == 6) {
                        info.tm_year = ((r[0] - '0') * 1000 + (r[1] - '0') *
                        100 + (r[2] - '0') * 10 + (r[3] - '0')) - 1900;
                        info.tm_mon = ((r[5] - '0') * 10 + (r[6] - '0')) - 1;
                        info.tm_mday = (r[8] - '0') * 10 + (r[9] - '0');
                        // Al saselea camp din files e anul-luna-ziua ultimei
                        // modificari; ele trebuie modificate "aparent" din
                        // char-uri, in int-uri, tinand cont de codul lor ASCII
                    }
                    if (nr == 7) {
                        info.tm_hour = (r[0] - '0') * 10 + (r[1] - '0');
                        info.tm_min = (r[3] - '0') * 10 + (r[4] - '0');
                        info.tm_sec = (r[6] - '0') * 10 + (r[7] - '0');
                        // Al saptelea camp din files e ora-min-secunda ultimei
                        // modificari; ele trebuie modificate "aparent" din
                        // char-uri, in int-uri, tinand cont de codul lor ASCII
                        ret = mktime(& info);
                        if (ret != -1) {
                            sprintf(arch.header.mtime, "%lo", ret);
                        }
                    }
                    if (nr == 9) {
                        strcpy(arch.header.name, r);
                        strcpy(arch.header.linkname, r);
                        // Al noulea camp din files e name-ul, ce este identic
                        // cu linkname-ul
                        for (i = strlen(arch.header.name); i < N_LUN; i++) {
                           arch.header.name[i] = arch.header.linkname[i] = '\0';
                           // Completez name-ul si linkname-ul cu '\0'
                           // pana la 100
                        }
                    }
                    nr++;
                    r = strtok(NULL, " \n");
                }
                arch.header.typeflag = '0';
                strcpy(arch.header.magic, "GNUtar ");
                arch.header.magic[7] = '\0';
                for (i = 0; i < N_LUN3; i++) {
                    arch.header.devminor[i] = arch.header.devmajor[i] = '\0';
                }
                // Completez typeflag, magic, devminor si devmajor cu valorile
                // implicite date de enunt
                char * line1 = malloc(N_MAX * sizeof(char));
                int ok1 = 1;
                while (fgets(line1, N_MAX, h) && ok1 == 1) {
                    // Cat timp n-am gasit name-ul cautat, in fisierul usermap,
                    // si fisierul nu este la final, continui algoritmul
                    r = strtok(line1, ":");
                    // Separatorul va fi ":"
                    if (strlen(r) == strlen(arch.header.uname)) {
                        if (strcmp(r, arch.header.uname) == 0) {
                            ok1 = 0; nr = 1;
                            while (r != NULL && nr <= 4) {
                                if (nr == 3) {
                                    sprintf(arch.header.uid, "%07o", atoi(r));
                                    arch.header.uid[7] = '\0';
                                    // Al treilea camp din usermap este uid-ul
                                    // ce trebuie convertit in octal
                                }
                                if (nr == 4) {
                                    sprintf(arch.header.gid, "%07o", atoi(r));
                                    arch.header.gid[7] = '\0';
                                    // Al treilea camp din usermap este gid-ul
                                    // ce trebuie convertit in octal
                                }
                                nr++;
                                r = strtok(NULL, ":");
                            }
                        }
                    }
                }
                free(line1);
                int dim = chksum(arch);
                sprintf(arch.header.chksum, "%06o", dim);
                arch.header.chksum[6] = '\0'; arch.header.chksum[7] = ' ';
                // Completez chksum-ul cu suma tuturor campurilor din header
                // utilizand functia chksum
                fseek(h, 0, SEEK_SET);
                // Pozitionez cursorul la inceputul fisierului usermap
                write_archive(arch, f);
                // Scriu tot header-ul in arhiva
                char nume[N_LUN];
                strcpy(nume, "./");
                strcat(nume, director);
                strcat(nume, arch.header.name);
                FILE * k = fopen(nume, "rt");
                // Deschid fisierul ce trebuie inclus in arhiva, concatenand
                // calea catre el
                if (k) {
                    // Daca fisierul exista si a fost deschis cu succes
                    // continui algoritmul
                    int x = dimd / N_MAX;
                    // Calculez numarul de blocuri de date intregi de 512
                    for (i = 0; i <= x; i++) {
                        for (j = 0; j < N_MAX; j++) {
                            arch.charptr[j] = '\0';
                        }
                        // Completez cu '\0' pana la multiplu de 512 blocul
                        // de date
                        fread(arch.charptr, N_MAX, 1, k);
                        // Citesc datele din fisierul ce trebuie arhivat
                        fwrite(arch.charptr, N_MAX, 1, f);
                        // Scriu in arhiva datele din fisierul de arhivat
                    }
                    fclose(k);
                } else {
                    printf("> Failed!\n"); return;
                }
            }
            for (i = 0; i < N_MAX; i++) fwrite("\0", 1, 1, f);
                // Finalul arhivei il marchez de o zonă de 512 octeti de 0
            printf("> Done!\n");
            fclose(f); fclose(g); fclose(h);
            // Inchid toate fisierele
            free(arhiva); free(director); free(line);
            // Eliberez memoria
        } else {
            printf("> Failed!\n"); return;
        }
    }
}
int decimal(char * size1) {
    long dim = 0;
    long nr = atol(size1);
    int baza = 1;
    int ultcifra;
    while (nr) {
        ultcifra = nr % 10;
        nr = nr / 10;
        dim = dim + ultcifra * baza;
        baza = baza * 8;
    }
    return dim;
}
// Functie de transformat din baza 8 in
// baza 10
void list(char comanda[N_MAX]) {
    int ok = 1;
    char * arhiva, * p;
    p = strtok(comanda, " \n");
    if (p != NULL) p = strtok(NULL, " \n");
    while (p != NULL && ok <= 1) {
        if (ok == 1) arhiva = strdup(p);
        ok++;
        p = strtok(NULL, " \n");
    }
    // Tai comanda primita in cuvinte pentru a
    // identifica numele arhivei
    if (p != NULL || ok != 2) {
        printf("> Wrong command!\n");
        return;
    } else {
        FILE * f = fopen(arhiva, "rb");
        if (f == NULL) {
            printf("> File not found!\n");
            return;
        }
        // Daca arhiva exista si a fost deschisa cu succes
        // continui algoritmul
        char * name = malloc(N_LUN * sizeof(char));
        char * size = malloc(N_LUN2 * sizeof(char));
        fread(name, N_LUN, 1, f);
        int poz = N_LUN;
        long dim;
        int nrbloc;
        while (strcmp(name, "\0") != 0) {
            printf("> %s\n", name);
            poz += 2*N_LUN2;
            fseek(f, poz, SEEK_SET);
            fread(size, N_LUN2, 1, f);
            poz += N_LUN2;
            dim = decimal(size);
            nrbloc = dim / N_MAX + 1;
            poz = poz + 376 + nrbloc * N_MAX;
            fseek(f, poz, SEEK_SET);
            fread(name, N_LUN, 1, f);
            poz += N_LUN;
            // Folosindu-ma de size sar cu cursorul peste blocuri de 512
            // si ma pozitionez de fiecare data pe name-ul fiecarui
            // fisier din arhiva
        }
        free(arhiva);
        free(name);
        free(size);
        // Eliberez memoria
        fclose(f);
        // Inchid arhiva
    }
}
void extract(char comanda[N_MAX]) {
    int ok = 1;
    char * fisier, * arhiva, * p;
    p = strtok(comanda, " \n");
    if (p != NULL) p = strtok(NULL, " \n");
    while (p != NULL && ok <= 2) {
        if (ok == 1) fisier = strdup(p);
        if (ok == 2) arhiva = strdup(p);
        ok++;
        p = strtok(NULL, " \n");
    }
    // Tai comanda primita in cuvinte pentru a
    // identifica numele fisierului ce trebuie dezarhivat
    // si numele arhivei din care trebuie extras
    if (p != NULL || ok != 3) {
        printf("> Wrong command!\n"); return;
    } else {
        // Daca comanda a fost corecta
        FILE * f = fopen(arhiva, "rb");
        if (f == NULL) {
            printf("> File not found!\n");
            return;
        }
        // Daca arhiva a fost deschisa cu succes incep prelucrarea
        char * name = malloc(N_LUN * sizeof(char));
        char * size = malloc(N_LUN2 * sizeof(char));
        fread(name, N_LUN, 1, f);
        int poz = N_LUN;
        long dim;
        int nrbloc;
        int ok1 = 0;
        while (ok1 == 0 && strcmp(name, "\0") != 0) {
            // Continui sa caut fisierul in arhiva, pana la finalul ei
            if (strcmp(name, fisier) == 0 && strlen(fisier) == strlen(name)) {
                ok1 = 1;
                break;
                // Daca am gasit fisierul, opresc cautarea
            }
            poz += 2*N_LUN2;
            fseek(f, poz, SEEK_SET);
            fread(size, N_LUN2, 1, f);
            poz += N_LUN2;
            dim = decimal(size);
            nrbloc = dim / N_MAX + 1;
            poz = poz + 376 + nrbloc * N_MAX;
            fseek(f, poz, SEEK_SET);
            fread(name, N_LUN, 1, f);
            poz += N_LUN;
            // Ma pozitionez cu cursorul in arhiva de fiecare data pe
            // campul destinat memorarii numelui
        }
        if (ok1 == 0) {
            printf("> File not found!\n");
            return;
        }
        char extras[N_LUN+10];
        strcpy(extras, "extracted_");
        strcat(extras, fisier);
        FILE * g = fopen(extras, "wb");
        // Deschid un fisier pentru a-l dezarhiva
        poz += 2*N_LUN2;
        fseek(f, poz, SEEK_SET);
        fread(size, N_LUN2, 1, f);
        poz += N_LUN2;
        dim = decimal(size);
        // Transform size-ul din char in intreg, in baza 10
        poz = poz + 376;
        fseek(f, poz, SEEK_SET);
        char date[N_MAX];
        int x = dim / N_MAX, i;
        // Calculez numarul de blocuri intregi de date de 512 ce exista
        int rest = dim - x * N_MAX;
        for (i = 0; i < x; i++) {
            fread(date, N_MAX, 1, f);
            fwrite(date, N_MAX, 1, g);
        }
        fread(date, rest, 1, f);
        fwrite(date, rest, 1, g);
        // Scriu in fisierul extras toate blocurile de date intregi, iar
        // apoi si restul, ce nu sunt intr-un bloc de 512 complet
        printf("> File extracted!\n");
        free(arhiva);
        free(fisier);
        free(name);
        free(size);
        // Eliberez memoria
        fclose(f);
        fclose(g);
        // Inchid arhiva si fisierul extras
    }
}
int main() {
    char comanda[N_MAX], copie[N_MAX];
    fgets(comanda, N_MAX, stdin);
    char * p;
    strcpy(copie, comanda);
    p = strtok(copie, " \n");
    if (p == NULL) {
        printf("> Wrong command!\n");
    } else {
        while (strcmp(comanda, "exit\n") != 0) {
            // Asteapta comenzi pana la primirea lui
            // "exit"
            if (strcmp(p, "create") == 0) {
                create(comanda);
            } else if (strcmp(p, "list") == 0) {
                list(comanda);
            } else if (strcmp(p, "extract") == 0) {
                extract(comanda);
            } else {
                printf("> Wrong command!\n");
            }
            fgets(comanda, N_MAX, stdin);
            strcpy(copie, comanda);
            p = strtok(copie, " \n");
            while (p == NULL) {
                printf("> Wrong command!\n");
                fgets(comanda, N_MAX, stdin);
                strcpy(copie, comanda);
                p = strtok(copie, " \n");
            }
            // Daca comanda este "\n" este gresita si
            // asteapta o comanda noua
        }
    }
    return 0;
}
