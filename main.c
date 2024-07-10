#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

typedef struct Livro {
    char titulo[100];
    char autor[100];
    char isbn[20];
    int ano;
    int disponivel; // 1 se disponível, 0 se emprestado
    struct Livro *proximo;
} Livro;

typedef struct Lista {
    Livro *cabeca;
} Lista;

typedef struct Emprestimo {
    char isbn[20];
    char usuario[100];
    struct Emprestimo *proximo;
} Emprestimo;

typedef struct Fila {
    Emprestimo *frente;
    Emprestimo *tras;
} Fila;

typedef struct Pilha {
    Emprestimo *topo;
} Pilha;

Livro* buscarLivroPorISBN(Lista *lista, char *isbn);

// Função para criar um novo livro
Livro* criarLivro(char *titulo, char *autor, char *isbn, int ano) {
    Livro *novoLivro = (Livro *)malloc(sizeof(Livro));
    strcpy(novoLivro->titulo, titulo);
    strcpy(novoLivro->autor, autor);
    strcpy(novoLivro->isbn, isbn);
    novoLivro->ano = ano;
    novoLivro->disponivel = 1;
    novoLivro->proximo = NULL;
    return novoLivro;
}

// Função para adicionar um livro à lista ligada
void adicionarLivro(Lista *lista, Livro *livro) {
    // Verifica se já existe um livro com o mesmo ISBN
    Livro *existente = buscarLivroPorISBN(lista, livro->isbn);
    if (existente != NULL) {
        printf("Erro: Já existe um livro com o mesmo ISBN.\n");
        free(livro); // Libera a memória alocada para o novo livro
        return;
    }
    livro->proximo = lista->cabeca;
    lista->cabeca = livro;
}

// Função para salvar livros em um arquivo de texto
void salvarLivros(Lista *lista, const char *nomeArquivo) {
    FILE *arquivo = fopen(nomeArquivo, "w");
    if (!arquivo) {
        perror("Não foi possível abrir o arquivo para escrita");
        return;
    }
    Livro *atual = lista->cabeca;
    while (atual) {
        fprintf(arquivo, "%s,%s,%s,%d,%d\n", atual->titulo, atual->autor, atual->isbn, atual->ano, atual->disponivel);
        atual = atual->proximo;
    }
    fclose(arquivo);
}

// Função para carregar livros de um arquivo de texto
void carregarLivros(Lista *lista, const char *nomeArquivo) {
    FILE *arquivo = fopen(nomeArquivo, "r");
    if (!arquivo) {
        perror("Não foi possível abrir o arquivo para leitura");
        return;
    }
    char linha[256];
    while (fgets(linha, sizeof(linha), arquivo)) {
        char titulo[100], autor[100], isbn[20];
        int ano, disponivel;
        sscanf(linha, "%[^,],%[^,],%[^,],%d,%d", titulo, autor, isbn, &ano, &disponivel);
        Livro *livro = criarLivro(titulo, autor, isbn, ano);
        livro->disponivel = disponivel;
        adicionarLivro(lista, livro);
    }
    fclose(arquivo);
}

// Função para exibir os livros na lista
void exibirLivros(Lista *lista) {
    Livro *atual = lista->cabeca;
    while (atual) {
        printf("Título: %s\nAutor: %s\nISBN: %s\nAno: %d\nDisponível: %s\n\n", 
            atual->titulo, atual->autor, atual->isbn, atual->ano, 
            atual->disponivel ? "Sim" : "Não");
        atual = atual->proximo;
    }
}

// Função para buscar um livro por título
Livro* buscarLivroPorTitulo(Lista *lista, const char *titulo) {
    Livro *atual = lista->cabeca;
    while (atual) {
        if (strcmp(atual->titulo, titulo) == 0) {
            return atual;
        }
        atual = atual->proximo;
    }
    return NULL;
}

// Função para buscar um livro pelo ISBN
Livro* buscarLivroPorISBN(Lista *lista, char *isbn) {
    Livro *atual = lista->cabeca;
    while (atual) {
        if (strcmp(atual->isbn, isbn) == 0) {
            return atual; // Retorna o livro encontrado
        }
        atual = atual->proximo;
    }
    return NULL; // Retorna NULL se o livro não for encontrado
}

// Função para criar um novo empréstimo
Emprestimo* criarEmprestimo(char *isbn, char *usuario) {
    Emprestimo *novoEmprestimo = (Emprestimo *)malloc(sizeof(Emprestimo));
    strcpy(novoEmprestimo->isbn, isbn);
    strcpy(novoEmprestimo->usuario, usuario);
    novoEmprestimo->proximo = NULL;
    return novoEmprestimo;
}

// Função para registrar um empréstimo
void registrarEmprestimo(Lista *lista, Fila *fila, Pilha *pilha, char *isbn, char *usuario) {
    Livro *livro = buscarLivroPorISBN(lista, isbn);
    if (livro && livro->disponivel) {
        livro->disponivel = 0;
        Emprestimo *emprestimo = criarEmprestimo(isbn, usuario);
        emprestimo->proximo = pilha->topo;
        pilha->topo = emprestimo;
        printf("Empréstimo registrado com sucesso!\n");
    } else if (livro && !livro->disponivel) {
        Emprestimo *espera = criarEmprestimo(isbn, usuario);
        if (fila->tras) {
            fila->tras->proximo = espera;
            fila->tras = espera;
        } else {
            fila->frente = fila->tras = espera;
        }
        printf("Livro indisponível. Colocado na fila de espera.\n");
    } else {
        printf("Livro não encontrado.\n");
    }
}

// Função para devolver um livro
void devolverLivro(Lista *lista, Pilha *pilha, char *isbn) {
    Livro *livro = buscarLivroPorISBN(lista, isbn);
    if (livro && !livro->disponivel) {
        livro->disponivel = 1;
        if (pilha->topo && strcmp(pilha->topo->isbn, isbn) == 0) {
            Emprestimo *topo = pilha->topo;
            pilha->topo = pilha->topo->proximo;
            free(topo);
            printf("Livro devolvido com sucesso!\n");
        }
    } else {
        printf("Livro não encontrado ou já devolvido.\n");
    }
}

// Função para salvar empréstimos em um arquivo binário
void salvarEmprestimos(Pilha *pilha, const char *nomeArquivo) {
    FILE *arquivo = fopen(nomeArquivo, "wb");
    if (!arquivo) {
        perror("Não foi possível abrir o arquivo para escrita");
        return;
    }
    Emprestimo *atual = pilha->topo;
    while (atual) {
        fwrite(atual, sizeof(Emprestimo), 1, arquivo);
        atual = atual->proximo;
    }
    fclose(arquivo);
}

// Função para carregar empréstimos de um arquivo binário
void carregarEmprestimos(Pilha *pilha, const char *nomeArquivo) {
    FILE *arquivo = fopen(nomeArquivo, "rb");
    if (!arquivo) {
        perror("Não foi possível abrir o arquivo para leitura");
        return;
    }
    Emprestimo emp;
    while (fread(&emp, sizeof(Emprestimo), 1, arquivo)) {
        Emprestimo *novoEmprestimo = criarEmprestimo(emp.isbn, emp.usuario);
        novoEmprestimo->proximo = pilha->topo;
        pilha->topo = novoEmprestimo;
    }
    fclose(arquivo);
}

// Pesquisa sequencial por ISBN
Livro* pesquisaSequencial(Lista *lista, const char *isbn) {
    Livro *atual = lista->cabeca;
    while (atual) {
        if (strcmp(atual->isbn, isbn) == 0) {
            return atual;
        }
        atual = atual->proximo;
    }
    return NULL;
}

// Pesquisa binária por título (assume lista ordenada)
Livro* pesquisaBinaria(Livro *livros[], int inicio, int fim, const char *titulo) {
    if (inicio <= fim) {
        int meio = inicio + (fim - inicio) / 2;
        int cmp = strcmp(livros[meio]->titulo, titulo);
        if (cmp == 0) {
            return livros[meio];
        } else if (cmp > 0) {
            return pesquisaBinaria(livros, inicio, meio - 1, titulo);
        } else {
            return pesquisaBinaria(livros, meio + 1, fim, titulo);
        }
    }
    return NULL;
}

// Função para converter a lista ligada em um array
int listaParaArray(Lista *lista, Livro *array[]) {
    int n = 0;
    Livro *atual = lista->cabeca;
    while (atual) {
        array[n++] = atual;
        atual = atual->proximo;
    }
    return n;
}

void troca(Livro **a, Livro **b) {
    Livro *temp = *a;
    *a = *b;
    *b = temp;
}

void bubbleSort(Livro *array[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (strcmp(array[j]->titulo, array[j + 1]->titulo) > 0) {
                troca(&array[j], &array[j + 1]);
            }
        }
    }
}

// Função para exibir o menu e obter a escolha do usuário
int menu() {
    int escolha;
    printf("\nMenu:\n");
    printf("1. Adicionar Livro\n");
    printf("2. Registrar Empréstimo\n");
    printf("3. Devolver Livro\n");
    printf("4. Exibir Livros\n");
    printf("5. Buscar Livro por Título\n");
    printf("6. Ordenar Livros por Título\n");
    printf("7. Sair\n");
    printf("Escolha uma opção: ");
    scanf("%d", &escolha);
    getchar(); // Captura o caractere de nova linha após o scanf
    return escolha;
}

// Função principal (main)
int main() {
    
    setlocale(LC_ALL, "Portuguese");
    Lista lista;
    lista.cabeca = NULL;

    Fila fila;
    fila.frente = fila.tras = NULL;

    Pilha pilha;
    pilha.topo = NULL;

    // Carregar livros do arquivo
    carregarLivros(&lista, "livros.txt");
    carregarEmprestimos(&pilha, "emprestimos.bin");

    int escolha;
    do {
        escolha = menu();
        switch (escolha) {
            case 1: {
                char titulo[100], autor[100], isbn[20];
                int ano;
                printf("Digite o título do livro: ");
                scanf(" %[^\n]", titulo);
                printf("Digite o autor do livro: ");
                scanf(" %[^\n]", autor);
                printf("Digite o ISBN do livro: ");
                scanf("%s", isbn);
                printf("Digite o ano de publicação: ");
                scanf("%d", &ano);
                Livro *livro = criarLivro(titulo, autor, isbn, ano);
                adicionarLivro(&lista, livro);
                salvarLivros(&lista, "livros.txt");
                break;
            }
            case 2: {
                char isbn[20], usuario[100];
                printf("Digite o ISBN do livro: ");
                scanf("%s", isbn);
                printf("Digite o nome do usuário: ");
                scanf(" %[^\n]", usuario);
                registrarEmprestimo(&lista, &fila, &pilha, isbn, usuario);
                salvarLivros(&lista, "livros.txt");
                salvarEmprestimos(&pilha, "emprestimos.dat");
                break;
            }
            case 3: {
                char isbn[20];
                printf("Digite o ISBN do livro: ");
                scanf("%s", isbn);
                devolverLivro(&lista, &pilha, isbn);
                salvarLivros(&lista, "livros.txt");
                salvarEmprestimos(&pilha, "emprestimos.dat");
                break;
            }
            case 4: {
                exibirLivros(&lista);
                break;
            }
            case 5: {
                char titulo[100];
                printf("Digite o título do livro: ");
                scanf(" %[^\n]", titulo);
                Livro *buscado = buscarLivroPorTitulo(&lista, titulo);
                if (buscado) {
                    printf("Livro encontrado: %s por %s\n", buscado->titulo, buscado->autor);
                } else {
                    printf("Livro não encontrado.\n");
                }
                break;
            }
            case 6: {
                Livro *livros[100];
                int n = listaParaArray(&lista, livros);
                bubbleSort(livros, n);
                printf("Livros ordenados por título:\n");
                for (int i = 0; i < n; i++) {
                    printf("Título: %s\n", livros[i]->titulo);
                }
                break;
            }
            case 7: {
                printf("Saindo...\n");
                break;
            }
            default: {
                printf("Opção inválida!\n");
                break;
            }
        }
    } while (escolha != 7);

    return 0;
}
