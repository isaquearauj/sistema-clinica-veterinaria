#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOME_TAM 50
#define ESPECIE_TAM 30
#define DATA_TAM 11 // "DD/MM/AAAA" + '\0'
#define TELEFONE_TAM 16
#define GROWTH_STEP 5

#define ARQ_ANIMAIS "animais.bin"
#define ARQ_VETS "veterinarios.bin"
#define ARQ_CONS "consultas.bin"

// ======== Estruturas ========
typedef struct
{
    int idAnimal;
    char nome[NOME_TAM];
    char especie[ESPECIE_TAM];
    char dataNascimento[DATA_TAM];
    double peso;
} Animal;

typedef struct
{
    int crmVet;
    char nome[NOME_TAM];
    char telefone[TELEFONE_TAM];
} Veterinario;

typedef struct
{
    int idConsulta;
    int idAnimal; // FK -> Animal.idAnimal
    int crmVet;   // FK -> Veterinario.crmVet
    char dataConsulta[DATA_TAM];
    double valor;
} Consulta;

static Animal *g_animais = NULL;
static int g_nAnimais = 0;
static int g_capAnimais = 0;

static Veterinario *g_vets = NULL;
static int g_nVets = 0;
static int g_capVets = 0;

static Consulta *g_consultas = NULL;
static int g_nCons = 0;
static int g_capCons = 0;

static int g_nextIdAnimal = 1;
static int g_nextIdConsulta = 1;

// ======== Utilidades ========
static void limpar_buffer_entrada()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}

// Conversões de data "DD/MM/AAAA"
static int parse_data(const char *s, int *dd, int *mm, int *aaaa)
{
    if (!s || strlen(s) < 10)
        return 0;
    // Espera "DD/MM/AAAA"
    if (sscanf(s, "%2d/%2d/%4d", dd, mm, aaaa) != 3)
        return 0;
    if (*dd < 1 || *dd > 31 || *mm < 1 || *mm > 12 || *aaaa < 1)
        return 0;
    return 1;
}
static int data_to_int(const char *s)
{
    int d, m, a;
    if (!parse_data(s, &d, &m, &a))
        return -1;
    return a * 10000 + m * 100 + d; // AAAAMMDD
}

// ======== Capacidade dinâmica ========
static int garantir_capacidade_animais(int extra)
{
    if (g_nAnimais + extra <= g_capAnimais)
        return 1;
    int novo = g_capAnimais + GROWTH_STEP;
    Animal *p = (Animal *)realloc(g_animais, novo * sizeof(Animal));
    if (!p)
        return 0;
    g_animais = p;
    g_capAnimais = novo;
    return 1;
}
static int garantir_capacidade_veterinarios(int extra)
{
    if (g_nVets + extra <= g_capVets)
        return 1;
    int novo = g_capVets + GROWTH_STEP;
    Veterinario *p = (Veterinario *)realloc(g_vets, novo * sizeof(Veterinario));
    if (!p)
        return 0;
    g_vets = p;
    g_capVets = novo;
    return 1;
}
static int garantir_capacidade_consultas(int extra)
{
    if (g_nCons + extra <= g_capCons)
        return 1;
    int novo = g_capCons + GROWTH_STEP;
    Consulta *p = (Consulta *)realloc(g_consultas, novo * sizeof(Consulta));
    if (!p)
        return 0;
    g_consultas = p;
    g_capCons = novo;
    return 1;
}

// Ajuste para manter até 5 posições livres
static void ajustar_capacidade_animais()
{
    int livres = g_capAnimais - g_nAnimais;
    if (livres > GROWTH_STEP)
    {
        int novo = g_nAnimais + GROWTH_STEP;
        Animal *p = (Animal *)realloc(g_animais, novo * sizeof(Animal));
        if (p)
        {
            g_animais = p;
            g_capAnimais = novo;
        }
    }
}
static void ajustar_capacidade_veterinarios()
{
    int livres = g_capVets - g_nVets;
    if (livres > GROWTH_STEP)
    {
        int novo = g_nVets + GROWTH_STEP;
        Veterinario *p = (Veterinario *)realloc(g_vets, novo * sizeof(Veterinario));
        if (p)
        {
            g_vets = p;
            g_capVets = novo;
        }
    }
}
static void ajustar_capacidade_consultas()
{
    int livres = g_capCons - g_nCons;
    if (livres > GROWTH_STEP)
    {
        int novo = g_nCons + GROWTH_STEP;
        Consulta *p = (Consulta *)realloc(g_consultas, novo * sizeof(Consulta));
        if (p)
        {
            g_consultas = p;
            g_capCons = novo;
        }
    }
}

// ======== Buscas ========
static int encontrar_indice_animal_por_id(int id)
{
    for (int i = 0; i < g_nAnimais; i++)
        if (g_animais[i].idAnimal == id)
            return i;
    return -1;
}
static int encontrar_indice_veterinario_por_crm(int crm)
{
    for (int i = 0; i < g_nVets; i++)
        if (g_vets[i].crmVet == crm)
            return i;
    return -1;
}
static int encontrar_indice_consulta_por_id(int id)
{
    for (int i = 0; i < g_nCons; i++)
        if (g_consultas[i].idConsulta == id)
            return i;
    return -1;
}
static int tem_consulta_para_animal(int idAnimal)
{
    for (int i = 0; i < g_nCons; i++)
        if (g_consultas[i].idAnimal == idAnimal)
            return 1;
    return 0;
}
static int tem_consulta_para_vet(int crm)
{
    for (int i = 0; i < g_nCons; i++)
        if (g_consultas[i].crmVet == crm)
            return 1;
    return 0;
}

// ======== Persistência ========
static int salvar_animais(const char *path)
{
    FILE *f = fopen(path, "wb");
    if (!f)
        return 0;
    if (fwrite(&g_nAnimais, sizeof(int), 1, f) != 1)
    {
        fclose(f);
        return 0;
    }
    if (g_nAnimais > 0 && fwrite(g_animais, sizeof(Animal), g_nAnimais, f) != (size_t)g_nAnimais)
    {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}
static int carregar_animais(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        g_nAnimais = 0;
        g_capAnimais = 0;
        g_animais = NULL;
        g_nextIdAnimal = 1;
        return 1;
    }

    int qtd = 0;
    if (fread(&qtd, sizeof(int), 1, f) != 1 || qtd < 0)
    {
        fclose(f);
        return 0;
    }

    int cap = ((qtd / GROWTH_STEP) + 1) * GROWTH_STEP;
    Animal *p = (Animal *)malloc((cap > 0 ? cap : GROWTH_STEP) * sizeof(Animal));
    if (!p)
    {
        fclose(f);
        return 0;
    }

    if (qtd > 0 && fread(p, sizeof(Animal), qtd, f) != (size_t)qtd)
    {
        free(p);
        fclose(f);
        return 0;
    }
    fclose(f);

    if (g_animais)
        free(g_animais);
    g_animais = p;
    g_nAnimais = qtd;
    g_capAnimais = (cap > 0 ? cap : GROWTH_STEP);

    int maxId = 0;
    for (int i = 0; i < g_nAnimais; i++)
        if (g_animais[i].idAnimal > maxId)
            maxId = g_animais[i].idAnimal;
    g_nextIdAnimal = maxId + 1;
    if (g_nextIdAnimal < 1)
        g_nextIdAnimal = 1;

    return 1;
}

static int salvar_vets(const char *path)
{
    FILE *f = fopen(path, "wb");
    if (!f)
        return 0;
    if (fwrite(&g_nVets, sizeof(int), 1, f) != 1)
    {
        fclose(f);
        return 0;
    }
    if (g_nVets > 0 && fwrite(g_vets, sizeof(Veterinario), g_nVets, f) != (size_t)g_nVets)
    {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}
static int carregar_vets(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        g_nVets = 0;
        g_capVets = 0;
        g_vets = NULL;
        return 1;
    }

    int qtd = 0;
    if (fread(&qtd, sizeof(int), 1, f) != 1 || qtd < 0)
    {
        fclose(f);
        return 0;
    }

    int cap = ((qtd / GROWTH_STEP) + 1) * GROWTH_STEP;
    Veterinario *p = (Veterinario *)malloc((cap > 0 ? cap : GROWTH_STEP) * sizeof(Veterinario));
    if (!p)
    {
        fclose(f);
        return 0;
    }

    if (qtd > 0 && fread(p, sizeof(Veterinario), qtd, f) != (size_t)qtd)
    {
        free(p);
        fclose(f);
        return 0;
    }
    fclose(f);

    if (g_vets)
        free(g_vets);
    g_vets = p;
    g_nVets = qtd;
    g_capVets = (cap > 0 ? cap : GROWTH_STEP);
    return 1;
}

static int salvar_cons(const char *path)
{
    FILE *f = fopen(path, "wb");
    if (!f)
        return 0;
    if (fwrite(&g_nCons, sizeof(int), 1, f) != 1)
    {
        fclose(f);
        return 0;
    }
    if (g_nCons > 0 && fwrite(g_consultas, sizeof(Consulta), g_nCons, f) != (size_t)g_nCons)
    {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}
static int carregar_cons(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        g_nCons = 0;
        g_capCons = 0;
        g_consultas = NULL;
        g_nextIdConsulta = 1;
        return 1;
    }

    int qtd = 0;
    if (fread(&qtd, sizeof(int), 1, f) != 1 || qtd < 0)
    {
        fclose(f);
        return 0;
    }

    int cap = ((qtd / GROWTH_STEP) + 1) * GROWTH_STEP;
    Consulta *p = (Consulta *)malloc((cap > 0 ? cap : GROWTH_STEP) * sizeof(Consulta));
    if (!p)
    {
        fclose(f);
        return 0;
    }

    if (qtd > 0 && fread(p, sizeof(Consulta), qtd, f) != (size_t)qtd)
    {
        free(p);
        fclose(f);
        return 0;
    }
    fclose(f);

    if (g_consultas)
        free(g_consultas);
    g_consultas = p;
    g_nCons = qtd;
    g_capCons = (cap > 0 ? cap : GROWTH_STEP);

    int maxId = 0;
    for (int i = 0; i < g_nCons; i++)
        if (g_consultas[i].idConsulta > maxId)
            maxId = g_consultas[i].idConsulta;
    g_nextIdConsulta = maxId + 1;
    if (g_nextIdConsulta < 1)
        g_nextIdConsulta = 1;

    return 1;
}

// ======== Cabecalho/Menu util ========
static void cabecalho(const char *titulo)
{
    printf("\n========================================\n");
    printf("  %s\n", titulo);
    printf("========================================\n");
}

// ======== CRUD: Animais ========
static void cadastrar_animal()
{
    if (!garantir_capacidade_animais(1))
    {
        printf("Erro de memoria.\n");
        return;
    }

    Animal a;
    a.idAnimal = g_nextIdAnimal++;
    printf("\n[Cadastrar Animal]\n");
    printf("Nome: ");
    limpar_buffer_entrada();
    scanf(" %49[^\n]", a.nome);
    printf("Especie: ");
    scanf(" %29[^\n]", a.especie);
    printf("Data de Nascimento (DD/MM/AAAA): ");
    scanf(" %10s", a.dataNascimento);
    printf("Peso (kg): ");
    if (scanf("%lf", &a.peso) != 1)
    {
        limpar_buffer_entrada();
        a.peso = 0.0;
    }

    g_animais[g_nAnimais++] = a;
    printf("Animal cadastrado com id %d.\n", a.idAnimal);
}

static void atualizar_animal()
{
    int id;
    printf("\n[Atualizar Animal]\nInforme o id do animal: ");
    if (scanf("%d", &id) != 1)
    {
        limpar_buffer_entrada();
        printf("Entrada invalida.\n");
        return;
    }

    int idx = encontrar_indice_animal_por_id(id);
    if (idx < 0)
    {
        printf("Animal nao encontrado.\n");
        return;
    }

    int opc;
    do
    {
        printf("\n--- Escolha o campo para atualizar ---\n");
        printf("(1) Nome\n");
        printf("(2) Especie\n");
        printf("(3) Data de Nascimento\n");
        printf("(4) Peso (kg)\n");
        printf("(0) Concluir\n");
        printf("Opcao: ");
        if (scanf("%d", &opc) != 1)
        {
            limpar_buffer_entrada();
            opc = -1;
        }

        switch (opc)
        {
        case 1:
            limpar_buffer_entrada();
            printf("Novo nome: ");
            scanf(" %49[^\n]", g_animais[idx].nome);
            break;
        case 2:
            limpar_buffer_entrada();
            printf("Nova especie: ");
            scanf(" %29[^\n]", g_animais[idx].especie);
            break;
        case 3:
            printf("Nova data (DD/MM/AAAA): ");
            scanf(" %10s", g_animais[idx].dataNascimento);
            break;
        case 4:
            printf("Novo peso (kg): ");
            if (scanf("%lf", &g_animais[idx].peso) != 1)
            {
                limpar_buffer_entrada();
            }
            break;
        case 0:
            printf("Atualizacao concluida.\n");
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (opc != 0);
}

static void remover_animal()
{
    int id;
    printf("\n[Remover Animal]\nInforme o id do animal: ");
    if (scanf("%d", &id) != 1)
    {
        limpar_buffer_entrada();
        printf("Entrada invalida.\n");
        return;
    }

    int idx = encontrar_indice_animal_por_id(id);
    if (idx < 0)
    {
        printf("Animal nao encontrado.\n");
        return;
    }

    if (tem_consulta_para_animal(id))
    {
        printf("Nao e possivel remover: ha consultas vinculadas.\n");
        return;
    }

    for (int i = idx; i < g_nAnimais - 1; i++)
        g_animais[i] = g_animais[i + 1];
    g_nAnimais--;
    ajustar_capacidade_animais();
    printf("Animal removido.\n");
}

static void consultar_animal_por_id()
{
    int id;
    printf("\n[Consultar Animal por ID]\nInforme o id: ");
    if (scanf("%d", &id) != 1)
    {
        limpar_buffer_entrada();
        printf("Entrada invalida.\n");
        return;
    }

    int idx = encontrar_indice_animal_por_id(id);
    if (idx < 0)
    {
        printf("Animal nao encontrado.\n");
        return;
    }

    Animal *a = &g_animais[idx];
    printf("\n#%d | Nome: %s | Especie: %s | Nasc: %s | Peso: %.2f kg\n",
           a->idAnimal, a->nome, a->especie, a->dataNascimento, a->peso);
}

static void listar_animais()
{
    printf("\n[Listar Animais]\n");
    if (g_nAnimais == 0)
    {
        printf("Nenhum animal cadastrado.\n");
        return;
    }
    for (int i = 0; i < g_nAnimais; i++)
    {
        printf("#%d | Nome: %s | Especie: %s | Nasc: %s | Peso: %.2f kg\n",
               g_animais[i].idAnimal, g_animais[i].nome,
               g_animais[i].especie, g_animais[i].dataNascimento, g_animais[i].peso);
    }
}

// ======== CRUD: Veterinários ========
static void cadastrar_veterinario()
{
    if (!garantir_capacidade_veterinarios(1))
    {
        printf("Erro de memoria.\n");
        return;
    }

    Veterinario v;
    printf("\n[Cadastrar Veterinario]\n");
    printf("CRM (apenas numeros): ");
    if (scanf("%d", &v.crmVet) != 1)
    {
        limpar_buffer_entrada();
        printf("Entrada invalida.\n");
        return;
    }
    if (encontrar_indice_veterinario_por_crm(v.crmVet) != -1)
    {
        printf("Ja existe veterinario com esse CRM.\n");
        return;
    }

    printf("Nome: ");
    limpar_buffer_entrada();
    scanf(" %49[^\n]", v.nome);
    printf("Telefone: ");
    scanf(" %15s", v.telefone);

    g_vets[g_nVets++] = v;
    printf("Veterinario cadastrado (CRM %d).\n", v.crmVet);
}

static void atualizar_veterinario()
{
    int crm;
    printf("\n[Atualizar Veterinario]\nInforme o CRM: ");
    if (scanf("%d", &crm) != 1)
    {
        limpar_buffer_entrada();
        printf("Entrada invalida.\n");
        return;
    }

    int idx = encontrar_indice_veterinario_por_crm(crm);
    if (idx < 0)
    {
        printf("Veterinario nao encontrado.\n");
        return;
    }

    int opc;
    do
    {
        printf("\n--- Escolha o campo para atualizar ---\n");
        printf("(1) Nome\n");
        printf("(2) Telefone\n");
        printf("(0) Concluir\n");
        printf("Opcao: ");
        if (scanf("%d", &opc) != 1)
        {
            limpar_buffer_entrada();
            opc = -1;
        }

        switch (opc)
        {
        case 1:
            limpar_buffer_entrada();
            printf("Novo nome: ");
            scanf(" %49[^\n]", g_vets[idx].nome);
            break;
        case 2:
            printf("Novo telefone: ");
            scanf(" %15s", g_vets[idx].telefone);
            break;
        case 0:
            printf("Atualizacao concluida.\n");
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (opc != 0);
}

static void remover_veterinario()
{
    int crm;
    printf("\n[Remover Veterinario]\nInforme o CRM: ");
    if (scanf("%d", &crm) != 1)
    {
        limpar_buffer_entrada();
        printf("Entrada invalida.\n");
        return;
    }

    int idx = encontrar_indice_veterinario_por_crm(crm);
    if (idx < 0)
    {
        printf("Veterinario nao encontrado.\n");
        return;
    }

    if (tem_consulta_para_vet(crm))
    {
        printf("Nao e possivel remover: ha consultas vinculadas.\n");
        return;
    }

    for (int i = idx; i < g_nVets - 1; i++)
        g_vets[i] = g_vets[i + 1];
    g_nVets--;
    ajustar_capacidade_veterinarios();
    printf("Veterinario removido.\n");
}

static void consultar_veterinario_por_crm()
{
    int crm;
    printf("\n[Consultar Veterinario por CRM]\nInforme o CRM: ");
    if (scanf("%d", &crm) != 1)
    {
        limpar_buffer_entrada();
        printf("Entrada invalida.\n");
        return;
    }

    int idx = encontrar_indice_veterinario_por_crm(crm);
    if (idx < 0)
    {
        printf("Veterinario nao encontrado.\n");
        return;
    }

    Veterinario *v = &g_vets[idx];
    printf("CRM %d | Nome: %s | Tel: %s\n", v->crmVet, v->nome, v->telefone);
}

static void listar_veterinarios()
{
    printf("\n[Listar Veterinarios]\n");
    if (g_nVets == 0)
    {
        printf("Nenhum veterinario cadastrado.\n");
        return;
    }
    for (int i = 0; i < g_nVets; i++)
    {
        printf("CRM %d | Nome: %s | Tel: %s\n",
               g_vets[i].crmVet, g_vets[i].nome, g_vets[i].telefone);
    }
}

// ======== CRUD: Consultas ========
static void cadastrar_consulta()
{
    if (!garantir_capacidade_consultas(1))
    {
        printf("Erro de memoria.\n");
        return;
    }

    Consulta c;
    printf("\n[Cadastrar Consulta]\n");
    printf("ID do Animal: ");
    if (scanf("%d", &c.idAnimal) != 1)
    {
        limpar_buffer_entrada();
        printf("Entrada invalida.\n");
        return;
    }
    if (encontrar_indice_animal_por_id(c.idAnimal) < 0)
    {
        printf("Animal inexistente.\n");
        return;
    }

    printf("CRM do Veterinario: ");
    if (scanf("%d", &c.crmVet) != 1)
    {
        limpar_buffer_entrada();
        printf("Entrada invalida.\n");
        return;
    }
    if (encontrar_indice_veterinario_por_crm(c.crmVet) < 0)
    {
        printf("Veterinario inexistente.\n");
        return;
    }

    printf("Data da consulta (DD/MM/AAAA): ");
    scanf(" %10s", c.dataConsulta);
    printf("Valor (R$): ");
    if (scanf("%lf", &c.valor) != 1)
    {
        limpar_buffer_entrada();
        c.valor = 0.0;
    }

    c.idConsulta = g_nextIdConsulta++;

    g_consultas[g_nCons++] = c;
    printf("Consulta cadastrada (id %d).\n", c.idConsulta);
}

static void atualizar_consulta()
{
    int id;
    printf("\n[Atualizar Consulta]\nInforme o id da consulta: ");
    if (scanf("%d", &id) != 1)
    {
        limpar_buffer_entrada();
        printf("Entrada invalida.\n");
        return;
    }

    int idx = encontrar_indice_consulta_por_id(id);
    if (idx < 0)
    {
        printf("Consulta nao encontrada.\n");
        return;
    }

    int opc;
    do
    {
        printf("\n--- Escolha o campo para atualizar ---\n");
        printf("(1) ID Animal\n");
        printf("(2) CRM Veterinario\n");
        printf("(3) Data da Consulta\n");
        printf("(4) Valor (R$)\n");
        printf("(0) Concluir\n");
        printf("Opcao: ");
        if (scanf("%d", &opc) != 1)
        {
            limpar_buffer_entrada();
            opc = -1;
        }

        switch (opc)
        {
        case 1:
        {
            int novoId;
            printf("Novo ID Animal: ");
            if (scanf("%d", &novoId) != 1)
            {
                limpar_buffer_entrada();
                printf("Invalido.\n");
                break;
            }
            if (encontrar_indice_animal_por_id(novoId) < 0)
            {
                printf("Animal inexistente.\n");
                break;
            }
            g_consultas[idx].idAnimal = novoId;
        }
        break;
        case 2:
        {
            int novoCrm;
            printf("Novo CRM Vet: ");
            if (scanf("%d", &novoCrm) != 1)
            {
                limpar_buffer_entrada();
                printf("Invalido.\n");
                break;
            }
            if (encontrar_indice_veterinario_por_crm(novoCrm) < 0)
            {
                printf("Vet inexistente.\n");
                break;
            }
            g_consultas[idx].crmVet = novoCrm;
        }
        break;
        case 3:
            printf("Nova data (DD/MM/AAAA): ");
            scanf(" %10s", g_consultas[idx].dataConsulta);
            break;
        case 4:
            printf("Novo valor (R$): ");
            if (scanf("%lf", &g_consultas[idx].valor) != 1)
            {
                limpar_buffer_entrada();
            }
            break;
        case 0:
            printf("Atualizacao concluida.\n");
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (opc != 0);
}

static void remover_consulta()
{
    int id;
    printf("\n[Remover Consulta]\nInforme o id da consulta: ");
    if (scanf("%d", &id) != 1)
    {
        limpar_buffer_entrada();
        printf("Entrada invalida.\n");
        return;
    }

    int idx = encontrar_indice_consulta_por_id(id);
    if (idx < 0)
    {
        printf("Consulta nao encontrada.\n");
        return;
    }

    for (int i = idx; i < g_nCons - 1; i++)
        g_consultas[i] = g_consultas[i + 1];
    g_nCons--;
    ajustar_capacidade_consultas();
    printf("Consulta removida.\n");
}

// ======== Listagens / Filtros da Estrutura 3 ========
static void mostrar_consulta_expandida(const Consulta *c)
{
    // Busca Animal e Vet
    int ia = encontrar_indice_animal_por_id(c->idAnimal);
    int iv = encontrar_indice_veterinario_por_crm(c->crmVet);

    const char *nomeA = (ia >= 0 ? g_animais[ia].nome : "??");
    const char *espA = (ia >= 0 ? g_animais[ia].especie : "??");
    const char *nomeV = (iv >= 0 ? g_vets[iv].nome : "??");
    const char *telV = (iv >= 0 ? g_vets[iv].telefone : "??");

    printf("#%d | Data: %s | Valor: R$ %.2f | Animal: %s (id %d, %s) | Vet: %s (CRM %d, %s)\n",
           c->idConsulta, c->dataConsulta, c->valor,
           nomeA, c->idAnimal, espA,
           nomeV, c->crmVet, telV);
}

static void listar_todas_consultas()
{
    printf("\n[Listar Consultas]\n");
    if (g_nCons == 0)
    {
        printf("Nenhuma consulta cadastrada.\n");
        return;
    }
    for (int i = 0; i < g_nCons; i++)
    {
        mostrar_consulta_expandida(&g_consultas[i]);
    }
}

static void listar_consultas_por_data_min()
{
    char data[DATA_TAM];
    printf("\n[Consultas a partir de uma data]\nData (DD/MM/AAAA): ");
    scanf(" %10s", data);
    int corte = data_to_int(data);
    if (corte < 0)
    {
        printf("Data invalida.\n");
        return;
    }

    int achou = 0;
    for (int i = 0; i < g_nCons; i++)
    {
        if (data_to_int(g_consultas[i].dataConsulta) >= corte)
        {
            mostrar_consulta_expandida(&g_consultas[i]);
            achou = 1;
        }
    }
    if (!achou)
        printf("Nenhum registro encontrado.\n");
}

static void listar_consultas_por_crm()
{
    int crm;
    printf("\n[Consultas por CRM do veterinario]\nCRM: ");
    if (scanf("%d", &crm) != 1)
    {
        limpar_buffer_entrada();
        printf("Entrada invalida.\n");
        return;
    }
    int achou = 0;
    for (int i = 0; i < g_nCons; i++)
        if (g_consultas[i].crmVet == crm)
        {
            mostrar_consulta_expandida(&g_consultas[i]);
            achou = 1;
        }
    if (!achou)
        printf("Nenhum registro encontrado.\n");
}

static void listar_consultas_por_especie()
{
    char esp[ESPECIE_TAM];
    printf("\n[Consultas por especie do animal]\nEspecie: ");
    limpar_buffer_entrada();
    scanf(" %29[^\n]", esp);

    int achou = 0;
    for (int i = 0; i < g_nCons; i++)
    {
        int ia = encontrar_indice_animal_por_id(g_consultas[i].idAnimal);
        if (ia >= 0 && strcasecmp(g_animais[ia].especie, esp) == 0)
        {
            mostrar_consulta_expandida(&g_consultas[i]);
            achou = 1;
        }
    }
    if (!achou)
        printf("Nenhum registro encontrado.\n");
}

// ======== Relatórios .txt ========
static void gerar_relatorio_data_min()
{
    char data[DATA_TAM];
    printf("\n[Relatorio] Consultas a partir de uma data\nData (DD/MM/AAAA): ");
    scanf(" %10s", data);
    int corte = data_to_int(data);
    if (corte < 0)
    {
        printf("Data invalida.\n");
        return;
    }

    FILE *f = fopen("relatorio_data.txt", "w");
    if (!f)
    {
        printf("Erro ao criar arquivo.\n");
        return;
    }

    int total = 0;
    fprintf(f, "RELATORIO: Consultas a partir de %s\n\n", data);
    for (int i = 0; i < g_nCons; i++)
    {
        if (data_to_int(g_consultas[i].dataConsulta) >= corte)
        {
            int ia = encontrar_indice_animal_por_id(g_consultas[i].idAnimal);
            int iv = encontrar_indice_veterinario_por_crm(g_consultas[i].crmVet);
            fprintf(f, "#%d | Data: %s | Valor: %.2f | Animal: %s (id %d, %s) | Vet: %s (CRM %d)\n",
                    g_consultas[i].idConsulta, g_consultas[i].dataConsulta, g_consultas[i].valor,
                    ia >= 0 ? g_animais[ia].nome : "??", g_consultas[i].idAnimal, ia >= 0 ? g_animais[ia].especie : "??",
                    iv >= 0 ? g_vets[iv].nome : "??", g_consultas[i].crmVet);
            total++;
        }
    }
    fprintf(f, "\nTotal: %d consultas.\n", total);
    fclose(f);
    printf("Gerado: relatorio_data.txt\n");
}

static void gerar_relatorio_crm()
{
    int crm;
    printf("\n[Relatorio] Consultas por CRM do veterinario\nCRM: ");
    if (scanf("%d", &crm) != 1)
    {
        limpar_buffer_entrada();
        printf("Entrada invalida.\n");
        return;
    }

    char nomeArq[64];
    snprintf(nomeArq, sizeof(nomeArq), "relatorio_crm_%d.txt", crm);
    FILE *f = fopen(nomeArq, "w");
    if (!f)
    {
        printf("Erro ao criar arquivo.\n");
        return;
    }

    int total = 0;
    fprintf(f, "RELATORIO: Consultas por CRM %d\n\n", crm);
    for (int i = 0; i < g_nCons; i++)
        if (g_consultas[i].crmVet == crm)
        {
            int ia = encontrar_indice_animal_por_id(g_consultas[i].idAnimal);
            int iv = encontrar_indice_veterinario_por_crm(g_consultas[i].crmVet);
            fprintf(f, "#%d | Data: %s | Valor: %.2f | Animal: %s (id %d, %s) | Vet: %s (CRM %d)\n",
                    g_consultas[i].idConsulta, g_consultas[i].dataConsulta, g_consultas[i].valor,
                    ia >= 0 ? g_animais[ia].nome : "??", g_consultas[i].idAnimal, ia >= 0 ? g_animais[ia].especie : "??",
                    iv >= 0 ? g_vets[iv].nome : "??", g_consultas[i].crmVet);
            total++;
        }
    fprintf(f, "\nTotal: %d consultas.\n", total);
    fclose(f);
    printf("Gerado: %s\n", nomeArq);
}

static void gerar_relatorio_especie()
{
    char esp[ESPECIE_TAM];
    printf("\n[Relatorio] Consultas por especie do animal\nEspecie: ");
    limpar_buffer_entrada();
    scanf(" %29[^\n]", esp);

    char nomeArq[96];
    snprintf(nomeArq, sizeof(nomeArq), "relatorio_especie_%s.txt", esp);
    // Trocar espaços por '_' para nome de arquivo
    for (char *p = nomeArq; *p; ++p)
        if (*p == ' ')
            *p = '_';
    FILE *f = fopen(nomeArq, "w");
    if (!f)
    {
        printf("Erro ao criar arquivo.\n");
        return;
    }

    int total = 0;
    fprintf(f, "RELATORIO: Consultas por especie '%s'\n\n", esp);
    for (int i = 0; i < g_nCons; i++)
    {
        int ia = encontrar_indice_animal_por_id(g_consultas[i].idAnimal);
        if (ia >= 0 && strcasecmp(g_animais[ia].especie, esp) == 0)
        {
            int iv = encontrar_indice_veterinario_por_crm(g_consultas[i].crmVet);
            fprintf(f, "#%d | Data: %s | Valor: %.2f | Animal: %s (id %d, %s) | Vet: %s (CRM %d)\n",
                    g_consultas[i].idConsulta, g_consultas[i].dataConsulta, g_consultas[i].valor,
                    g_animais[ia].nome, g_consultas[i].idAnimal, g_animais[ia].especie,
                    iv >= 0 ? g_vets[iv].nome : "??", g_consultas[i].crmVet);
            total++;
        }
    }
    fprintf(f, "\nTotal: %d consultas.\n", total);
    fclose(f);
    printf("Gerado: %s\n", nomeArq);
}

// ======== Submenus ========
static void submenu_consultar_animais()
{
    int op;
    do
    {
        cabecalho("ANIMAIS => CONSULTAR");
        printf("(1) Consultar por ID\n");
        printf("(2) Listar todos\n");
        printf("(0) Voltar\n");
        printf("----------------------------------------\n");
        printf("Escolha: ");
        if (scanf("%d", &op) != 1)
        {
            limpar_buffer_entrada();
            op = -1;
        }

        switch (op)
        {
        case 1:
            consultar_animal_por_id();
            break;
        case 2:
            listar_animais();
            break;
        case 0:
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (op != 0);
}

static void submenu_consultar_veterinarios()
{
    int op;
    do
    {
        cabecalho("VETERINARIOS => CONSULTAR");
        printf("(1) Consultar por CRM\n");
        printf("(2) Listar todos\n");
        printf("(0) Voltar\n");
        printf("----------------------------------------\n");
        printf("Escolha: ");
        if (scanf("%d", &op) != 1)
        {
            limpar_buffer_entrada();
            op = -1;
        }

        switch (op)
        {
        case 1:
            consultar_veterinario_por_crm();
            break;
        case 2:
            listar_veterinarios();
            break;
        case 0:
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (op != 0);
}

static void submenu_consultar_consultas()
{
    int op;
    do
    {
        cabecalho("CONSULTAS => CONSULTAR/LISTAR");
        printf("(1) Listar todas\n");
        printf("(2) Filtrar por data minima (>=)\n");
        printf("(3) Filtrar por CRM do veterinario\n");
        printf("(4) Filtrar por especie do animal\n");
        printf("(0) Voltar\n");
        printf("----------------------------------------\n");
        printf("Escolha: ");
        if (scanf("%d", &op) != 1)
        {
            limpar_buffer_entrada();
            op = -1;
        }

        switch (op)
        {
        case 1:
            listar_todas_consultas();
            break;
        case 2:
            listar_consultas_por_data_min();
            break;
        case 3:
            listar_consultas_por_crm();
            break;
        case 4:
            listar_consultas_por_especie();
            break;
        case 0:
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (op != 0);
}

// ======== Menus principais por entidade ========
static void menu_animais()
{
    int op;
    do
    {
        cabecalho("MENU => ANIMAIS");
        printf("(1) Cadastrar\n");
        printf("(2) Atualizar\n");
        printf("(3) Remover\n");
        printf("(4) Consultar/Listar\n");
        printf("(0) Voltar\n");
        printf("----------------------------------------\n");
        printf("Escolha: ");
        if (scanf("%d", &op) != 1)
        {
            limpar_buffer_entrada();
            op = -1;
        }

        switch (op)
        {
        case 1:
            cadastrar_animal();
            break;
        case 2:
            atualizar_animal();
            break;
        case 3:
            remover_animal();
            break;
        case 4:
            submenu_consultar_animais();
            break;
        case 0:
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (op != 0);
}

static void menu_veterinarios()
{
    int op;
    do
    {
        cabecalho("MENU => VETERINARIOS");
        printf("(1) Cadastrar\n");
        printf("(2) Atualizar\n");
        printf("(3) Remover\n");
        printf("(4) Consultar/Listar\n");
        printf("(0) Voltar\n");
        printf("----------------------------------------\n");
        printf("Escolha: ");
        if (scanf("%d", &op) != 1)
        {
            limpar_buffer_entrada();
            op = -1;
        }

        switch (op)
        {
        case 1:
            cadastrar_veterinario();
            break;
        case 2:
            atualizar_veterinario();
            break;
        case 3:
            remover_veterinario();
            break;
        case 4:
            submenu_consultar_veterinarios();
            break;
        case 0:
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (op != 0);
}

// ======== Menu Relatórios ========
static void menu_relatorios()
{
    int op;
    do
    {
        cabecalho("MENU => RELATORIOS (.txt)");
        printf("(1) Gerar por data minima\n");
        printf("(2) Gerar por CRM do veterinario\n");
        printf("(3) Gerar por especie do animal\n");
        printf("(0) Voltar\n");
        printf("----------------------------------------\n");
        printf("Escolha: ");
        if (scanf("%d", &op) != 1)
        {
            limpar_buffer_entrada();
            op = -1;
        }

        switch (op)
        {
        case 1:
            gerar_relatorio_data_min();
            break;
        case 2:
            gerar_relatorio_crm();
            break;
        case 3:
            gerar_relatorio_especie();
            break;
        case 0:
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (op != 0);
}

// ======== Popular exemplos (opcional útil para gerar os .bin com >=10) ========
static void popular_exemplos()
{
    // Limpar atuais
    g_nAnimais = g_nVets = g_nCons = 0;
    g_nextIdAnimal = 1;
    g_nextIdConsulta = 1;

    // Animais (10)
    const char *nomesA[10] = {"Luna", "Thor", "Mel", "Rex", "Nina", "Bob", "Maya", "Max", "Lola", "Koda"};
    const char *espsA[10] = {"Cachorro", "Gato", "Cachorro", "Cachorro", "Gato", "Cachorro", "Pássaro", "Gato", "Cachorro", "Coelho"};
    const char *datasA[10] = {"01/01/2020", "15/03/2019", "22/07/2021", "05/11/2018", "09/09/2020", "30/06/2017", "12/12/2022", "25/05/2016", "10/10/2019", "02/08/2023"};
    double pesosA[10] = {12.5, 4.2, 9.8, 20.1, 3.9, 18.0, 0.2, 5.0, 14.3, 1.1};

    for (int i = 0; i < 10; i++)
    {
        if (!garantir_capacidade_animais(1))
            return;
        Animal a;
        a.idAnimal = g_nextIdAnimal++;
        strncpy(a.nome, nomesA[i], NOME_TAM);
        a.nome[NOME_TAM - 1] = '\0';
        strncpy(a.especie, espsA[i], ESPECIE_TAM);
        a.especie[ESPECIE_TAM - 1] = '\0';
        strncpy(a.dataNascimento, datasA[i], DATA_TAM);
        a.dataNascimento[DATA_TAM - 1] = '\0';
        a.peso = pesosA[i];
        g_animais[g_nAnimais++] = a;
    }

    // Veterinarios (10)
    const char *nomesV[10] = {"Dr. Silva", "Dra. Souza", "Dr. Pereira", "Dra. Rocha", "Dr. Lima", "Dra. Alves", "Dr. Gomes", "Dra. Costa", "Dr. Araujo", "Dra. Moraes"};
    const char *telsV[10] = {"34-1111-1111", "34-2222-2222", "34-3333-3333", "34-4444-4444", "34-5555-5555",
                             "34-6666-6666", "34-7777-7777", "34-8888-8888", "34-9999-9999", "34-1234-5678"};
    for (int i = 0; i < 10; i++)
    {
        if (!garantir_capacidade_veterinarios(1))
            return;
        Veterinario v;
        v.crmVet = 1000 + i; // único
        strncpy(v.nome, nomesV[i], NOME_TAM);
        v.nome[NOME_TAM - 1] = '\0';
        strncpy(v.telefone, telsV[i], TELEFONE_TAM);
        v.telefone[TELEFONE_TAM - 1] = '\0';
        g_vets[g_nVets++] = v;
    }

    // Consultas (10) — distribui datas e valores
    const char *datasC[10] = {"01/09/2024", "15/10/2024", "20/12/2024", "03/01/2025", "18/02/2025",
                              "05/03/2025", "22/04/2025", "10/05/2025", "19/06/2025", "01/07/2025"};
    double valores[10] = {120, 150, 90, 200, 80, 140, 95, 110, 180, 160};

    for (int i = 0; i < 10; i++)
    {
        if (!garantir_capacidade_consultas(1))
            return;
        Consulta c;
        c.idConsulta = g_nextIdConsulta++;
        c.idAnimal = g_animais[(i * 3) % 10].idAnimal;
        c.crmVet = g_vets[(i * 2) % 10].crmVet;
        strncpy(c.dataConsulta, datasC[i], DATA_TAM);
        c.dataConsulta[DATA_TAM - 1] = '\0';
        c.valor = valores[i];
        g_consultas[g_nCons++] = c;
    }

    printf("Dados de exemplo inseridos (10 de cada estrutura).\n");
}

// ======== Menu principal ========
static void menu_consultas()
{
    int op;
    do
    {
        cabecalho("MENU => CONSULTAS");
        printf("(1) Cadastrar\n");
        printf("(2) Atualizar\n");
        printf("(3) Remover\n");
        printf("(4) Consultar/Listar (filtros)\n");
        printf("(0) Voltar\n");
        printf("----------------------------------------\n");
        printf("Escolha: ");
        if (scanf("%d", &op) != 1)
        {
            limpar_buffer_entrada();
            op = -1;
        }

        switch (op)
        {
        case 1:
            cadastrar_consulta();
            break;
        case 2:
            atualizar_consulta();
            break;
        case 3:
            remover_consulta();
            break;
        case 4:
            submenu_consultar_consultas();
            break;
        case 0:
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (op != 0);
}

static void menu_principal()
{
    int op;
    do
    {
        cabecalho("CLINICA VET => MENU PRINCIPAL");
        printf("(1) Animais\n");
        printf("(2) Veterinarios\n");
        printf("(3) Consultas\n");
        printf("(4) Relatorios (TXT)\n");
        printf("(5) Popular exemplos (gera 10 de cada)\n");
        printf("(6) Salvar agora\n");
        printf("(0) Sair (salva e encerra)\n");
        printf("----------------------------------------\n");
        printf("Escolha: ");
        if (scanf("%d", &op) != 1)
        {
            limpar_buffer_entrada();
            op = -1;
        }

        switch (op)
        {
        case 1:
            menu_animais();
            break;
        case 2:
            menu_veterinarios();
            break;
        case 3:
            menu_consultas();
            break;
        case 4:
            menu_relatorios();
            break;
        case 5:
            popular_exemplos();
            break;
        case 6:
        {
            if (!salvar_animais(ARQ_ANIMAIS))
                puts("Falha ao salvar animais.");
            if (!salvar_vets(ARQ_VETS))
                puts("Falha ao salvar veterinarios.");
            if (!salvar_cons(ARQ_CONS))
                puts("Falha ao salvar consultas.");
            else
                puts("Arquivos salvos.");
        }
        break;
        case 0:
            printf("Salvando e saindo...\n");
            if (!salvar_animais(ARQ_ANIMAIS))
                puts("Falha ao salvar animais.");
            if (!salvar_vets(ARQ_VETS))
                puts("Falha ao salvar veterinarios.");
            if (!salvar_cons(ARQ_CONS))
                puts("Falha ao salvar consultas.");
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (op != 0);
}

// ======== Inicialização ========
static int inicializar_aplicacao()
{
    g_capAnimais = GROWTH_STEP;
    g_capVets = GROWTH_STEP;
    g_capCons = GROWTH_STEP;

    g_animais = (Animal *)malloc(g_capAnimais * sizeof(Animal));
    g_vets = (Veterinario *)malloc(g_capVets * sizeof(Veterinario));
    g_consultas = (Consulta *)malloc(g_capCons * sizeof(Consulta));

    if (!g_animais || !g_vets || !g_consultas)
    {
        printf("Falha de memoria na inicializacao.\n");
        free(g_animais);
        free(g_vets);
        free(g_consultas);
        return 0;
    }

    // Carregar dados dos binários (se existirem)
    if (!carregar_animais(ARQ_ANIMAIS))
    {
        puts("Erro ao carregar animais.");
        return 0;
    }
    if (!carregar_vets(ARQ_VETS))
    {
        puts("Erro ao carregar veterinarios.");
        return 0;
    }
    if (!carregar_cons(ARQ_CONS))
    {
        puts("Erro ao carregar consultas.");
        return 0;
    }

    return 1;
}

int main()
{
    if (!inicializar_aplicacao())
        return 1;
    menu_principal();

    free(g_animais);
    free(g_vets);
    free(g_consultas);
    return 0;
}
