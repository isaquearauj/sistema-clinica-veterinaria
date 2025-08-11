#include <stdio.h>
#include <stdlib.h>

#define GROWTH_STEP 5

// ------------ Structs ------------
typedef struct
{
    int codigoAnimal;
    char nome[50];
    char especie[30];
    char dataNascimento[11]; // "DD/MM/AAAA"
} Animal;

typedef struct
{
    int crmVet;
    char nome[50];
    char telefone[16];
} Veterinario;

typedef struct
{
    int codigoConsulta;
    int codigoAnimal;
    int crmVet;
    char dataConsulta[11]; // "DD/MM/AAAA"
} Consulta;

// ------------ Vetores dinâmicos (ponteiros) ------------
int main(void)
{
    // Ponteiros para os vetores
    Animal *animais = NULL;
    Veterinario *vets = NULL;
    Consulta *cons = NULL;

    // Quantidades e capacidades
    int qtdAnimais = 0, capAnimais = 0;
    int qtdVets = 0, capVets = 0;
    int qtdCons = 0, capCons = 0;

    // Alocação inicial mínima (5 cada, por exemplo)
    capAnimais = GROWTH_STEP;
    capVets = GROWTH_STEP;
    capCons = GROWTH_STEP;

    animais = (Animal *)malloc(capAnimais * sizeof(Animal));
    vets = (Veterinario *)malloc(capVets * sizeof(Veterinario));
    cons = (Consulta *)malloc(capCons * sizeof(Consulta));

    if (!animais || !vets || !cons)
    {
        printf("Erro de memória na alocação inicial.\n");
        free(animais);
        free(vets);
        free(cons);
        return 1;
    }

    // ----------------- Demonstração simples de crescimento -----------------
    // Obs.: Aqui não inserimos registros de verdade; só simulamos o aumento
    // da quantidade para mostrar o uso do realloc quando lotar.

    // Simula "preencher" até a capacidade atual de animais
    qtdAnimais = capAnimais;

    // Tentativa de "adicionar mais um" → precisa crescer
    if (qtdAnimais == capAnimais)
    {
        int novaCap = capAnimais + GROWTH_STEP;
        Animal *tmp = (Animal *)realloc(animais, novaCap * sizeof(Animal));
        if (!tmp)
        {
            printf("Erro de memória ao crescer vetor de animais.\n");
            free(animais);
            free(vets);
            free(cons);
            return 1;
        }
        animais = tmp;
        capAnimais = novaCap;
        // Agora teríamos espaço para mais elementos
    }

    // Idem para veterinários (só demonstrando o padrão)
    qtdVets = capVets;
    if (qtdVets == capVets)
    {
        int novaCap = capVets + GROWTH_STEP;
        Veterinario *tmp = (Veterinario *)realloc(vets, novaCap * sizeof(Veterinario));
        if (!tmp)
        {
            printf("Erro de memória ao crescer vetor de veterinários.\n");
            free(animais);
            free(vets);
            free(cons);
            return 1;
        }
        vets = tmp;
        capVets = novaCap;
    }

    // Idem para consultas
    qtdCons = capCons;
    if (qtdCons == capCons)
    {
        int novaCap = capCons + GROWTH_STEP;
        Consulta *tmp = (Consulta *)realloc(cons, novaCap * sizeof(Consulta));
        if (!tmp)
        {
            printf("Erro de memória ao crescer vetor de consultas.\n");
            free(animais);
            free(vets);
            free(cons);
            return 1;
        }
        cons = tmp;
        capCons = novaCap;
    }
    // ----------------- Fim da demonstração -----------------

    // Aqui, mais à frente, chamaremos os menus.
    // Por enquanto, só stubs vazios:
    // menuPrincipal();

    // Liberações
    free(animais);
    free(vets);
    free(cons);
    return 0;
}

// ------------ Menus (stubs vazios pra preencher depois) ------------
void menuPrincipal(void)
{
    // TODO: implementar menu principal (Animais, Veterinários, Consultas, Relatórios, Sair)
}

void menuAnimais(void)
{
    // TODO: implementar submenu de Animais (Cadastrar, Consultar, Alterar, Remover, Listar)
}
void menuVeterinarios(void)
{
    printf("======= Menu Veterinarios =======\n");
    printf("(1) Cadastrar\n");
    printf("(2) Consultar por CRMV\n");
    printf("(3) Alterar\n");
    printf("(4) Remover\n");
    printf("(5) Listar Todos\n");
    printf("(6) Voltar\n");
    printf("Escolha: ");
    int escolha;
    scanf("%d", &escolha);

    switch (escolha)
    {

    case 1:
        // TODO func_cadastrovet;
        break;
    case 2:
        // TODO func_contultaCRMV;
        break;
    case 3:
        // TODO func_alterar;
        break;
    case 4:
        // TODO func_removerVet;
        break;
    case 5:
        // TODO func_listarVet;
        break;
    case 6:
        menuPrincipal();
        break;
    }
}
void menuConsultas(void)
{
    // TODO: implementar submenu de Consultas (Cadastrar, Consultar, Alterar, Remover, Listar/Filtrar)
}
void menuRelatorios(void)
{
    //
}
