#include <stdio.h>
#include <string.h>
#include "parse.h" 
#include <stdlib.h>

int read_chord(char* in, Chord* new_chord){
    int i;
    Note temp;
    int arr_ptr;
    char note_str[NOTE_SIZE];

    in += 2;
    
    for (arr_ptr = 0; arr_ptr < CHORD_SIZE; arr_ptr++){
        //  lé ate ao proximo espaço ou newline (excluindo)
        sscanf(in, "%[^ \n]", note_str);

        // verifica se oq foi lido é um input valido
        temp = get_note_enum(note_str);

        if (temp == Inv){
            return 1;
        }

        // adiciona ao novo accorde e avança o buffer ate encontrar um character não espaço
        new_chord->note_array[arr_ptr] = temp;
        new_chord->size++;
        
        in += strlen(note_str);
        
        while (in[0] == ' '){
            in++;
        }
        // encontrar o fim da string
        if (in[0] == '\0' || in[0] == '\n'){
            break;   
        }
        // preparar a string para a proxima nota
        for (i = 0; i < NOTE_SIZE; i++){
            note_str[i] = '\0';
        }
    }
    return 0;
}

int write_array_to_file(char* name, FILE* fptr, Chord** arr, int size){
    int i, j;
    fptr = fopen(name, "w+");
    if (fptr == NULL){
        return 1;
    }

    for (i = 0; i < size; i++){
        fprintf(fptr,"a ");
        for (j = 0; j < arr[i]->size;j++){
            fprintf(fptr,"%s ",get_note_string(arr[i]->note_array[j]));
        }
        fprintf(fptr,"\n");
    }

    if (fclose(fptr) != 0){
        return 1;
    }
    return 0;
}

int read_array_from_file(char* name, FILE* fptr, Chord** arr){
    char buf[BUFSIZ], *res;
    int ptr = 0;

    fptr = fopen(name, "r");

    if (NULL == fptr) {
        return -1;
    }

    res = fgets(buf, BUFSIZ ,fptr);
    
    while (res != NULL && ptr < MAX_CHORDS){
        arr[ptr] = (Chord*)calloc(1,sizeof(Chord));
        read_chord(buf, arr[ptr++]);
        res = fgets(buf, BUFSIZ ,fptr);
    }
    

    if (fclose(fptr) != 0){
        return -1;
    }
    return ptr;
}