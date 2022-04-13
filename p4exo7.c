#include "p4exo7.h"

#define BUFFLEN 1024

int ecrire_block(char* nom, Block* b){
    FILE* f;
    f=fopen(nom,"w");
    if (!f){
		printf("Erreur d'ouverture de %s\n", nom);
		return 0;
	}
    CellProtected *vote;
    char*pr_str;
    if (b){
        fprintf(f, "(%lx,%lx)\n", b->author->val, b->author->n);
        fprintf(f, "%d\n",b->nb_votes);
        vote = b->votes;
        while(vote){
            pr_str=protected_to_str(vote->data);
            if (!pr_str){
                fclose(f);
                return 0;
            }
            fprintf(f, "%s\n", pr_str);

            free(pr_str);
            vote = vote->next;
        }
        fprintf(f, "%s\n",b->hash);
        fprintf(f, "%s\n",b->previous_hash);
        fprintf(f, "%d\n",b->nonce);
    }
    fclose(f);
    return 1;
}

Block * create_Block(Key *author, CellProtected *votes, unsigned char *hash, unsigned char *previous_hash, int nonce, int nb_votes){
    Block* b =(Block*)malloc(sizeof(Block));
    if(!b){
        return NULL;
    }
    b->author=author;
    b->votes=votes;
    b->hash=hash;
    b->previous_hash=previous_hash;
    b->nonce=nonce;
    b->nb_votes = nb_votes;
    return b;
}

Block* lire_block(char* nom){
    FILE* f = fopen(nom,"r");
    if (f==NULL){
		printf("Erreur d'ouverture de %s\n", nom);
		return NULL;
	}
    Key* cle=(Key*)malloc(sizeof(Key));
    if(!cle){
        fclose(f);
        return NULL;
    }
    char buffer[BUFFLEN];
    long val,n;
    Protected* pr;
    CellProtected* c;
	CellProtected* lcp = NULL;
    unsigned char* hash;
    unsigned char* hash_precedent;
    int nonce;
    int nb_votes;
    if (fgets(buffer, BUFFLEN, f) == NULL){
        free(cle);
        fclose(f);
        return NULL;
    }
    if (sscanf(buffer,"(%lx,%lx)",&val, &n) != 2){
        free(cle);
        fclose(f);
        return NULL;   
    }
    
    init_key(cle, val, n);
    
    //lit nb_votes
    if (fgets(buffer, BUFFLEN, f) == NULL){
        free(cle);
        fclose(f);
        return NULL;
    }
    if (sscanf(buffer,"%d",&nb_votes) != 1){
        free(cle);
        fclose(f);
        return NULL;   
    }

    //lit les déclarations de vote
    for (int i = 0; i < nb_votes; i++){
        if (fgets(buffer, BUFFLEN, f) == NULL){
            delete_list_cell(lcp);
            free(cle);
            fclose(f);
            return NULL;   
        }
        pr = str_to_protected(buffer);
        if (!pr){
            delete_list_cell(lcp);
            free(cle);
            fclose(f);
            return NULL;
        }
        c = create_cell_protected(pr);
        if (!c){
            free(pr);
            delete_list_cell(lcp);
            free(cle);
            fclose(f);
            return NULL;
            }
        add_cellProtected_to_front(&lcp, c);
    }

    //lit le hash du bloc
    if (fgets(buffer, BUFFLEN, f) == NULL){
        delete_list_cell(lcp);
        free(cle);
        fclose(f);
        return NULL;
    }
    hash = (unsigned char*)malloc(sizeof(unsigned char) * strlen(buffer)); //alloue une chaine d'unsigned char de la bonne taille
    if (!hash){
        delete_list_cell(lcp);
        free(cle);
        fclose(f);
        return NULL;
    }
    if (sscanf(buffer,"%s",hash) != 1){
        delete_list_cell(lcp);
        free(cle);
        free(hash);
        fclose(f);
        return NULL;
    }

    //lit le hash du bloc precedent
    if (fgets(buffer, BUFFLEN, f) == NULL){
        delete_list_cell(lcp);
        free(cle);
        free(hash);
        fclose(f);
        return NULL;
    }
    hash_precedent = (unsigned char*)malloc(sizeof(unsigned char) * strlen(buffer)); //alloue une chaine d'unsigned char de la bonne taille
    if (!hash_precedent){
        delete_list_cell(lcp);
        free(cle);
        free(hash);
        fclose(f);
        return NULL;
    }
    if (sscanf(buffer,"%s", hash_precedent) != 1){
        delete_list_cell(lcp);
        free(cle);
        free(hash);
        free(hash_precedent);
        fclose(f);
        return NULL;
    }

    //lit nonce
    if (fgets(buffer, BUFFLEN, f) == NULL){
        delete_list_cell(lcp);
        free(cle);
        free(hash);
        free(hash_precedent);
        fclose(f);
        return NULL;
    }
    if (sscanf(buffer,"%d", &nonce) != 1){
        delete_list_cell(lcp);
        free(cle);
        free(hash);
        free(hash_precedent);
        fclose(f);
        return NULL;
    }


    Block *b = create_Block(cle,lcp,hash,hash_precedent,nonce, nb_votes);
    if(!b){       
        delete_list_cell(lcp);
        free(cle);
        free(hash);
        free(hash_precedent);
        fclose(f);
    }

    fclose(f);
    return b;
}

// char* block_to_str(Block* block){
//     char* key = key_to_str(block->author);
//     if (!key)
//         return NULL;
//     char* listevotes = protected_to_str(block->votes->data);
//     if (!listevotes)
//         free(key);
//         return NULL;
//     int cpt=0;
//     while (block->votes){
//         cpt++;
//         block=block->votes->next;
//     }
//     int size = strlen(key)+strlen(listevotes)*cpt+strlen(block->previous_hash)+strlen(block->nonce)+3+cpt;
//     char* caractere = (char*)malloc(size*sizeof(char));
//     if(!caractere){
//         free(key);
//         free(listevotes);
//         return NULL;
//     }
//     int i=0;
//     int j=0;
//     while(key[j]){
//         caractere[i]=key[j];
//         i++;
//         j++;
//     }
//     caractere[i] = ' ';
//     i++;
//     j=0;
//     while(listevotes[j]){
//         caractere[i]=listevotes[j];
//         i++;
//         j++;
//     }
//     caractere[i] = ' ';
//     i++;
//     j=0;
//     while(block->previous_hash[j]){//pas sur de pouvoir copie ca 
//         caractere[i]=block->previous_hash[j];
//         i++;
//         j++;
//     }
//     caractere[i] = ' ';
//     i++;
//     caractere[i]=block->nonce;
//     i++;
//     caractere[i]='\0';
//     free(key);
//     free(listevotes);
//     return caractere;
// }

//question 7.5
// unsigned char* crypteensha256 (char* message){
//     unsigned char *d = SHA256(message,strlen(message),0);
//     int i;
//     for(i=0;i<SHA256_DIGEST_LENGTH;i++){
//         printf("%02x",d[i]);
//     }
//     putchar('\n');
//     return d;
// }

//question 7.9
void delete_block(Block *b){
    if (b != NULL){
        CellProtected* tmp;
        CellProtected* vote;
        vote = b->votes;
        while(vote){
            tmp = vote;
            vote = vote->next;
            free(tmp);
        }

        free(b->hash);
        free(b->previous_hash);
        free(b);
    }
}



int main(){
    Key *k = malloc(sizeof(Key));
    if (!k)
        return 1;
    k->n = 123;
    k->val = 456;


    CellProtected *lcp = read_protected("declarations.txt");
	if (!lcp){
		free(k);
		return 1;
	}
    unsigned char *h = malloc(sizeof(unsigned char) * 3);
    h[0] = 'h'; 
    h[1] = '1'; 
    h[2] = '\0'; 
    unsigned char *prev_h = malloc(sizeof(unsigned char) * 3);
    prev_h[0] = 'h'; 
    prev_h[1] = '0'; 
    prev_h[2] = '\0'; 


    CellProtected* tmp = lcp;
	int nb_votes = 0;
	while(tmp){ //calcule la taile de la liste
		nb_votes++;
		tmp = tmp->next;
	}

    Block *b = create_Block(k, lcp, h, prev_h, 111, nb_votes);

    ecrire_block("blocks.txt", b);

    free(b->author);
    delete_list_cell(b->votes);
    b->votes = NULL;
    delete_block(b);


    Block *b2 = lire_block("blocks.txt");

    ecrire_block("blocks.txt", b2);

    free(b2->author);
    delete_list_cell(b2->votes);
    b2->votes = NULL;
    delete_block(b2);
}

