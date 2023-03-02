#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_LENGTH 83

/**********************
 *
 * MODIFIEZ ET RENDEZ CE SEUL FICHIER COMME REPONSE
 *
 * INDIQUEZ VOTRE NOM ET PRENOM CI-DESSOUS:
 * DOZ:
 * Louka:
 *
 ********************/

/* Quelques fonctions d'aide vous sont fournies. Vous pouvez les utiliser pour lire une image PPM.
 *
 * N'hésitez pas si vous rencontrez un problème avec ces fonctions
 * */



/* Retourne 1 si s est un commentaire, 0 sinon */
int is_comment(char* s) {
    if(strlen(s) < 1)
        return 0;

    if(s[0] == '#')
        return 1;

    return 0;
}

/* Lis width et height dans s et les place respectivement dans pw et ph */
void read_width_height(char* s, int* pw, int* ph) {
    sscanf(s, "%d %d", pw, ph);
}

/* Renvoie un pointeur sur les 3w entiers dans s, dans le même ordre */
int* read_pixels(char* s, int w) {
    int* intergers = (int*) malloc(sizeof(int) * 3 * w);

    for(int i = 0, j = 0; j < 3 * w; i += 4, j++) {
        s[i + 3] = '\0';
        intergers[j] = strtol(&s[i], NULL, 10);
    }

    return intergers;
}

/**
 * Lit et renvoie la première ligne de l'entrée standard qui n'est pas un commentaire d'un fichier PPM.
 * On suppose qu'un fichier PPM a été donné en entrée standard. 
 * Tous les commentaires ignorés de l'entrée standard sont consommés.
 *
 * La chaîne qui est renvoyée a été réservée avec malloc et peut être libérée avec free dès que son utilisation n'est plus requise.
 */
char* read_line(){
    char * buffer = (char *) malloc( MAX_LENGTH * sizeof(char) );
    if(buffer == NULL)
        return NULL;
    while(1){
        fgets(buffer, MAX_LENGTH * sizeof(char), stdin);
        if(is_comment(buffer) == 1)
            continue;
        return buffer;
    }

}

/**
 * Renvoie vrai si c est un caractère représentant un chiffre et faux sinon
 */
int isnumber(char c){
    return c >= '0' && c <= '9';
}

/**
 * Lit le premier nombre présent dans la chaîne src et l'écrit au début de dst suivi d'un espace.
 * On suppose que ce nombre existe et est un entier inférieur à 999.
 * Le nombre est écrit dans dst avec 3 chiffres. S'il est inférieur à 100 alors il
 * est complété à gauche avec des 0.
 *
 * La fonction renvoie l'indice du premier caractère suivant ce nombre qui est un chiffre ou qui est '\0'.
 */
int copy_next_int(char* src, char* dst){
    dst[0] = '0';
    dst[1] = '0';
    dst[2] = '0';
    dst[3] = ' ';
    
    int start = 0;
    while(!isnumber(src[start]))
        start++;
    int end = start + 1;
    while(isnumber(src[end]))
        end++;

    for(int j = start; j < end; j++)
        dst[2 - (end - j - 1)] = src[j];

    while(!isnumber(src[end]) && src[end] != '\0')
        end++;
    return end;
}

/**
 * Cette fonction lit l'entrée standard et renvoie une chaîne contenant les 3w premiers entiers qu'elle lit.
 * On suppose qu'un fichier PPM a été donné en entrée standard et on suppose que les premières lignes de ce fichier
 * ont été consommées de sorte que la première ligne de l'entrée standard décrit le début d'une ligne de l'image.
 * On suppose que ce fichier est bien formé : aucune ligne ne fait plus de 80 caractères et il y a un saut de 
 * ligne après le dernier pixel de chaque ligne de l'image.
 * On suppose tous ces entiers inférieurs à 999. Il sont alors écrit dans la chaîne résultat avec 3 chiffres.
 *
 * Donner en entrée standard un fichier ne respectant pas ces consignes peut provoquer une erreur de segmentation.
 *
 * Tous les entiers inférieurs à 100 sont complétés à gauche avec des 0.
 * Tous les entier sont séparés par des espaces. 
 * Toutes les lignes de l'entrée standard lues sont consommées. 
 * 
 * La chaîne qui est renvoyée a été réservée avec malloc et peut être libérée avec free dès que son utilisation n'est plus requise.
 * 
 */
char* read_pixels_line(int w){
    char * buffer = (char *) malloc( MAX_LENGTH * sizeof(char) );
    if(buffer == NULL)
        return NULL;
    char *  pixels = (char *) malloc((12 * w + 1) * sizeof(char));
    pixels[12 * w] = '\0';
    if(pixels == NULL)
        return NULL;
    char* src;
    char * dst = pixels;
    int counter = 0;
    while(counter < 3 * w){
        fgets(buffer, MAX_LENGTH * sizeof(char), stdin);
        if(is_comment(buffer) == 1)
            continue;
        fflush(stdout);
    
        src = buffer;
        while(src[0] != '\0'){
            int index = copy_next_int(src, dst);
            src += index;
            dst += 4;
            counter += 1;
        }
    }
    free(buffer);

    return pixels;
}

/* lit une image ppm donnée en entrée standard 
et fait pointer pw et ph respectivement sur la largeur w et la hauteur h.
Renvoie un pointeur sur h pointeurs sur 3w entiers correspondant
aux différents pixels de l’image dans l’ordre */
int** read_ppm(int* pw, int* ph) {
    int** pixels;
    char* buffer;

    /* "P3" */
    buffer = read_line();
    free(buffer);

    /* Width height */
    buffer = read_line();
    read_width_height(buffer, pw, ph);
    free(buffer);

    /* Pixels */
    pixels = (int**) malloc(sizeof(int*) * (*ph));
    for(int i = 0; i < *ph; i++) {
        buffer = read_pixels_line(*pw);
        pixels[i] = read_pixels(buffer, *pw);
    	free(buffer);
    }

    return pixels;
}

/* Renvoie x / 12.92 si x <= 0.04045, ((x+0.055)/1055)^2.4 sinon */
float lin(float x) {
    if(x <= 0.04045)
        return x / 12.92;
    
    return pow((x + 0.055) / 1.055, 2.4);
}

/* Renvoie 0.2126 * lin(r / 255) + 0.7152 * lin(g / 255) + 0.0722 * lin(b / 255) */
float mean(int r, int g, int b) {
    return 0.2126 * lin(r / 255) + 0.7152 * lin(g / 255) + 0.0722 * lin(b / 255);
}

/* Calcule m = mean(r, g, b)
Renvoie 12.92 ∗ m si m <= 0.0031308, 1.055 * (m^(1/2.4)) − 0.055 sinon */
float lum(int r, int g, int b) {
    float m = mean(r, g ,b);

    if(m <= 0.0031308)
        return m * 12.92;
    
    return pow(m, 1 / 2.4) * 1.055 - 0.055;
}

/* Renvoie un pointeur sur h pointeurs sur w entiers correspondant 
aux différents pixels de l’image dans l’ordre transformés en gris */
int** grayscale(int** pixels, int w, int h) {
    int** gray_pixels = (int**) malloc(sizeof(int*) * h);

    for(int i = 0; i < h; i++) {
        gray_pixels[i] = (int*) malloc(sizeof(int) * w);

        for(int j = 0; j < w; j++) {
            int r = pixels[i][j * 3];
            int g = pixels[i][j * 3 + 1];
            int b = pixels[i][j * 3 + 2];

            /* De base arrondi à l'inférieur */
            gray_pixels[i][j] = (int) (255 * lum(r, g, b));
        }
    }

    return gray_pixels;
}

/* Renvoie la moyenne du niveau de gris de tous les pixels situés dans le rectangle 
entre les colonnes c et c + wr - 1 et entre les lignes l et l + hr - 1 */
float get_intensity(int** graypixels, int c, int l, int wr, int hr) {
    int c_end = c + wr - 1;
    int l_end = l + hr - 1;
    int sum = 0;
    int nb_elem = 1;

    for(int i = l; i <= l_end; i++) {
        for(int j = c; j <= c_end; j++) {
            sum += graypixels[i][j];
            nb_elem++;
        }
    }

    return (float) sum / (float) nb_elem;
}

/* Cette fonction découpe l’image en waa × haa rectangles
Elle renvoie un pointeur sur haa pointeurs sur waa flottants correspondant 
aux intensités de chaque rectangle */
float** get_intensities(int** graypixels, int w, int h, int waa, int haa) {
    float** intensities = (float**) malloc(sizeof(float*) * haa);
    int rect_w = w / waa;
    int rect_h = h / haa;

    for(int i = 0; i < haa; i++) {
        intensities[i] = (float*) malloc(sizeof(float) * waa);

        for(int j = 0; j < waa; j++) {
            int c = j * rect_w;
            int l = i * rect_h;

            intensities[i][j] = get_intensity(graypixels, c, l, rect_w, rect_h);
        }
    }

    return intensities;
}

/* Renvoie une intensité de gris convertie en charactère approprié issu de chartable */
char intensity_to_char(float intensity, char* chartable) {
    int charlen = strlen(chartable);
    float val = (1 - (intensity / 255)) * (float) (charlen - 1);

    return chartable[(int) round(val)];
}

/* Renvoie des listes de caratères des intensités converties */
char** intensities_to_chars(float** intensities, char* chartableW, int waa, int haa) {
    char** chars = (char**) malloc(sizeof(char*) * haa);

    for(int i = 0; i < haa; i++) {
        chars[i] = (char*) malloc(sizeof(char) * waa);

        for(int j = 0; j < waa; j++) {
            chars[i][j] = intensity_to_char(intensities[i][j], chartableW);
        }
    }

    return chars;
}

int main(void) {
    int w, h;
    int haa = 500, waa = 500;
    char* chartable = " .:-=+*#%@";
    int** ppm = read_ppm(&w, &h);
    /*int** gray_ppm = grayscale(ppm, w, h);
    float** intensities = get_intensities(gray_ppm, w, h, waa, haa);
    char** chars = intensities_to_chars(intensities, chartable, waa, haa);

    for(int i = 0; i < haa; i++) {
        for(int j = 0; j < waa; j++) {
            printf("%c", chars[i][j]);
        }

        printf("\n");
    }
    printf("\n");

    free(ppm);
    free(gray_ppm);
    free(intensities);
    free(chars);*/

    return EXIT_SUCCESS;
}
