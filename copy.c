#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>

/*
Implémentation d'un équivalent simplifié de la commande de copie de fichier cp
*/

/* copie un fichier
@param 1 adresse du fichier source, 
@param2 adresse du fichier cible
*/
int copieFichier(char filename[], char dest[] ) { 
    int fichiersource = open(filename, O_RDONLY); // on ouvre le fichier source en lecture

    if(fichiersource < 0) {
        printf("impossible d'ouvrir le fichier source ! \n");
        exit(EXIT_FAILURE);
    }

    int fichiercible = open(dest, O_WRONLY | O_CREAT); // crée (s'il n'existe pas) et ouvre le fichier cible (ecriture)

    if(fichiercible < 0) {
        printf("impossible d'ouvrir le fichier cible ! \n");
        exit(EXIT_FAILURE);
    }

    //taille du buffer proportionnelle à la taille des pages (4ko)
    int bufferSize = 4096;
    char *buffer = malloc(bufferSize); // buffer de 4096 o = 32 ko

    while(read(fichiersource, buffer,bufferSize) != 0) {
        write(fichiercible, buffer, bufferSize);
        bzero(buffer, bufferSize);
    }

    // on récupère les droits du fichier source
    struct stat stats; // buffer qui va collecter les stats du fichier source
    if(stat(filename, &stats) == -1) {
        perror("erreur stats");
        exit(EXIT_FAILURE);
    }

    // copie des droits dans le fichier cible
    int tmp = chmod(filename, stats.st_mode);
    if(tmp == -1) {
        perror("erreur copie droits d'accès");
        exit(EXIT_FAILURE);
    }

    free(buffer); // on libère l'espace alloue

    //fermeture des fichiers
    close(fichiersource);
    close(fichiercible);

    return 1;
}

/*
cree un chemin dans @path à partir du repertoire et du fichier passés en paramètre
*/
int creerChemin(char *dossier, char *fichier, char *path) {
    strcpy(path,dossier);
    if(strcmp(&dossier[strlen(dossier)-1],"/")) { //si le nom du repertoire ne finit pas par '/', on l'ajoute
        strcat(path,"/");
    }
    strcat(path,fichier);
    return 1;
}

/*
copie le contenu d'un répertoire dans un repertoire cible 
@param sourcename : chemin du repertoire source
@destname : chemin du repertoire cible
*/
int copieRepertoire(char* sourcename, char* destname) {
    DIR* dossiersrc = opendir(sourcename); // on ouvre le répertoire source
    if(dossiersrc < 0) {
        perror("erreur ouverture répertoire");
        exit(EXIT_FAILURE);
    }

    struct dirent* f = NULL;

    while((f = readdir(dossiersrc)) != NULL ) {
        char *nom = f->d_name;
        if( !strcmp(nom, ".") || !strcmp(nom,"..") || !strcmp(nom,".DS_Store")) { // si le fichier lu est "." ou ".." on lit le suivant
            continue;
        }

        //on crée le chemin du fichier source (dans la variable cheminsrc)
        char *cheminsrc = malloc(strlen(sourcename)+strlen(nom)+2);
        creerChemin(sourcename,nom,cheminsrc);

        //on crée le chemin du fichier cible (dans la variable chemindst)
        char *chemindst = malloc(strlen(destname)+strlen(nom)+2);
        creerChemin(destname,nom,chemindst);

        //on copie le fichier vers le répertoire cible
        copieFichier(cheminsrc,chemindst);

        free(cheminsrc); // on libère l'espace alloue
        free(chemindst);

    }

    closedir(dossiersrc); // fermeture du répertoire source
    return 1;

}

int isFile(char *path) { //retourne > 0 si le chemin correspond à un fichier
    struct stat stats;
    stat(path, &stats);
    return S_ISREG(stats.st_mode);
}

int isDirectory(char *path) { //retourne > 0 si le chemin correspond à un repertoire
    struct stat stats;
    stat(path, &stats);
    return S_ISDIR(stats.st_mode);
}

/*
copie récursive (fichiers et répertoires) 
@param sourcename : chemin du fichier/repertoire source
@param destname : "                            " cible
*/
int copieRec(char* sourcename, char* destname) {
    if(isFile(sourcename)) { // cas de base : le chemin correspond à un fichier
        copieFichier(sourcename,destname);
    } else if(isDirectory(sourcename)) { // cas récursif : le chemin correspond à un répertoire
        DIR* dossiersrc = opendir(sourcename);
        if(dossiersrc < 0) {
            perror("erreur ouverture répertoire");
            exit(EXIT_FAILURE);
        }
        //on récupère les droits du répertoire
        struct stat stats; 
        stat(sourcename,&stats);
        
        mkdir(destname,stats.st_mode); //on crée le répertoire cible avec les mêmes droits

        struct dirent* f = NULL;

        while((f = readdir(dossiersrc)) != NULL) {
            char *nom = f->d_name;

            if( strcmp(nom, ".") && strcmp(nom,"..") && strcmp(nom,".DS_Store")) { // si le fichier lu n'est pas "." ou "..."

                char *cheminsrc = malloc(strlen(sourcename)+strlen(nom)+2); // chemin du fichier source
                char *chemindst = malloc(strlen(destname)+strlen(nom)+2); // chemin du fichier cible (copie)

                // on cree les chemins source et cible
                creerChemin(sourcename,nom,cheminsrc); 
                creerChemin(destname,nom,chemindst);

                copieRec(cheminsrc,chemindst); // on copie récursivement le fichier

                free(cheminsrc); // on libère l'espace alloue
                free(chemindst);
            }

        } closedir(dossiersrc);
    } else { // dernier cas : le chemin ne correspond à un autre élément
        printf("ce chemin ne correspond ni à un repertoire, ni à un fichier : %s",sourcename);
    }

    return 1;
}


int main(int argc, char *argv[]) {
    //copieFichier(argv[1],argv[2]);
    //copieRepertoire(argv[1],argv[2]);
    copieRec(argv[1],argv[2]);
    
    return 0;

}