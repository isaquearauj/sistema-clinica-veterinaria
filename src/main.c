#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOME_TAM       50
#define ESPECIE_TAM    30
#define DATA_TAM       11    // "DD/MM/AAAA" + '\0'
#define TELEFONE_TAM   16
#define GROWTH_STEP     5

// Structs principais
typedef struct Animal {
    int  idAnimal;                          // sequencial (1..N)
    char nome[NOME_TAM];
    char especie[ESPECIE_TAM];
    char dataNascimento[DATA_TAM];
} Animal;

typedef struct {
    int  crmVet;                            // identificador único (real)
    char nome[NOME_TAM];
    char telefone[TELEFONE_TAM];
} Veterinario;

typedef struct {
    int  idConsulta;                        // (para próxima entrega)
    int  idAnimal;                          // FK -> Animal.idAnimal
    int  crmVet;                            // FK -> Veterinario.crmVet
    char dataConsulta[DATA_TAM];
} Consulta;

// Estado do programa (2ª entrega)
static Animal      *g_animais      = NULL;
static int          g_nAnimais     = 0;
static int          g_capAnimais   = 0;

static Veterinario *g_vets         = NULL;
static int          g_nVets        = 0;
static int          g_capVets      = 0;

// Reservado para a próxima entrega (consultas)
static Consulta    *g_consultas    = NULL;
static int          g_nCons        = 0;
static int          g_capCons      = 0;

// Utilidade
static void limpar_buffer_entrada(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static int garantir_capacidade_animais(int extra) {
    if (g_nAnimais + extra <= g_capAnimais) return 1;
    int novo = g_capAnimais + GROWTH_STEP;
    Animal *p = (Animal*)realloc(g_animais, novo * sizeof(Animal));
    if (!p) return 0;
    g_animais = p; g_capAnimais = novo; return 1;
}

static int garantir_capacidade_veterinarios(int extra) {
    if (g_nVets + extra <= g_capVets) return 1;
    int novo = g_capVets + GROWTH_STEP;
    Veterinario *p = (Veterinario*)realloc(g_vets, novo * sizeof(Veterinario));
    if (!p) return 0;
    g_vets = p; g_capVets = novo; return 1;
}

// IDs de Animal devem permanecer 1..N, sem buracos após remoções
static void resequenciar_animais(void) {
    for (int i = 0; i < g_nAnimais; i++) {
        g_animais[i].idAnimal = i + 1;
    }
    /* IMPORTANTE (próxima entrega):
       Se existirem Consultas, será necessário atualizar
       as FKs (consultas[i].idAnimal) após essa resequenciação. */
}

// Buscas
static int encontrar_indice_animal_por_id(int id) {
    for (int i = 0; i < g_nAnimais; i++)
        if (g_animais[i].idAnimal == id) return i;
    return -1;
}

static int encontrar_indice_veterinario_por_crm(int crm) {
    for (int i = 0; i < g_nVets; i++)
        if (g_vets[i].crmVet == crm) return i;
    return -1;
}

// CRUD Animal
static void cadastrar_animal(void) {
    if (!garantir_capacidade_animais(1)) { printf("Erro de memoria.\n"); return; }

    Animal a;
    a.idAnimal = g_nAnimais + 1;  // contínuo (reajustado em remoções)
    printf("\n[Cadastrar Animal]\n");
    printf("Nome: ");         limpar_buffer_entrada(); scanf(" %49[^\n]", a.nome);
    printf("Especie: ");      scanf(" %29[^\n]", a.especie);
    printf("Data de Nascimento (DD/MM/AAAA): "); scanf(" %10s", a.dataNascimento);

    g_animais[g_nAnimais++] = a;
    printf("Animal cadastrado com id %d.\n", a.idAnimal);
}

static void atualizar_animal(void) {
    int id;
    printf("\n[Atualizar Animal]\n");
    printf("Informe o id do animal: ");
    if (scanf("%d", &id) != 1) { limpar_buffer_entrada(); printf("Entrada invalida.\n"); return; }

    int idx = encontrar_indice_animal_por_id(id);
    if (idx < 0) { printf("Animal nao encontrado.\n"); return; }

    int opc;
    do {
        printf("\n--- Escolha o campo para atualizar ---\n");
        printf("(1) Nome\n");
        printf("(2) Especie\n");
        printf("(3) Data de Nascimento\n");
        printf("(0) Concluir\n");
        printf("Opcao: ");
        if (scanf("%d", &opc) != 1) { limpar_buffer_entrada(); opc = -1; }

        switch (opc) {
            case 1: limpar_buffer_entrada(); printf("Novo nome: "); scanf(" %49[^\n]", g_animais[idx].nome); break;
            case 2: limpar_buffer_entrada(); printf("Nova especie: "); scanf(" %29[^\n]", g_animais[idx].especie); break;
            case 3: printf("Nova data (DD/MM/AAAA): "); scanf(" %10s", g_animais[idx].dataNascimento); break;
            case 0: printf("Atualizacao concluida.\n"); break;
            default: printf("Opcao invalida.\n");
        }
    } while (opc != 0);
}

static void remover_animal(void) {
    int id;
    printf("\n[Remover Animal]\n");
    printf("Informe o id do animal: ");
    if (scanf("%d", &id) != 1) { limpar_buffer_entrada(); printf("Entrada invalida.\n"); return; }

    int idx = encontrar_indice_animal_por_id(id);
    if (idx < 0) { printf("Animal nao encontrado.\n"); return; }

    // OBS: Na próxima entrega, bloquear se houver consultas vinculadas ao animal

    for (int i = idx; i < g_nAnimais - 1; i++) {
        g_animais[i] = g_animais[i + 1];
    }
    g_nAnimais--;
    resequenciar_animais();
    printf("Animal removido e IDs resequenciados.\n");
}

static void consultar_animal_por_id(void) {
    int id;
    printf("\n[Consultar Animal por ID]\n");
    printf("Informe o id: ");
    if (scanf("%d", &id) != 1) { limpar_buffer_entrada(); printf("Entrada invalida.\n"); return; }

    int idx = encontrar_indice_animal_por_id(id);
    if (idx < 0) { printf("Animal nao encontrado.\n"); return; }

    Animal *a = &g_animais[idx];
    printf("\n#%d | Nome: %s | Especie: %s | Nasc: %s\n",
           a->idAnimal, a->nome, a->especie, a->dataNascimento);
}

static void listar_animais(void) {
    printf("\n[Listar Animais]\n");
    if (g_nAnimais == 0) { printf("Nenhum animal cadastrado.\n"); return; }
    for (int i = 0; i < g_nAnimais; i++) {
        printf("#%d | Nome: %s | Especie: %s | Nasc: %s\n",
               g_animais[i].idAnimal, g_animais[i].nome,
               g_animais[i].especie, g_animais[i].dataNascimento);
    }
}

// CRUD Veterinário
static void cadastrar_veterinario(void) {
    if (!garantir_capacidade_veterinarios(1)) { printf("Erro de memoria.\n"); return; }

    Veterinario v;
    printf("\n[Cadastrar Veterinario]\n");
    printf("CRM: "); if (scanf("%d", &v.crmVet) != 1) { limpar_buffer_entrada(); printf("Entrada invalida.\n"); return; }
    if (encontrar_indice_veterinario_por_crm(v.crmVet) != -1) { printf("Ja existe veterinario com esse CRM.\n"); return; }

    printf("Nome: "); limpar_buffer_entrada(); scanf(" %49[^\n]", v.nome);
    printf("Telefone: "); scanf(" %15s", v.telefone);

    g_vets[g_nVets++] = v;
    printf("Veterinario cadastrado (CRM %d).\n", v.crmVet);
}

static void atualizar_veterinario(void) {
    int crm;
    printf("\n[Atualizar Veterinario]\n");
    printf("Informe o CRM: ");
    if (scanf("%d", &crm) != 1) { limpar_buffer_entrada(); printf("Entrada invalida.\n"); return; }

    int idx = encontrar_indice_veterinario_por_crm(crm);
    if (idx < 0) { printf("Veterinario nao encontrado.\n"); return; }

    int opc;
    do {
        printf("\n--- Escolha o campo para atualizar ---\n");
        printf("(1) Nome\n");
        printf("(2) Telefone\n");
        printf("(0) Concluir\n");
        printf("Opcao: ");
        if (scanf("%d", &opc) != 1) { limpar_buffer_entrada(); opc = -1; }

        switch (opc) {
            case 1: limpar_buffer_entrada(); printf("Novo nome: "); scanf(" %49[^\n]", g_vets[idx].nome); break;
            case 2: printf("Novo telefone: "); scanf(" %15s", g_vets[idx].telefone); break;
            case 0: printf("Atualizacao concluida.\n"); break;
            default: printf("Opcao invalida.\n");
        }
    } while (opc != 0);
}

static void remover_veterinario(void) {
    int crm;
    printf("\n[Remover Veterinario]\n");
    printf("Informe o CRM: ");
    if (scanf("%d", &crm) != 1) { limpar_buffer_entrada(); printf("Entrada invalida.\n"); return; }

    int idx = encontrar_indice_veterinario_por_crm(crm);
    if (idx < 0) { printf("Veterinario nao encontrado.\n"); return; }

    // OBS: Na próxima entrega, bloquear se houver consultas vinculadas ao CRM informado

    for (int i = idx; i < g_nVets - 1; i++) {
        g_vets[i] = g_vets[i + 1];
    }
    g_nVets--;
    printf("Veterinario removido.\n");
}

static void consultar_veterinario_por_crm(void) {
    int crm;
    printf("\n[Consultar Veterinario por CRM]\n");
    printf("Informe o CRM: ");
    if (scanf("%d", &crm) != 1) { limpar_buffer_entrada(); printf("Entrada invalida.\n"); return; }

    int idx = encontrar_indice_veterinario_por_crm(crm);
    if (idx < 0) { printf("Veterinario nao encontrado.\n"); return; }

    Veterinario *v = &g_vets[idx];
    printf("CRM %d | Nome: %s | Tel: %s\n", v->crmVet, v->nome, v->telefone);
}

static void listar_veterinarios(void) {
    printf("\n[Listar Veterinarios]\n");
    if (g_nVets == 0) { printf("Nenhum veterinario cadastrado.\n"); return; }
    for (int i = 0; i < g_nVets; i++) {
        printf("CRM %d | Nome: %s | Tel: %s\n",
               g_vets[i].crmVet, g_vets[i].nome, g_vets[i].telefone);
    }
}

// Função utilitária menu
static void cabecalho(const char *titulo) {
    printf("\n========================================\n");
    printf("  %s\n", titulo);
    printf("========================================\n");
}

// Submenu: Consultar (Animal)
static void submenu_consultar_animais(void) {
    int op;
    do {
        cabecalho("ANIMAIS — CONSULTAR");
        printf("(1) Consultar por ID\n");
        printf("(2) Listar todos\n");
        printf("(0) Voltar\n");
        printf("----------------------------------------\n");
        printf("Escolha: ");
        if (scanf("%d", &op) != 1) { limpar_buffer_entrada(); op = -1; }

        switch (op) {
            case 1: consultar_animal_por_id(); break;
            case 2: listar_animais(); break;
            case 0: break;
            default: printf("Opcao invalida.\n");
        }
    } while (op != 0);
}

// Submenu: Consultar (Veterinario)
static void submenu_consultar_veterinarios(void) {
    int op;
    do {
        cabecalho("VETERINARIOS — CONSULTAR");
        printf("(1) Consultar por CRM\n");
        printf("(2) Listar todos\n");
        printf("(0) Voltar\n");
        printf("----------------------------------------\n");
        printf("Escolha: ");
        if (scanf("%d", &op) != 1) { limpar_buffer_entrada(); op = -1; }

        switch (op) {
            case 1: consultar_veterinario_por_crm(); break;
            case 2: listar_veterinarios(); break;
            case 0: break;
            default: printf("Opcao invalida.\n");
        }
    } while (op != 0);
}

// Menu Animais
static void menu_animais(void) {
    int op;
    do {
        cabecalho("MENU — ANIMAIS");
        printf("(1) Cadastrar\n");
        printf("(2) Atualizar\n");
        printf("(3) Remover\n");
        printf("(4) Consultar\n");
        printf("(0) Voltar\n");
        printf("----------------------------------------\n");
        printf("Escolha: ");
        if (scanf("%d", &op) != 1) { limpar_buffer_entrada(); op = -1; }

        switch (op) {
            case 1: cadastrar_animal(); break;
            case 2: atualizar_animal(); break;
            case 3: remover_animal(); break;
            case 4: submenu_consultar_animais(); break;
            case 0: break;
            default: printf("Opcao invalida.\n");
        }
    } while (op != 0);
}

// Menu Veterinarios
static void menu_veterinarios(void) {
    int op;
    do {
        cabecalho("MENU — VETERINARIOS");
        printf("(1) Cadastrar\n");
        printf("(2) Atualizar\n");
        printf("(3) Remover\n");
        printf("(4) Consultar\n");
        printf("(0) Voltar\n");
        printf("----------------------------------------\n");
        printf("Escolha: ");
        if (scanf("%d", &op) != 1) { limpar_buffer_entrada(); op = -1; }

        switch (op) {
            case 1: cadastrar_veterinario(); break;
            case 2: atualizar_veterinario(); break;
            case 3: remover_veterinario(); break;
            case 4: submenu_consultar_veterinarios(); break;
            case 0: break;
            default: printf("Opcao invalida.\n");
        }
    } while (op != 0);
}

// Submenus (próxima entrega)
static void menu_consultas(void) {
    cabecalho("MENU — CONSULTAS");
    printf("** Em desenvolvimento para a proxima entrega **\n");
    /* TODO: CRUD de Consultas e filtros */
}
static void menu_relatorios(void) {
    cabecalho("MENU — RELATORIOS");
    printf("** Em desenvolvimento para a proxima entrega **\n");
    /* TODO: Geracao de arquivos .txt */
}

// Menu principal
static void menu_principal(void) {
    int escolha;
    do {
        cabecalho("CLINICA VET — MENU PRINCIPAL");
        printf("(1) Animais\n");
        printf("(2) Veterinarios\n");
        printf("(3) Consultas\n");
        printf("(4) Relatorios (TXT)\n");
        printf("(0) Sair\n");
        printf("----------------------------------------\n");
        printf("Escolha: ");
        if (scanf("%d", &escolha) != 1) { limpar_buffer_entrada(); escolha = -1; }

        switch (escolha) {
            case 1: menu_animais(); break;
            case 2: menu_veterinarios(); break;
            case 3: menu_consultas(); break;   // stub
            case 4: menu_relatorios(); break;  // stub
            case 0: printf("Saindo...\n"); break;
            default: printf("Opcao invalida.\n");
        }
    } while (escolha != 0);
}

// Inicialização
static int inicializar_aplicacao(void) {
    g_capAnimais = GROWTH_STEP;
    g_capVets    = GROWTH_STEP;
    g_capCons    = GROWTH_STEP; // reservado

    g_animais = (Animal*)malloc(g_capAnimais * sizeof(Animal));
    g_vets    = (Veterinario*)malloc(g_capVets    * sizeof(Veterinario));
    g_consultas = (Consulta*)malloc(g_capCons * sizeof(Consulta));

    if (!g_animais || !g_vets || !g_consultas) {
        printf("Falha de memoria na inicializacao.\n");
        free(g_animais); free(g_vets); free(g_consultas);
        return 0;
    }
    return 1;
}

int main(void) {
    if (!inicializar_aplicacao()) return 1;
    menu_principal();

    free(g_animais);
    free(g_vets);
    free(g_consultas);
    return 0;
}
